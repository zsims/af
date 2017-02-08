import * as React from 'react';

import Divider from 'material-ui/Divider';

import BackupSources from './BackupSources';
import BackupDestinations from './BackupDestinations';

export default class Settings extends React.Component<any, any> {
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