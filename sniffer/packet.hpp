#ifndef SNIFFER_PACKET_HPP
#define SNIFFER_PACKET_HPP

#include <string>
#include <sstream>
#include <napi.h>

namespace {
    using namespace std;

    struct TransportLayerSegment {
        const u_char *payload = nullptr;
        u_int payloadLen;
        string formattedPayload{};

        virtual ~TransportLayerSegment() = default;

        static stringstream formatPayloadLine(const u_char *payload, u_int len, u_int offset) {
            stringstream result;
            u_int i;
            u_int gap;
            const u_char *ch;

            /* offset */
            result << setw(5) << setfill('0') << offset << "   ";

            /* hex */
            ch = payload;
            for (i = 0; i < len; i++) {
                result << hex << setw(2) << setfill('0') << u_int(*ch) << dec << ' ';
                ch++;
                /* print extra space after 8th byte for visual aid */
                if (i == 7)
                    result << ' ';
            }
            /* print space to handle line less than 8 bytes */
            if (len < 8)
                result << ' ';

            /* fill hex gap with spaces if not full line */
            if (len < 16) {
                gap = 16 - len;
                for (i = 0; i < gap; i++) {
                    result << "   ";
                }
            }
            result << "   ";

            // 打印
            ch = payload;
            for (i = 0; i < len; i++) {
                if (isprint(*ch))
                    result << char(*ch);
                else
                    result << '.';

                ch++;
            }

            result << '\n';
            return result;
        }

        virtual void formatPayload() {
            stringstream result;
            // 剩余长度
            u_int remainingLen = this->payloadLen;
            // 每行理论长度
            u_int lineWidth = 16;
            // 每行的实际长度
            u_int lineLen;
            u_int offset = 0;
            const u_char *ch = this->payload;

            if (this->payloadLen <= 0) {
                this->formattedPayload = result.str();
                return;
            }

            // 单行数据
            if (this->payloadLen <= lineWidth) {
                result << formatPayloadLine(ch, this->payloadLen, offset).rdbuf();
                this->formattedPayload = result.str();
                return;
            }

            // 多行数据
            while (true) {
                // 当前行实际长度
                lineLen = lineWidth % remainingLen;
                // 打印当前行
                result << formatPayloadLine(ch, lineLen, offset).rdbuf();
                // 计算剩余长度
                remainingLen = remainingLen - lineLen;
                // 将指针移动到剩余数据上
                ch = ch + lineLen;
                // 计算数据偏移
                offset = offset + lineWidth;
                // 检查剩余数据是否不超过一行
                if (remainingLen <= lineWidth) {
                    // 打印最后一行
                    result << formatPayloadLine(ch, remainingLen, offset).rdbuf();
                    break;
                }
            }
            this->formattedPayload = result.str();
        }

//        virtual void print() const {
//            cout << "       载荷长度: " << this->payloadLen << endl;
//            cout << this->formattedPayload;
//        }

        [[nodiscard]] virtual Napi::Object toNapiObject(Napi::Env env) const = 0;
    };

    struct TCPSegment : public TransportLayerSegment {
        u_int srcPort;
        u_int destPort;
        u_int seq;
        u_int ack;
        u_int headerLen;
        bool finFlg;
        bool synFlg;
        bool rstFlg;
        bool pshFlg;
        bool ackFlg;
        bool urgFlg;
        bool eceFlg;
        bool cwrFlg;
        u_int windowSize;
        u_int checksum;
        u_int urgentPtr;
        [[nodiscard]] Napi::Object toNapiObject(Napi::Env env) const override {
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("srcPort", this->srcPort);
            obj.Set("destPort", this->destPort);
            obj.Set("seq", this->seq);
            obj.Set("ack", this->ack);
            obj.Set("headerLen", this->headerLen);
            obj.Set("finFlg", this->finFlg);
            obj.Set("synFlg", this->synFlg);
            obj.Set("rstFlg", this->rstFlg);
            obj.Set("pshFlg", this->pshFlg);
            obj.Set("ackFlg", this->ackFlg);
            obj.Set("urgFlg", this->urgFlg);
            obj.Set("eceFlg", this->eceFlg);
            obj.Set("cwrFlg", this->cwrFlg);
            obj.Set("windowSize", this->windowSize);
            obj.Set("checksum", this->checksum);
            obj.Set("urgentPtr", this->urgentPtr);
            obj.Set("payload", this->formattedPayload);
            obj.Set("payloadLen", this->payloadLen);
            return obj;
        }
//        void print() const override {
//            cout << "       源端口: " << this->srcPort << endl
//                 << "       目的端口:" << this->destPort << endl
//                 << "       SEQ: " << this->seq << endl
//                 << "       ACK: " << this->ack << endl
//                 << "       头部长度: " << this->headerLen << endl
//                 << "       finFlg: " << boolalpha << this->finFlg << endl
//                 << "       synFlg: " << boolalpha << this->synFlg << endl
//                 << "       rstFlg: " << boolalpha << this->rstFlg << endl
//                 << "       pshFlg: " << boolalpha << this->pshFlg << endl
//                 << "       ackFlg: " << boolalpha << this->ackFlg << endl
//                 << "       urgFlg: " << boolalpha << this->urgFlg << endl
//                 << "       eceFlg: " << boolalpha << this->eceFlg << endl
//                 << "       cwrFlg: " << boolalpha << this->cwrFlg << endl
//                 << "       窗口大小: " << this->windowSize << endl
//                 << "       校验和: 0x" << hex << setw(4) << setfill('0') << this->checksum << endl << dec
//                 << "       紧急指针: " << this->urgentPtr << endl;
//            if (this->payload) {
//                TransportLayerSegment::print();
//            }
//        }
    };

