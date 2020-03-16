import React from 'react';

interface IProps {}
interface IState {}

export default class App extends React.Component<IProps, IState> {
  constructor(props: IProps) {
    super(props);
    this.state = {};
  }

  render() {
    return <div></div>;
  }
}
