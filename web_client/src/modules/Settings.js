import React, {Component} from 'react';

import Divider from 'material-ui/Divider';

import BackupSources from './BackupSources';
import BackupDestinations from './BackupDestinations';

export default class Settings extends Component {
  render() {
    return (
      <div className="Settings">
        <BackupSources/>
        <Divider/>
        <BackupDestinations/>
      </div>
    );
  }
}