import * as React from 'react';

import FlatButton from 'material-ui/FlatButton';
import {List, ListItem} from 'material-ui/List';

import ContentAdd from 'material-ui/svg-icons/content/add';
import FileFolder from 'material-ui/svg-icons/file/folder';

interface Source {
  name: string;
  path: string;
}

function createSource(path: string): Source {
  return {name: path.substr(path.lastIndexOf("\\") + 1), path: path}
}

interface BackupSourcesState {
  sources: Source[];
}

export default class BackupSources extends React.Component<any, BackupSourcesState> {
  constructor(props: any) {
    super(props);
    this.state = {
      sources: [createSource("C:\\User\\zsims\\Documents"), createSource("U:\\Movies")],
    };
  }

  render() {
    return (
      <div>
        <h3>Locations to backup</h3>
        <List>
          {this.state.sources.map(function (source) {
            return <ListItem
              key={source.path}
              primaryText={source.name}
              secondaryText={source.path}
              leftIcon={<FileFolder/>}
            />;
          })
          }
        </List>
        <FlatButton label="Add location" icon={<ContentAdd/>}/>
      </div>
    );
  }
}