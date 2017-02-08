import * as React from 'react';
import {observer, inject} from 'mobx-react';

import Divider from 'material-ui/Divider';

import BackupSources from './BackupSources';
import BackupDestinations from './BackupDestinations';

interface ISettingsProps {
  destinationStore: any;
}

@inject('destinationStore') @observer
export default class Settings extends React.Component<ISettingsProps, any> {
  render() {
    return (
      <div className="Settings">
        <BackupSources/>
        <Divider/>
        <BackupDestinations destinationStore={this.props.destinationStore}/>
      </div>
    );
  }
}