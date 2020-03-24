#ifndef SNIFFER_DEVICE_HPP
#define SNIFFER_DEVICE_HPP

#include <iomanip>
#include <iostream>
#include <napi.h>
#include <pcap.h>
#include <string>
#include <vector>

namespace {

using namespace std;

struct SnifferDevice final {
    struct address {
        string ip{};
        string netmask{};
        string broadcast{};
    };
    enum class status { UNKNOWN, CONNECTED, DISCONNECTED, NOT_APPLICABLE };
    string name{};
    string description{};
    string mac{};
    vector<address> addresses{};
    bool loopbackFlag = false;
    bool upFlag = false;
    bool runningFlag = false;
    bool wirelessFlag = false;
    status connStatus = status::UNKNOWN;

    Napi::Object toNapiObject(Napi::Env env) const {
        Napi::Object devObj = Napi::Object::New(env);
        devObj["name"] = this->name;
        devObj["description"] = this->description;
        devObj["mac"] = this->mac;
        size_t addrsSize = this->addresses.size();
        if (addrsSize > 0) {
            Napi::Array addrsArr = Napi::Array::New(env, addrsSize);
            for (size_t j = 0; j < addrsSize; ++j) {
                const address &addr = this->addresses[j];
                Napi::Object addrObj = Napi::Object::New(env);
                if (!addr.ip.empty()) {
                    addrObj["ip"] = addr.ip;
                }
                if (!addr.netmask.empty()) {
                    addrObj["netmask"] = addr.netmask;
                }
                if (!addr.broadcast.empty()) {
                    addrObj["broadcast"] = addr.broadcast;
                }
                addrsArr[j] = addrObj;
            }
            devObj["addrs"] = addrsArr;
        }
        devObj["loopback"] = this->loopbackFlag;
        devObj["up"] = this->upFlag;
        devObj["running"] = this->runningFlag;
        devObj["wireless"] = this->wirelessFlag;
        switch (this->connStatus) {
        case SnifferDevice::status::UNKNOWN:
            devObj["connStatus"] = "未知";
            break;
        case SnifferDevice::status::CONNECTED:
            devObj["connStatus"] = "已连接";
            break;
        case SnifferDevice::status::DISCONNECTED:
            devObj["connStatus"] = "未连接";
            break;
        case SnifferDevice::status::NOT_APPLICABLE:
            devObj["connStatus"] = "无";
            break;
        default:
            break;
        }
        return devObj;
    }

    void print() {
        cout.setf(ios::left);
        cout << "----------------------------------------" << endl;
        cout << setw(6) << "名称: " << name << endl;
        cout << setw(6) << "描述: " << description << endl;
        cout << setw(6) << "MAC: " << mac << endl;
        for (address &addr : addresses) {
            cout << "##########" << endl;
            if (!addr.ip.empty()) {
                cout << setw(6) << "IP: " << addr.ip << endl;
            }
            if (!addr.netmask.empty()) {
                cout << setw(6) << "掩码: " << addr.netmask << endl;
            }
            if (!addr.broadcast.empty()) {
                cout << setw(6) << "广播: " << addr.broadcast << endl;
            }
            cout << "##########" << endl;
        }
        cout << setw(6) << "回环: " << boolalpha << loopbackFlag << endl;
        cout << setw(6) << "开启: " << boolalpha << upFlag << endl;
        cout << setw(6) << "运行: " << boolalpha << runningFlag << endl;
        cout << setw(6) << "无线: " << boolalpha << wirelessFlag << endl;
        cout << setw(6) << "连接状态: ";
        switch (connStatus) {
        case status::UNKNOWN:
            cout << "未知";
            break;
        case status::CONNECTED:
            cout << "已连接";
            break;
        case status::DISCONNECTED:
            cout << "未连接";
            break;
        case status::NOT_APPLICABLE:
            cout << "无";
            break;
        }
        cout << endl;
    }
};
} // namespace

#endif // SNIFFER_DEVICE_HPP
