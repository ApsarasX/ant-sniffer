#ifndef SNIFFER_UTILS_HPP
#define SNIFFER_UTILS_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include "device.hpp"
//#include "errors.hpp"

using namespace std;

class Utils final {
public:
    static string getIPFromSockAddr(struct sockaddr *sockAddr) {
        struct sockaddr_in *sockAddrIn = (struct sockaddr_in *) sockAddr;
        char ip[INET6_ADDRSTRLEN]{'\0'};
        inet_ntop(sockAddrIn->sin_family, (void *) &(sockAddrIn->sin_addr), ip, INET6_ADDRSTRLEN);
        return string(ip);
    }

    static string getNetmaskFromSockAddr(struct sockaddr *sockAddr) {
        struct sockaddr_in *sockAddrIn = (struct sockaddr_in *) sockAddr;
        char netmask[INET_ADDRSTRLEN]{'\0'};
        inet_ntop(sockAddrIn->sin_family, (void *) &(sockAddrIn->sin_addr), netmask, INET_ADDRSTRLEN);
        return string(netmask);
    }


    static string getMACFromUcharArr(u_char const mac[6]) {
        stringstream result;
        for (int i = 0; i < 6; ++i) {
            result << hex << setfill('0') << setw(2) << (u_int) (mac[i]);
            if (i < 5) {
                result << ':';
            }
        }
        return result.str();
    }

    static string getMACFromSockAddr(struct sockaddr *sockAddr) {
        struct sockaddr_dl *link = (struct sockaddr_dl *) sockAddr->sa_data;
        u_char macTmp[link->sdl_alen];
        memcpy(macTmp, LLADDR(link), link->sdl_alen);
        char mac[21]{'\0'};
        if (link->sdl_alen == 6) {
            sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) macTmp[0],
                    (unsigned char) macTmp[1],
                    (unsigned char) macTmp[2],
                    (unsigned char) macTmp[3],
                    (unsigned char) macTmp[4],
                    (unsigned char) macTmp[5]);
        } else if (link->sdl_alen > 6) {
            // 兼容OSX 10.6.5
            sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) macTmp[1],
                    (unsigned char) macTmp[2],
                    (unsigned char) macTmp[3],
                    (unsigned char) macTmp[4],
                    (unsigned char) macTmp[5],
                    (unsigned char) macTmp[6]);
        }
        return string(mac);
    }

    static vector<SnifferDevice> getAllDevices() noexcept(true) {
        vector<SnifferDevice> devices{};
        pcap_if_t *pcapDevices;
        char errBuf[PCAP_ERRBUF_SIZE];
        if (pcap_findalldevs(&pcapDevices, errBuf) == PCAP_ERROR) {
//            throw FindAllDevicesException("查找网卡失败");
            return devices;
        }
        for (pcap_if_t *devIt = pcapDevices; devIt; devIt = devIt->next) {
            SnifferDevice device{
                    .name = devIt->name,
                    .description = devIt->description ? devIt->description : "",
                    .loopbackFlag = bool(PCAP_IF_LOOPBACK & devIt->flags),
                    .upFlag = bool(PCAP_IF_UP & devIt->flags),
                    .runningFlag = bool(PCAP_IF_RUNNING & devIt->flags),
                    .wirelessFlag = bool(0x00000008 & devIt->flags)
            };
            // 获取设备连接状态
            switch (devIt->flags & 0x00000030) {
                case 0x00000000:
                    device.connStatus = SnifferDevice::status::UNKNOWN;
                    break;
                case 0x00000010:
                    device.connStatus = SnifferDevice::status::CONNECTED;
                    break;
                case 0x00000020:
                    device.connStatus = SnifferDevice::status::DISCONNECTED;
                    break;
                case 0x00000030:
                    device.connStatus = SnifferDevice::status::NOT_APPLICABLE;
                    break;
            }
            for (auto addrIt = devIt->addresses; addrIt; addrIt = addrIt->next) {
                SnifferDevice::address deviceAddr{};
                switch (addrIt->addr->sa_family) {
                    case AF_INET: {
                        if (addrIt->netmask) {
                            deviceAddr.netmask = getNetmaskFromSockAddr(addrIt->netmask);
                        }
                        if (addrIt->broadaddr) {
                            deviceAddr.broadcast = getIPFromSockAddr(addrIt->broadaddr);
                        }
                        // 此处不写break
                    }
                    case AF_INET6: {
                        if (addrIt->addr) {
                            deviceAddr.ip = getIPFromSockAddr(addrIt->addr);
                        }
                        break;
                    }
                        // 这个case每个设备只会命中一次
                    case AF_LINK: {
                        if (addrIt->addr) {
                            device.mac = getMACFromSockAddr(addrIt->addr);
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (!(deviceAddr.ip.empty() && deviceAddr.netmask.empty() && deviceAddr.broadcast.empty())) {
                    device.addresses.push_back(deviceAddr);
                }
            }
            devices.push_back(device);
        }
        pcap_freealldevs(pcapDevices);
        return devices;
    }

private:
    Utils() = default;

    ~Utils() = default;
};

#endif