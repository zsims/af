import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {Provider} from 'mobx-react';

import App from './App';
import SystemLogs from './components/SystemLogs';
import Restore from './components/Restore';
import Settings from './components/Settings';
import Summary from './components/Summary';
import * as injectTapEventPlugin from 'react-tap-event-plugin';

import DestinationStore from './stores/DestinationStore';

import './index.css';

import {Router, Route, hashHistory, IndexRoute} from 'react-router';

const stores = {
  destinationStore: new DestinationStore()
}

// Needed for onTouchTap
// http://www.material-ui.com/#/get-started/installation
injectTapEventPlugin();

ReactDOM.render((
    <Provider {...stores}>
      <Router history={hashHistory}>
        <Route path="/" component={App}>
          <IndexRoute component={Summary}/>
          <Route path="/restore" component={Restore}/>
          <Route path="/settings" component={Settings}/>
          <Route path="/systemlogs" component={SystemLogs}/>
        </Route>
      </Router>
    </Provider>
  ), document.getElementById('root')
);
