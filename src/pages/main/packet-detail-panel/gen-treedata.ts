import dayjs from 'dayjs';
import { padHex, padFlag } from 'utils/pad';

export default function genTreedata(data: any): any {
  let treeData: any = [
    {
      title: '总览',
      key: 'overall',
      children: [
        {
          title: `时间: ${data.timestamp}(${dayjs(data.timestamp).format(
            'YYYY-MM-DD HH:mm:ss.SSS'
          )})`,
          key: 'timestamp',
          selectable: false
        },
        {
          title: `长度: ${data.capLen}字节`,
          key: 'caplen',
          selectable: false
        }
      ]
    },
    {
      title: '数据链路层(以太网II型)',
      key: 'linklayer',
      children: [
        {
          title: `来源MAC: ${data.srcMAC}`,
          key: 'srcmac',
          selectable: false
        },
        {
          title: `目标MAC: ${data.destMAC}`,
          key: 'destmac',
          selectable: false
        },
        {
          title: `类型: ${data.typeStr}(${padHex(data.type)})`,
          key: 'ethertype',
          selectable: false
        },
        {
          title: `填充长度: ${data.paddingLen}字节`,
          key: 'paddinglen',
          selectable: false
        }
      ]
    },
    {
      title: '网络层',
      key: 'internetlayer',
      children: [
        {
          title: `版本: ${data.payload.version}`,
          key: 'ipversion',
          selectable: false
        },
        {
          title: `头部长度: ${data.payload.headerLen}字节`,
          key: 'iphdrlen',
          selectable: false
        },
        {
          title: `总长度: ${data.payload.totalLen}字节`,
          key: 'iplen',
          selectable: false
        },
        {
          title: `ID: ${padHex(data.payload.id)}(${data.payload.id})`,
          key: 'ipid',
          selectable: false
        },
        {
          title: `禁止分片: ${data.payload.dfFlg}`,
          key: 'dfflg',
          selectable: false
        },
        {
          title: `更多分片: ${data.payload.mfFlg}`,
          key: 'mfflg',
          selectable: false
        },
        {
          title: `分片偏移: ${data.payload.fragmentOffset}`,
          key: 'fragmentoffset',
          selectable: false
        },
        {
          title: `TTL: ${data.payload.ttl}`,
          key: 'ttl',
          selectable: false
        },
        {
          title: `协议: ${data.payload.protocolStr}(${data.payload.protocol})`,
          key: 'protocol',
          selectable: false
        },
        {
          title: `检验和: ${padHex(data.payload.checksum)}`,
          key: 'ipchecksum',
          selectable: false
        },
        {
          title: `来源IP: ${data.payload.srcIP}`,
          key: 'srcip',
          selectable: false
        },
        {
          title: `目的IP: ${data.payload.destIP}`,
          key: 'destip',
          selectable: false
        }
      ]
    },
    {
      title: '传输层',
      key: 'transportlayer',
      children: [
        {
          title: `来源端口: ${data?.payload?.payload?.srcPort}`,
          key: 'srcport',
          selectable: false
        },
        {
          title: `目的端口: ${data?.payload?.payload?.destPort}`,
          key: 'destport',
          selectable: false
        }
      ]
    }
  ];
  if (data?.payload?.protocolStr === 'TCP') {
    const tcpPayload = data?.payload?.payload;
    const flagsArr = [];
    if (tcpPayload?.ackFlg) {
      flagsArr.push('ACK');
    }
    if (tcpPayload?.synFlg) {
      flagsArr.push('SYN');
    }
    if (tcpPayload?.finFlg) {
      flagsArr.push('FIN');
    }
    treeData[3].children = treeData[3].children.concat([
      {
        title: `SEQ: ${tcpPayload?.seq}`,
        key: 'seq',
        selectable: false
      },
      {
        title: `ACK: ${tcpPayload?.ack}`,
        key: 'ack',
        selectable: false
      },
      {
        title: `头部长度: ${tcpPayload?.headerLen}字节`,
        key: 'tcphdrlen',
        selectable: false
      },
      {
        title: `Flags: ${flagsArr.join(',')}`,
        key: 'flags',
        children: [
          {
            title: padFlag(0, tcpPayload?.cwrFlg, 'CWR'),
            key: 'cwrFlg',
            selectable: false
          },
          {
            title: padFlag(1, tcpPayload?.eceFlg, 'ECE'),
            key: 'eceFlg',
            selectable: false
          },
          {
            title: padFlag(2, tcpPayload?.urgFlg, 'URG'),
            key: 'urgFlg',
            selectable: false
          },
          {
            title: padFlag(3, tcpPayload?.ackFlg, 'ACK'),
            key: 'ackFlg',
            selectable: false
          },
          {
            title: padFlag(4, tcpPayload?.pshFlg, 'PSH'),
            key: 'pshFlg',
            selectable: false
          },
          {
            title: padFlag(5, tcpPayload?.rstFlg, 'RST'),
            key: 'rstFlg',
            selectable: false
          },
          {
            title: padFlag(6, tcpPayload?.synFlg, 'SYN'),
            key: 'synFlg',
            selectable: false
          },
          {
            title: padFlag(7, tcpPayload?.finFlg, 'FIN'),
            key: 'finflg',
            selectable: false
          }
        ]
      },
      {
        title: `窗口大小: ${tcpPayload?.windowSize}`,
        key: 'window',
        selectable: false
      },
      {
        title: `校验和: ${padHex(tcpPayload?.checksum)}`,
        key: 'tcpchecksum',
        selectable: false
      },
      {
        title: `紧急指针: ${tcpPayload?.urgentPtr}`,
        key: 'urgptr',
        selectable: false
      }
    ]);
  } else if (data?.payload?.protocolStr === 'UDP') {
    const udpPayload = data?.payload?.payload;
    treeData[3].children = treeData[3].children.concat([
      {
        title: `长度: ${udpPayload?.len}字节`,
        key: 'udplen',
        selectable: false
      },
      {
        title: `校验和: ${padHex(udpPayload?.checksum)}`,
        key: 'udpchecksum',
        selectable: false
      }
    ]);
  }
  if (['TCP', 'UDP'].includes(data?.payload?.protocolStr)) {
    const payload = data?.payload?.payload;
    if (payload?.payloadLen > 0) {
      treeData.push({
        title: '载荷数据',
        key: 'payload',
        children: [
          {
            title: `数据长度: ${payload?.payloadLen}字节`,
            key: 'payloadlen',
            selectable: false
          }
        ]
      });
    }
  }
  return treeData;
}
