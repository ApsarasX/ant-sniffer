#include <arpa/inet.h>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include "packet.hpp"

TCPSegment *parseTCPSegment(IPv4Datagram *datagram, const u_char *packet, u_int offset) {
    const struct tcphdr *rawSegment = reinterpret_cast<const struct tcphdr *>(packet + offset);
    TCPSegment *segment = new TCPSegment;
    segment->srcPort = ntohs(rawSegment->th_sport);
    segment->destPort = ntohs(rawSegment->th_dport);
    segment->seq = ntohl(rawSegment->th_seq);
    segment->ack = ntohl(rawSegment->th_ack);
    segment->headerLen = rawSegment->th_off * 4;
    segment->finFlg = rawSegment->th_flags & TH_FIN;
    segment->synFlg = rawSegment->th_flags & TH_SYN;
    segment->rstFlg = rawSegment->th_flags & TH_RST;
    segment->pshFlg = rawSegment->th_flags & TH_PUSH;
    segment->ackFlg = rawSegment->th_flags & TH_ACK;
    segment->urgFlg = rawSegment->th_flags & TH_URG;
    segment->eceFlg = rawSegment->th_flags & TH_ECE;
    segment->cwrFlg = rawSegment->th_flags & TH_CWR;
    segment->windowSize = ntohs(rawSegment->th_win);
    segment->checksum = ntohs(rawSegment->th_sum);
    segment->urgentPtr = ntohs(rawSegment->th_urp);
    segment->payload = const_cast<u_char *>(packet + offset + segment->headerLen);
    segment->payloadLen = datagram->totalLen - (datagram->headerLen + segment->headerLen);
    segment->formatPayload();
    return segment;
}

UDPSegment *parseUDPSegment(IPv4Datagram *datagram, const u_char *packet, u_int offset) {
    const struct udphdr *rawSegment = reinterpret_cast<const struct udphdr *> (packet + offset);
    UDPSegment *segment = new UDPSegment;
    segment->srcPort = ntohs(rawSegment->uh_sport);
    segment->destPort = ntohs(rawSegment->uh_dport);
    segment->len = ntohs(rawSegment->uh_ulen);
    segment->checksum = ntohs(rawSegment->uh_sum);
    segment->payload = const_cast<u_char *>(packet + offset + sizeof(struct udphdr));
    segment->payloadLen = datagram->totalLen - (datagram->headerLen + 8);
    segment->formatPayload();
    return segment;
}

IPv4Datagram *parseIPDatagram(const u_char *packet, u_int offset) {
    const struct ip *rawDatagram = reinterpret_cast<const struct ip *>(packet + offset);
    IPv4Datagram *datagram = new IPv4Datagram;
    datagram->version = rawDatagram->ip_v;
    datagram->headerLen = rawDatagram->ip_hl * 4;
    datagram->tos = rawDatagram->ip_tos;
    datagram->totalLen = ntohs(rawDatagram->ip_len);
    datagram->id = ntohs(rawDatagram->ip_id);
    datagram->dfFlg = rawDatagram->ip_off & IP_DF;
    datagram->mfFlg = rawDatagram->ip_off & IP_MF;
    datagram->fragmentOffset = rawDatagram->ip_off << 3;
    datagram->ttl = rawDatagram->ip_ttl;
    datagram->protocol = rawDatagram->ip_p;
    datagram->checksum = ntohs(rawDatagram->ip_sum);
    datagram->srcIP = inet_ntoa(rawDatagram->ip_src);
    datagram->destIP = inet_ntoa(rawDatagram->ip_dst);
    switch (rawDatagram->ip_p) {
        case IPPROTO_TCP:
            datagram->protocolStr = "TCP";
            datagram->payload = parseTCPSegment(datagram, packet, offset + datagram->headerLen);
            break;
        case IPPROTO_UDP:
            datagram->protocolStr = "UDP";
            datagram->payload = parseUDPSegment(datagram, packet, offset + datagram->headerLen);
            break;
        case IPPROTO_ICMP:
            break;
        case IPPROTO_IGMP:
            break;
    }
    return datagram;
}

// [链路层] 以太网帧头部
EtherFrame *parseEtherFrame(const u_char *packet) noexcept(false) {
    const ether_header_t *rawFrame = (ether_header_t *) packet;
    EtherFrame *frame = new EtherFrame;
    frame->destMAC = ether_ntoa((struct ether_addr *) rawFrame->ether_dhost);
    frame->srcMAC = ether_ntoa((struct ether_addr *) rawFrame->ether_shost);
    frame->type = ntohs(rawFrame->ether_type);
    switch (ntohs(rawFrame->ether_type)) {
        // IPv4
        case ETHERTYPE_IP:
            // [网络层] IP数据报头部
            frame->typeStr = "IPv4";
            frame->payload = parseIPDatagram(packet, ETHER_HDR_LEN);
            if (const IPv4Datagram *datagram = (const IPv4Datagram *)(frame->payload)) {
                frame->paddingLen = datagram->totalLen < 46 ? 46 - datagram->totalLen : 0;
            }
            break;
        case ETHERTYPE_IPV6:
            break;
        case ETHERTYPE_ARP:
            break;
        case ETHERTYPE_REVARP:
            break;
    }
    return frame;
}