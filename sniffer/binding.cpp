#include <napi.h>
#include "device.hpp"
#include "sniffer.hpp"
#include "utils.hpp"

using namespace std;

Napi::Array getDevs(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    vector<SnifferDevice> devices = Utils::getAllDevices();
    size_t devsSize = devices.size();
    Napi::Array devsArr = Napi::Array::New(env, devsSize);
    for (size_t i = 0; i < devsSize; ++i) {
        const SnifferDevice &device = devices[i];
        Napi::Object devObj = device.toNapiObject(env);
        devsArr[i] = devObj;
    }
    return devsArr;
}

Napi::Object openDev(const Napi::CallbackInfo &info) {
    Napi::HandleScope scope(info.Env());
    return Sniffer::NewInstance(info);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    Sniffer::Init(env);
    exports.Set("getDevs", Napi::Function::New(env, getDevs));
    exports.Set("openDev", Napi::Function::New(env, openDev));
    return exports;
}

NODE_API_MODULE(sniffer, InitAll);