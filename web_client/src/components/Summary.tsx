import * as React from 'react';

import ActiveBackup from './ActiveBackup';
import LastBackup from './LastBackup';
import StorageStatistics from './StorageStatistics';

export default class Summary extends React.Component<any, any> {
  render() {
    return (
      <div className="Summary">
        <ActiveBackup/>
        <LastBackup/>
        <StorageStatistics/>
      </div>
    );
  }
}