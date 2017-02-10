import * as React from 'react';
import {observer, inject} from 'mobx-react';

import Divider from 'material-ui/Divider';

import BackupSources from './BackupSources';
import BackupDestinations from './BackupDestinations';
import DestinationStore from '../stores/DestinationStore';

interface ISettingsProps {
  destinationStore: DestinationStore;
}

@observer
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