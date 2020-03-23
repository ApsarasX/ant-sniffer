import React from 'react';
// import { ipcRenderer } from 'electron';

interface IProps {}
interface IState {
  list: string[];
}

export default class WelcomePage extends React.Component<IProps, IState> {
  constructor(props: IProps) {
    super(props);
    this.state = {
      list: []
    };
  }

  static getDerivedStateFromProps() {
    // const devList = ipcRenderer.sendSync('getAllDevices');
    return {
      list: ['en1', 'en2']
    };
  }
  
  render() {
    return (
      <div>
        <ul>
          {this.state.list.map(dev => (
            <li key={dev}>{dev}</li>
          ))}
        </ul>
      </div>
    );
  }
}
