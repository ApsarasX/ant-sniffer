#ifndef LOOP_WORKER_HPP
#define LOOP_WORKER_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <napi.h>
#include "packet.hpp"
#include "parse.hpp"

#define QUEUE_MAX_SIZE 4096

namespace {
using namespace std;

class LoopWorker : public Napi::AsyncProgressWorker<EtherFrame> {
public:
    explicit LoopWorker(pcap_t *handle, Napi::Function callback) : AsyncProgressWorker(callback), pcapHandle(handle) {}

    ~LoopWorker() override = default;

    void Execute(const ExecutionProgress &progress) override {
        unique_lock<mutex> lock(this->execMux);
        unique_lock<mutex> pcapLock(pcapMux);
        while (this->loopFlag) {
            consumer.wait(pcapLock, [this] { return !this->loopFlag || !q.empty(); });
            EtherFrame *frame = q.front();
            q.pop();
            producer.notify_all();
            progress.Send(frame, 1);
            this->execCV.wait(lock);
        }
    }

    void OnProgress(const EtherFrame *data, size_t) override {
        Napi::Env env = Env();
        Napi::Object obj = data->toNapiObject(env);
        Callback().Call({obj});
        this->execCV.notify_all();
    }

    void Queue() {
        this->loopFlag = true;
        thread t([this] { pcap_loop(this->pcapHandle, 0, LoopWorker::handlePacket, nullptr); });
        t.detach();
        AsyncProgressWorker::Queue();
    }

    void Cancel() {
        this->loopFlag = false;
        pcap_breakloop(this->pcapHandle);
        // TODO: dead lock
        pcapMux.unlock();
    }

    static void handlePacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) noexcept(false) {
        unique_lock<mutex> lock(pcapMux);
        producer.wait(lock, [] { return q.size() < QUEUE_MAX_SIZE; });
        // TODO: 未释放
        EtherFrame *frame = parseEtherFrame(packet);
        frame->capLen = header->caplen;
        frame->timestamp = u_int64_t(header->ts.tv_sec) * 1000 + header->ts.tv_usec / 1000;
        q.push(frame);
        consumer.notify_all();
    }

private:
    pcap_t *pcapHandle;
    condition_variable execCV;
    mutex execMux;
    bool loopFlag = false;
    static inline condition_variable producer, consumer;
    static inline mutex pcapMux;
    static inline queue<EtherFrame *> q;
};
} // namespace

#endif // LOOP_WORKER_HPP