import React, {Component} from 'react';
import './App.css';

import MuiThemeProvider from 'material-ui/styles/MuiThemeProvider';
import getMuiTheme from 'material-ui/styles/getMuiTheme';
import AppBar from 'material-ui/AppBar';
import Menu from 'material-ui/Menu';
import MenuItem from 'material-ui/MenuItem';

import Divider from 'material-ui/Divider';

import ActionInfo from 'material-ui/svg-icons/action/info';
import ActionRestore from 'material-ui/svg-icons/action/restore';
import ActionSettings from 'material-ui/svg-icons/action/settings';
import AvLibraryBooks from 'material-ui/svg-icons/av/library-books';

const muiTheme = getMuiTheme({
  appBar: {
    height: 50,
  },
});

const style = {
  menu: {
    float: 'left'
  }
};


export default class App extends Component {
  static contextTypes = {
    router: React.PropTypes.object
  };

  state = {
    value: "/"
  };

  navigateTo = (event, value) => {
    this.setState({value: value});
    this.context.router.push(value);
  };

  render() {
    return (
      <MuiThemeProvider muiTheme={muiTheme}>
        <div className="App">
          <AppBar title="af backup" showMenuIconButton={false}/>
          <Menu style={style.menu} onChange={this.navigateTo} value={this.state.value}>
            <MenuItem value="/" leftIcon={<ActionInfo/>}>Summary</MenuItem>
            <MenuItem value="/restore" leftIcon={<ActionRestore/>}>Restore files</MenuItem>
            <Divider />
            <MenuItem value="/settings" leftIcon={<ActionSettings/>}>Settings</MenuItem>
            <MenuItem value="/systemlogs" leftIcon={<AvLibraryBooks/>}>System Logs</MenuItem>
          </Menu>
          <div style={{marginLeft: "250px"}}>
          {this.props.children}
          </div>
        </div>
      </MuiThemeProvider>
    );
  }
}
