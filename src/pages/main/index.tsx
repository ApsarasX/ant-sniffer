import React from 'react';
import { withRouter } from 'react-router';
import { Button, Input, Modal, Switch, message, Tag } from 'antd';
import { ipcRenderer } from 'electron';
import dayjs from 'dayjs';
import PacketDetailPanel from './packet-detail-panel';
import VirtualTable from './virtual-table';
import styles from './style.module.less';

const { Search } = Input;

interface IProps {
  location: any;
}
interface IState {
  running: boolean;
  scrollUpdate: boolean;
  packetDetailPanelVisible: boolean;
  packets: any[];
  expandedPacketIndex: number;
  filterExpr: string;
  lastFilterExpr: string;
}

class MainPage extends React.Component<IProps, IState> {
  packetsQueue: any[] = [];
  rawPackets: any[] = [];
  devName: string | null;
  timer: number = 0;
  messageMutex: boolean = false;
  constructor(props: IProps) {
    super(props);
    this.state = {
      running: false,
      scrollUpdate: true,
      packetDetailPanelVisible: false,
      packets: [],
      expandedPacketIndex: -1,
      filterExpr: '',
      lastFilterExpr: ''
    };
    this.devName = new URLSearchParams(props.location.search).get('dev');
  }

  componentDidMount() {
    if (this.devName) {
      ipcRenderer.send('openDevice', this.devName);
      this.start();
    }
  }
  componentWillUnmount() {
    this.stop();
  }

  start = () => {
    this.setState({
      running: true
    });
    ipcRenderer.send('startCapture');
    ipcRenderer.on('progress', (event, packet) => {
      if (this.state.running) {
        this.packetsQueue.push(packet);
      }
    });
    this.timer = window.setInterval(() => {
      const len = Math.min(400, this.packetsQueue.length);
      if (len > 0) {
        const packets = this.packetsQueue.splice(0, len);
        this.handlePacket(packets);
      }
    }, 100);
  };

  stop = () => {
    this.setState({
      running: false
    });
    ipcRenderer.removeAllListeners('progress');
    window.clearInterval(this.timer);
  };

  clear = () => {
    Modal.confirm({
      title: '是否清空当前数据',
      okText: '清空',
      cancelText: '保留',
      onOk: () => {
        this.setState({
          packets: [],
          expandedPacketIndex: -1
        });
        this.rawPackets = [];
      }
    });
  };

  handlePacket = (packets: any[]) => {
    this.rawPackets = this.rawPackets.concat(packets);
    let nextBathcPackts: any[] = [];
    for (let i = 0; i < packets.length; ++i) {
      const packet = packets[i];
      const nextRowPacket: any = {
        no: this.state.packets.length + i,
        time: dayjs(packet.timestamp).format('YYYY-MM-DD HH:mm:ss.SSS'),
        source: packet?.payload?.srcIP || packet.srcMAC,
        destination: packet?.payload?.destIP || packet.destMAC,
        protocol: packet?.payload?.protocolStr || '[未支持的协议]',
        length: packet.capLen
      };
      if (['TCP', 'UDP'].includes(packet?.payload?.protocolStr)) {
        let text = `${packet?.payload?.payload?.srcPort} → ${packet?.payload?.payload?.destPort}`;
        if (packet?.payload?.protocolStr === 'TCP') {
          const tcpPayload = packet?.payload?.payload;
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
          if (tcpPayload?.rstFlg) {
            flagsArr.push('RST');
          }
          text += `&emsp;[${flagsArr.join(',')}]&emsp;Ack = ${
            tcpPayload?.ack
          }&emsp;Seq = ${tcpPayload?.seq}&emsp;Win = ${tcpPayload?.windowSize}`;
        }
        nextRowPacket.info = text;
      }
      nextBathcPackts.push(nextRowPacket);
    }
    this.setState(state => ({
      packets: state.packets.concat(nextBathcPackts)
    }));
  };

  handleFilter = (value: string) => {
    ipcRenderer.send('filter-request', value);
    ipcRenderer.once('filter-response', (event, success) => {
      if (!this.messageMutex) {
        this.messageMutex = true;
        const onClose = () => {
          this.messageMutex = false;
        };
        if (success) {
          message.success({
            content: `${value ? '设置' : '清除'}过滤条件成功`,
            duration: 2,
            onClose: onClose
          });
          this.setState({
            lastFilterExpr: value
          });
        } else {
          message.error({
            content: `${value ? '设置' : '清除'}过滤条件失败`,
            duration: 2,
            onClose: onClose
          });
        }
        this.setState({
          filterExpr: ''
        });
      }
    });
  };

  handleScrollUpdateSwitch = (checked: boolean) => {
    this.setState({
      scrollUpdate: checked
    });
  };

  handleViewPacketDetail = (rowIndex: number) => {
    if (this.state.packets[rowIndex].protocol.startsWith('[')) {
      if (!this.messageMutex) {
        this.messageMutex = true;
        message.warning({
          content: '不支持解析该协议',
          duration: 2,
          onClose: () => {
            this.messageMutex = false;
          }
        });
      }
      return;
    }
    this.setState({
      packetDetailPanelVisible: true,
      expandedPacketIndex: rowIndex
    });
  };

  closePacketDetailPanel = () => {
    this.setState({
      packetDetailPanelVisible: false
    });
  };

  render() {
    const {
      running,
      packets,
      scrollUpdate,
      packetDetailPanelVisible,
      expandedPacketIndex,
      filterExpr,
      lastFilterExpr
    } = this.state;
    return (
      <div>
        <div className={styles.horizontalToolbar}>
          <div className={styles.leftToolbar}>
            <Button
              type="primary"
              className={styles.actionBtn}
              disabled={running}
              onClick={this.start}
            >
              开始
            </Button>
            <Button
              type="primary"
              danger
              disabled={!running}
              className={styles.actionBtn}
              onClick={this.stop}
            >
              停止
            </Button>
            <Button
              disabled={running || packets.length === 0}
              className={styles.actionBtn}
              onClick={this.clear}
            >
              清空
            </Button>
            <Search
              value={filterExpr}
              className={styles.filterSearch}
              placeholder="输入过滤表达式"
              enterButton="过滤"
              onChange={e => {
                this.setState({
                  filterExpr: e.target.value
                });
              }}
              onSearch={this.handleFilter}
            />
            {lastFilterExpr && (
              <Tag color="blue" className={styles.filterTag}>
                {lastFilterExpr}
              </Tag>
            )}
          </div>
          <div>
            <label className={styles.switchLabel}>滚动更新:&emsp;</label>
            <Switch
              checked={scrollUpdate}
              onClick={this.handleScrollUpdateSwitch}
            />
          </div>
        </div>
        <PacketDetailPanel
          height={window.innerHeight - 360}
          data={this.rawPackets[expandedPacketIndex]}
          visible={packetDetailPanelVisible}
          onClose={this.closePacketDetailPanel}
        />
        <VirtualTable
          height={window.innerHeight - 86}
          scrollUpdate={scrollUpdate}
          dataSource={packets}
          onRowClick={this.handleViewPacketDetail}
        />
      </div>
    );
  }
}

export default withRouter(MainPage as any);
