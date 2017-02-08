import * as React from 'react';
import {Card, CardActions, CardHeader, CardText} from 'material-ui/Card';
import FlatButton from 'material-ui/FlatButton';

import ActionAlarm from 'material-ui/svg-icons/action/alarm';
import AvPlayArrow from 'material-ui/svg-icons/av/play-arrow';
import NavigationExpandMore from 'material-ui/svg-icons/navigation/expand-more';
import NavigationExpandLess from 'material-ui/svg-icons/navigation/expand-less';

const timeOptions = {
  weekday: "short", year: "numeric", month: "short",
  day: "numeric", hour: "2-digit", minute: "2-digit"
};

interface ActiveBackupState {
  expanded: boolean;
  files: string[];
  startDate: Date;
}

export default class ActiveBackup extends React.Component<any, ActiveBackupState> {
  constructor(props: any) {
    super(props);
    this.state = {
      expanded: false,
      files: ["/file1.exe", "/readme.txt"],
      startDate: new Date(2016, 11, 3, 9, 30)
    };
  }

  toggleExpand = () => {
    this.setState({expanded: !this.state.expanded});
  };

  render() {
    return (
      <Card expanded={this.state.expanded}>
        <CardHeader
          title="Pending backup"
          subtitle={`Commencing ${this.state.startDate.toLocaleTimeString("en-us", timeOptions)}`}
          avatar={<ActionAlarm/>}
          actAsExpander={true}/>
        <CardActions>
          <FlatButton
            label={`Files (${this.state.files.length})`}
            icon={this.state.expanded ? <NavigationExpandLess/> : <NavigationExpandMore/>}
            onClick={this.toggleExpand}/>
          <FlatButton label="Start Now" icon={<AvPlayArrow/>}/>
        </CardActions>
        <CardText expandable={true}>
          <ul>
            {this.state.files.map(function(file, index) {
              return <li key={index}>{file}</li>;
            })}
          </ul>
        </CardText>
      </Card>
    );
  }
}