    struct UDPSegment : public TransportLayerSegment {
        // 源端口
        u_int srcPort;
        // 目的端口
        u_int destPort;
        // 总长度
        u_int len;
        // 校验和
        u_int checksum;

        [[nodiscard]] Napi::Object toNapiObject(Napi::Env env) const override {
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("srcPort", this->srcPort);
            obj.Set("destPort", this->destPort);
            obj.Set("len", this->len);
            obj.Set("checksum", this->checksum);
            obj.Set("payload", this->formattedPayload);
            obj.Set("payloadLen", this->payloadLen);
            return obj;
        }
//        void print() const override {
//            cout << "       源端口: " << this->srcPort << endl
//                 << "       目的端口:" << this->destPort << endl
//                 << "       长度: " << this->len << endl
//                 << "       校验和: 0x" << hex << setw(4) << setfill('0') << this->checksum << endl << dec;
//            if (this->payload) {
//                TransportLayerSegment::print();
//            }
//        }
    };

    struct InternetLayerDatagram {
        const TransportLayerSegment *payload = nullptr;

        virtual ~InternetLayerDatagram() {
            delete payload;
        }

//        virtual void print() const = 0;

        [[nodiscard]] virtual Napi::Object toNapiObject(Napi::Env env) const = 0;
    };

    struct IPv4Datagram : public InternetLayerDatagram {
        u_int version;
        u_int headerLen;
        u_char tos;
        u_int totalLen;
        u_int id;
        bool dfFlg;
        bool mfFlg;
        u_int fragmentOffset;
        u_int ttl;
        u_int protocol;
        string protocolStr;
        u_int checksum;
        string srcIP{};
        string destIP{};

        [[nodiscard]] Napi::Object toNapiObject(Napi::Env env) const override {
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("version", this->version);
            obj.Set("headerLen", this->headerLen);
            obj.Set("totalLen", this->totalLen);
            obj.Set("id", this->id);
            obj.Set("dfFlg", this->dfFlg);
            obj.Set("mfFlg", this->mfFlg);
            obj.Set("fragmentOffset", this->fragmentOffset);
            obj.Set("protocol", this->protocol);
            obj.Set("protocolStr", this->protocolStr);
            obj.Set("ttl", this->ttl);
            obj.Set("checksum", this->checksum);
            obj.Set("srcIP", this->srcIP);
            obj.Set("destIP", this->destIP);
            if (this->payload) {
                obj.Set("payload", this->payload->toNapiObject(env));
            }
            return obj;
        }
//        void print() const override {
//            cout << "   版本: " << this->version << endl
//                 << "   头部长度: " << this->headerLen << endl
//                 << "   总长度: " << this->totalLen << endl
//                 << "   ID: 0x" << hex << setw(4) << setfill('0') << this->id << endl << dec
//                 << "   禁止分片: " << boolalpha << this->dfFlg << endl
//                 << "   更多分片: " << boolalpha << this->mfFlg << endl
//                 << "   分片偏移: " << this->fragmentOffset << endl
//                 << "   协议: 0x" << hex << setw(2) << setfill('0') << this->protocol << endl << dec
//                 << "   TTL: " << this->ttl << endl
//                 << "   校验和: 0x" << hex << setw(4) << setfill('0') << this->checksum << endl << dec
//                 << "   源IP: " << this->srcIP << endl
//                 << "   目的IP: " << this->destIP << endl;
//            if (this->payload) {
//                this->payload->print();
//            }
//        }
    };

    struct LinkLayerFrame {
        const InternetLayerDatagram *payload = nullptr;

//        virtual void print() const = 0;
        [[nodiscard]] virtual Napi::Object toNapiObject(Napi::Env env) const = 0;

        virtual ~LinkLayerFrame() {
            delete payload;
        }
    };

    struct EtherFrame : public LinkLayerFrame {
        string destMAC{};
        string srcMAC{};
        u_int type;
        string typeStr;
        u_int paddingLen;
        u_int capLen;
        u_int64_t timestamp;

//        void print() const override {
//            cout << "目的MAC: " << this->destMAC << endl
//                 << "源MAC: " << this->srcMAC << endl
//                 << "以太类型: 0x" << hex << setw(4) << setfill('0') << this->type << endl << dec;
//            if (this->payload) {
//                this->payload->print();
//            }
//        }
        [[nodiscard]] Napi::Object toNapiObject(Napi::Env env) const override {
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("destMAC", Napi::String::New(env, this->destMAC));
            obj.Set("srcMAC", Napi::String::New(env, this->srcMAC));
            obj.Set("type", Napi::Number::New(env, this->type));
            obj.Set("typeStr", Napi::String::New(env, this->typeStr));
            obj.Set("paddingLen", Napi::Number::New(env, this->paddingLen));
            obj.Set("capLen", Napi::Number::New(env, this->capLen));
            obj.Set("timestamp", Napi::Number::New(env, this->timestamp));
            if (this->payload) {
                obj.Set("payload", this->payload->toNapiObject(env));
            }
            return obj;
        }
    };
}


#endif // SNIFFER_PACKET_HPP