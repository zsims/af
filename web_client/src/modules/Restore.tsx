import * as React from 'react';

import FlatButton from 'material-ui/FlatButton';
import {List, ListItem} from 'material-ui/List';
import {Toolbar, ToolbarGroup, ToolbarSeparator, ToolbarTitle} from 'material-ui/Toolbar';

import ActionDescription from 'material-ui/svg-icons/action/description';
import ActionSearch from 'material-ui/svg-icons/action/search';
import FileDownload from 'material-ui/svg-icons/file/file-download';
import FileFolder from 'material-ui/svg-icons/file/folder';
import MailOutline from 'material-ui/svg-icons/communication/mail-outline'
import ArrowBack from 'material-ui/svg-icons/navigation/arrow-back'

export default class Restore extends React.Component<any, any> {
  constructor() {
    super();
    this.state = {
      directory: "Documents",
      folders: [
        {name: "Tax"}
      ],
      files: [
        {name: "Samson.jpg", size: "21.1KB", type: (<ActionDescription/>)},
        {name: "Zac's secret plan.eml", size: "2.3MB", type: (<MailOutline/>)}
      ]
    };
  }

  render() {
    return (
        <div>
          <Toolbar>
            <ToolbarGroup firstChild={true}>
              <FlatButton icon={<ArrowBack/>}/>
              <ToolbarTitle text={this.state.directory}/>
            </ToolbarGroup>
            <ToolbarGroup lastChild={true}>
              <ToolbarSeparator />
              <FlatButton icon={<ActionSearch/>}/>
            </ToolbarGroup>
          </Toolbar>
          <List>
            {this.state.folders.map(function (folder: any) {
              return <ListItem
                key={folder.name}
                primaryText={folder.name}
                leftIcon={<FileFolder/>}
                rightIcon={<FileDownload/>}
              />;
            })}
            {this.state.files.map(function (file: any) {
              return <ListItem
                key={file.name}
                primaryText={file.name}
                secondaryText={file.size}
                leftIcon={file.type}
                rightIcon={<FileDownload/>}
              />;
            })}
          </List>
        </div>
    );
  }
}