import React from 'react';
import { BrowserRouter, Route, Redirect, Switch } from 'react-router-dom';
import WelcomePage from './pages/welcome';
import MainPage from './pages/main';

export default class App extends React.Component {
  render() {
    return (
      <BrowserRouter>
        <Switch>
          <Route exact path="/welcome">
            <WelcomePage />
          </Route>
          <Route exact path="/main">
            <MainPage />
          </Route>
          <Route path="*">
            <Redirect to="/welcome" />
          </Route>
        </Switch>
      </BrowserRouter>
    );
  }
}
