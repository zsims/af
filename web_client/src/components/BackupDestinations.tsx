import * as React from 'react';
import {observer, inject} from 'mobx-react';
import {Link} from 'react-router';

import FlatButton from 'material-ui/FlatButton';
import {List, ListItem} from 'material-ui/List';

import ContentAdd from 'material-ui/svg-icons/content/add';
import FileFolder from 'material-ui/svg-icons/file/folder-open';
import Lens from 'material-ui/svg-icons/image/lens';
import DestinationStore from '../stores/DestinationStore';
import { DestinationModel, TYPE_NULL, TYPE_DIRECTORY } from '../models/DestinationModel';

interface BackupDestinationsProps {
  destinationStore: DestinationStore;
}

class NullItem extends React.Component<{ destination: DestinationModel }, any> {
  render() {
    return <ListItem
      key={this.props.destination.id}
      primaryText="Null"
      leftIcon={<Lens />} />;
  }
}

class DirectoryItem extends React.Component<{ destination: DestinationModel }, any> {
  constructor(props: any) {
    super(props);
  }

  render() {
    return <ListItem
      key={this.props.destination.id}
      primaryText="Directory"
      secondaryText={this.props.destination.settings['path']}
      leftIcon={<FileFolder />} />;
  }
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
            switch(destination.type) {
              case TYPE_NULL:
                return <NullItem destination={destination} />;

              case TYPE_DIRECTORY:
                return <DirectoryItem destination={destination} />;
            }
          })}
        </List>
        <FlatButton label="Add destination" icon={<ContentAdd/>} containerElement={<Link to="/settings/adddestination" />} linkButton={true} />
      </div>
    );
  }
}