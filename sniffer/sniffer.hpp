#ifndef SNIFFER_HPP
#define SNIFFER_HPP

#include <iostream>
#include <string>
#include <napi.h>
#include <pcap.h>
#include "loop_worker.hpp"

namespace {
using namespace std;

class Sniffer : public Napi::ObjectWrap<Sniffer> {
public:
    static void Init(Napi::Env env) {
        Napi::HandleScope scope(env);
        Napi::Function ctor = DefineClass(
            env, "Sniffer",
            {InstanceMethod("setFilter", &Sniffer::setFilter), InstanceMethod("start", &Sniffer::start),
             InstanceMethod("restart", &Sniffer::restart), InstanceMethod("onProgress", &Sniffer::onProgress),
             InstanceMethod("keepAlive", &Sniffer::keepAlive), InstanceMethod("closeDev", &Sniffer::closeDev)});

        Sniffer::constructor = Napi::Persistent(ctor);
        Sniffer::constructor.SuppressDestruct();
    }

    static Napi::Object NewInstance(const Napi::CallbackInfo &info) {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);
        int length = info.Length();
        if (length <= 0 || !info[0].IsString()) {
            Napi::TypeError::New(env, "The first parameter must be a device name as a string")
                .ThrowAsJavaScriptException();
        }
        Napi::String devName = info[0].As<Napi::String>();
        if (string(devName).empty()) {
            Napi::TypeError::New(env, "Device name cannot be empty").ThrowAsJavaScriptException();
        }
        Napi::Number snaplen = Napi::Number::New(env, 65535);
        Napi::Boolean promisc = Napi::Boolean::New(env, true);
        Napi::Number bufSize = Napi::Number::New(env, 16777216); // 16 << 20
        Napi::Number timeout = Napi::Number::New(env, 1000);
        if (length >= 2) {
            if (!info[1].IsObject()) {
                Napi::TypeError::New(env, "The second parameter must be pcap attributes as a object")
                    .ThrowAsJavaScriptException();
            } else {
                Napi::Object attr = info[1].As<Napi::Object>();
                if (attr.Has("snaplen") && attr.Get("snaplen").IsNumber()) {
                    snaplen = attr.Get("snaplen").As<Napi::Number>();
                }
                if (attr.Has("promisc") && attr.Get("promisc").IsBoolean()) {
                    promisc = attr.Get("promisc").As<Napi::Boolean>();
                }
                if (attr.Has("bufSize") && attr.Get("bufSize").IsNumber()) {
                    bufSize = attr.Get("bufSize").As<Napi::Number>();
                }
                if (attr.Has("timeout") && attr.Get("timeout").IsNumber()) {
                    timeout = attr.Get("timeout").As<Napi::Number>();
                }
            }
        }
        Napi::Object sniffer = Sniffer::constructor.New({devName, snaplen, promisc, bufSize, timeout});
        return sniffer;
    }

    explicit Sniffer(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Sniffer>(info) {
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);
        this->devName = info[0].ToString().Utf8Value();
        int snaplen = info[1].ToNumber().Int32Value();
        bool promisc = info[2].ToBoolean().Value();
        int bufSize = info[3].ToNumber().Int32Value();
        int timeout = info[4].ToNumber().Int32Value();
        this->pcapHandle = pcap_create(this->devName.data(), this->errBuf);
        if (this->pcapHandle == nullptr) {
            return;
        }
        // 设置最大捕获包长度
        if (pcap_set_snaplen(this->pcapHandle, snaplen) == PCAP_ERROR_ACTIVATED) {
            return;
        }
        // 设置为混杂模式
        if (pcap_set_promisc(this->pcapHandle, int(promisc)) == PCAP_ERROR_ACTIVATED) {
            return;
        }
        // 设置Buffer
        if (pcap_set_buffer_size(this->pcapHandle, bufSize) == PCAP_ERROR_ACTIVATED) {
            return;
        }
        // 设置为混杂模式
        if (pcap_set_timeout(this->pcapHandle, timeout) == PCAP_ERROR_ACTIVATED) {
            return;
        }
        if (pcap_activate(this->pcapHandle) < 0) {
            return;
        }
        this->success = true;
    }

    ~Sniffer() override {
        cout << "------------------------------Sniffer 析构------------------------------" << endl;
        cout << "------------------------------Sniffer 析构------------------------------" << endl;
        cout << "------------------------------Sniffer 析构------------------------------" << endl;
    }

    void keepAlive(const Napi::CallbackInfo &info) {}

    Napi::Value setFilter(const Napi::CallbackInfo &info) {
        Napi::Env env = info.Env();
        if (info.Length() > 0 && info[0].IsString()) {
            bpf_u_int32 maskp;
            bpf_u_int32 netp;
            pcap_lookupnet(this->devName.data(), &netp, &maskp, this->errBuf);
            this->filterString = info[0].ToString().Utf8Value();
            if (pcap_compile(this->pcapHandle, &(this->filter), this->filterString.data(), 0, netp) == PCAP_ERROR) {
                return Napi::Boolean::New(env, false);
            }
            if (pcap_setfilter(this->pcapHandle, &(this->filter)) == PCAP_ERROR) {
                return Napi::Boolean::New(env, false);
            }
            pcap_freecode(&(this->filter));
            return Napi::Boolean::New(env, true);
        } else {
            Napi::TypeError::New(env, "The first parameter must be a string").ThrowAsJavaScriptException();
        }
        return Napi::Boolean::New(env, false);
    }

    void start(const Napi::CallbackInfo &info) {
        if (!this->worker) {
            this->worker = new LoopWorker(this->pcapHandle, this->callback.Value());
        }
        this->worker->Queue();
    }

    //        void stop(const Napi::CallbackInfo &info) {
    //            if (this->worker) {
    //                this->worker->Cancel();
    //                delete this->worker;
    //                this->worker = nullptr;
    //            }
    //        }

    void restart(const Napi::CallbackInfo &info) {
        //            this->stop(info);
        this->start(info);
    }

    void onProgress(const Napi::CallbackInfo &info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Requires at least one parameter").ThrowAsJavaScriptException();
            return;
        }
        if (!info[0].IsFunction()) {
            Napi::TypeError::New(env, "The first parameter must be a function").ThrowAsJavaScriptException();
            return;
        }
        this->callback = Napi::Persistent(info[0].As<Napi::Function>());
    }

    void closeDev(const Napi::CallbackInfo &info) {
        if (this->worker) {
            this->worker->Cancel();
            delete this->worker;
        }
        if (this->pcapHandle) {
            pcap_close(this->pcapHandle);
            this->pcapHandle = nullptr;
        }
    }

private:
    static inline Napi::FunctionReference constructor;

    pcap_t *pcapHandle = nullptr;

    char errBuf[PCAP_ERRBUF_SIZE]{};

    string devName;

    string filterString;

    struct bpf_program filter;

    bool success = false;

    LoopWorker *worker = nullptr;

    Napi::FunctionReference callback;
};

} // namespace

#endif
