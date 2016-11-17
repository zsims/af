import React, {Component} from 'react';

import FlatButton from 'material-ui/FlatButton';
import {List, ListItem} from 'material-ui/List';

import ContentAdd from 'material-ui/svg-icons/content/add';
import NotificationDriveEta from 'material-ui/svg-icons/notification/drive-eta';

export default class BackupDestinations extends Component {
  constructor(props) {
    super(props);
    this.state = {
      sources: [{name: "Local Folder", path: "B:\\Backup"}],
    };
  }

  render() {
    return (
      <div>
        <h3>Backup destinations</h3>
        <List>
          {this.state.sources.map(function (source) {
            return <ListItem
              key={source.path}
              primaryText={source.name}
              secondaryText={source.path}
              leftIcon={<NotificationDriveEta/>}
            />;
          })}
        </List>
        <FlatButton label="Add destination" icon={<ContentAdd/>}/>
      </div>
    );
  }
}