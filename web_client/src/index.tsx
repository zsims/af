import * as React from 'react';
import * as ReactDOM from 'react-dom';

import App from './App';
import SystemLogs from './components/SystemLogs';
import Restore from './components/Restore';
import Settings from './components/Settings';
import Summary from './components/Summary';
import * as injectTapEventPlugin from 'react-tap-event-plugin';

import './index.css';

import {Router, Route, hashHistory, IndexRoute} from 'react-router';

// Needed for onTouchTap
// http://www.material-ui.com/#/get-started/installation
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
