import React from 'react';
import ReactDOM from 'react-dom';

import App from './App';
import SystemLogs from './modules/SystemLogs';
import Restore from './modules/Restore';
import Settings from './modules/Settings';
import Summary from './modules/Summary';

import './index.css';

import {Router, Route, hashHistory, IndexRoute} from 'react-router';

import injectTapEventPlugin from 'react-tap-event-plugin';
injectTapEventPlugin();

ReactDOM.render((
    <Router history={hashHistory}>
      <Route path="/" component={App}>
        <IndexRoute component={Summary}/>

        <Route path="/restore" component={Restore}/>
        <Route path="/settings" component={Settings}/>
        <Route path="/systemlogs" component={SystemLogs}/>
      </Route>
    </Router>
  ), document.getElementById('root')
);
