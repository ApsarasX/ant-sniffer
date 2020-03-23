import React from 'react';
import { Drawer, Tree } from 'antd';
import genTreedata from './gen-treedata';

interface IProps {
  visible: boolean;
  data: any;
  onClose: () => void;
}
interface IState {
  expandedKeys: string[];
  treeData: any;
}

export default class PacketDetailPanel extends React.Component<IProps, IState> {
  constructor(props: IProps) {
    super(props);
    this.state = {
      expandedKeys: [],
      treeData: []
    };
  }

  static getDerivedStateFromProps(newProps: any) {
    const { data } = newProps;
    if (!data) {
      return null;
    }
    return {
      treeData: genTreedata(data)
    };
  }

  handleDrawerClose = () => {
    this.setState({ expandedKeys: [] });
    this.props.onClose();
  };

  handleExpandedKeysUpdate = (keys: any) => {
    this.setState({
      expandedKeys: keys
    });
  };

  render() {
    const { visible } = this.props;
    const { expandedKeys, treeData } = this.state;
    return (
      <Drawer
        visible={visible}
        placement="bottom"
        closable={false}
        height={window.innerHeight / 2}
        onClose={this.handleDrawerClose}
      >
        <Tree
          blockNode={true}
          multiple={true}
          defaultExpandedKeys={expandedKeys}
          defaultSelectedKeys={expandedKeys}
          selectedKeys={expandedKeys}
          expandedKeys={expandedKeys}
          onSelect={this.handleExpandedKeysUpdate}
          onExpand={this.handleExpandedKeysUpdate}
          treeData={treeData}
          style={{ userSelect: 'none', fontWeight: 500 }}
        />
        {
          <pre
            style={{
              paddingLeft: '52px',
              display: this.state.expandedKeys.includes('payload')
                ? 'block'
                : 'none'
            }}
          >
            <span>{this.props.data?.payload?.payload?.payload}</span>
          </pre>
        }
      </Drawer>
    );
  }
}
