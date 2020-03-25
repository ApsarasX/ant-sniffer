import React, { useEffect, useRef, useState } from 'react';
import { VariableSizeList as List } from 'react-window';
import './index.less';

function VirtualTable(props) {
  const { columns, height, dataSource, onRowClick, scrollUpdate } = props;
  const listRef = useRef();
  const [showScrollbar, setShowScrollbar] = useState(false);
  const resetVirtualList = () => {
    listRef.current.resetAfterIndex(0);
  };

  useEffect(() => resetVirtualList, []);
  useEffect(() => {
    if (scrollUpdate && !listRef.current.state.isScrolling) {
      listRef.current.scrollToItem(listRef.current.props.itemCount);
    }
    if (!showScrollbar && listRef.current.props.itemCount > height / 28) {
      setShowScrollbar(true);
    }
  }, [scrollUpdate, dataSource, showScrollbar, height]);


  return (
    <div className="virtual-table">
      <table className="virtual-table-header">
        <thead>
          <tr>
            {columns.map((column, columnIndex) => (
              <th
                key={`vt-hdrcol-${columnIndex}`}
                className="virtual-table-hdrcell"
                style={column.style}
              >
                {column.title}
              </th>
            ))}
            {showScrollbar && (
              <th className="virtual-table-hdrcell-scrollbar"></th>
            )}
          </tr>
        </thead>
      </table>
      <List
        ref={listRef}
        height={height}
        itemCount={dataSource.length}
        itemSize={() => 28}
        overscanCount={80}
      >
        {({ index: rowIndex, style }) => (
          <div
            className="virtual-table-row"
            style={style}
            onClick={onRowClick.bind(this, rowIndex)}
          >
            {columns.map((column, columnIndex) => (
              <div
                className="virtual-table-cell"
                key={`cell-${rowIndex}-${columnIndex}`}
                style={column.style}
                dangerouslySetInnerHTML={{
                  __html: dataSource[rowIndex][column.dataIndex]
                }}
              />
            ))}
          </div>
        )}
      </List>
    </div>
  );
}

const columns = [
  {
    title: '编号',
    dataIndex: 'no',
    style: {
      flex: 0.4
    }
  },
  {
    title: '时间',
    dataIndex: 'time'
  },
  {
    title: '来源',
    dataIndex: 'source'
  },
  {
    title: '目的',
    dataIndex: 'destination'
  },
  {
    title: '协议',
    dataIndex: 'protocol'
  },
  {
    title: '长度',
    dataIndex: 'length',
    style: {
      flex: 0.2,
      textAlign: 'right'
    }
  },
  {
    title: '信息',
    dataIndex: 'info',
    style: {
      flex: 5
    }
  }
];

export default function(props) {
  return <VirtualTable {...props} columns={columns} />;
}
