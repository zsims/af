import * as React from 'react';
import {observer, inject} from 'mobx-react';
import {Link} from 'react-router';

import FlatButton from 'material-ui/FlatButton';
import {List, ListItem} from 'material-ui/List';

import ContentAdd from 'material-ui/svg-icons/content/add';
import ContentArchive from 'material-ui/svg-icons/content/archive';
import DestinationStore from '../stores/DestinationStore';

interface BackupDestinationsProps {
  destinationStore: DestinationStore;
}

@observer
export default class BackupDestinations extends React.Component<BackupDestinationsProps, any> {
  constructor(props: any) {
    super(props);
  }

  render() {
    return (
      <div>
        <h3>Backup destinations</h3>
        <List>
          {this.props.destinationStore.destinations.map(function (destination) {
            return <ListItem
              key={destination.id}
              primaryText={destination.name}
              secondaryText={destination.summary}
              leftIcon={<ContentArchive/>}
            />;
          })}
        </List>
        <FlatButton label="Add destination" icon={<ContentAdd/>} containerElement={<Link to="/settings/adddestination" />} linkButton={true} />
      </div>
    );
  }
}