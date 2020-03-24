import React from 'react';
import { withRouter } from 'react-router-dom';
import { Button, Table, Tag } from 'antd';
import { ipcRenderer } from 'electron';
import styles from './style.module.less';

const { Column } = Table;

interface IProps {
  history: any;
}
interface IState {
  devList: any[];
}

class WelcomePage extends React.Component<IProps, IState> {
  constructor(props: IProps) {
    super(props);
    this.state = {
      devList: []
    };
  }

  componentDidMount() {
    this.fetchDeviceListData();
  }

  fetchDeviceListData() {
    const devList = ipcRenderer.sendSync('getAllDevices');

    for (let i = 0; i < devList.length; ) {
      if (devList[i].addrs) {
        const addrs = devList[i].addrs;
        const addrDevs = addrs.map((addr: any, idx: number) => ({
          ...devList[i],
          addr: addr,
          key: `${devList[i].name}-${idx}`,
          rowSpan: idx === 0 ? addrs.length : 0
        }));
        devList.splice(i, 1, ...addrDevs);
        i += addrs.length;
      } else {
        devList[i].key = devList[i].name;
        i += 1;
      }
    }
    this.setState({
      devList: devList
    });
  }

  renderContent = (value: any, row: any) => {
    const obj = {
      children: value,
      props: {
        rowSpan: 1
      }
    };
    if (row.hasOwnProperty('rowSpan')) {
      obj.props.rowSpan = row.rowSpan;
    }
    return obj;
  };

  renderTagsContent = (_: any, dev: any) => {
    const obj = {
      children: (
        <>
          {dev?.loopback && <Tag color="geekblue">回环设备</Tag>}
          {dev?.wireless && <Tag color="purple">无线设备</Tag>}
          {dev?.up && <Tag color="blue">已启动</Tag>}
          {dev?.running && <Tag color="green">运行中</Tag>}
        </>
      ),
      props: {
        rowSpan: 1
      }
    };
    if (dev.hasOwnProperty('rowSpan')) {
      obj.props.rowSpan = dev.rowSpan;
    }
    return obj;
  };

  renderActionsContent = (_: any, dev: any) => {
    const disabled = !(dev.up && dev.running && dev.connStatus === '已连接');
    const obj = {
      children: (
        <Button
          type="primary"
          disabled={disabled}
          onClick={this.handleClickDevice.bind(this, dev.name)}
        >
          {disabled ? '不可抓取' : '开始抓包'}
        </Button>
      ),
      props: {
        rowSpan: 1
      }
    };
    if (dev.hasOwnProperty('rowSpan')) {
      obj.props.rowSpan = dev.rowSpan;
    }
    return obj;
  };

  handleClickDevice = (devName: string) => {
    this.props.history.push(`/main?dev=${devName}`);
  };

  render() {
    const { devList } = this.state;
    return (
      <div className={styles.welcomePage}>
        <Table
          dataSource={devList}
          pagination={false}
          bordered={true}
          title={() => <h3 className={styles.tableTitle}>本机网络设备列表</h3>}
        >
          <Column
            title="设备名"
            dataIndex="name"
            align="center"
            render={this.renderContent}
          />
          <Column
            title="MAC"
            dataIndex="mac"
            align="center"
            render={this.renderContent}
          />
          <Column
            title="IP"
            key="ip"
            dataIndex={['addr', 'ip']}
            align="center"
          />
          <Column
            title="掩码"
            key="netmask"
            dataIndex={['addr', 'netmask']}
            align="center"
          />
          <Column
            title="广播"
            key="broadcast"
            dataIndex={['addr', 'broadcast']}
            align="center"
          />
          <Column title="标签" key="tags" render={this.renderTagsContent} />
          <Column
            title="连接状态"
            dataIndex="connStatus"
            align="center"
            render={this.renderContent}
          />
          <Column
            title="操作"
            key="action"
            align="center"
            render={this.renderActionsContent}
          />
        </Table>
      </div>
    );
  }
}

export default withRouter(WelcomePage as any);
