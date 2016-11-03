import React, {Component} from 'react';
import {Card, CardActions, CardHeader, CardText} from 'material-ui/Card';
import FlatButton from 'material-ui/FlatButton';

import FileCloudDone from 'material-ui/svg-icons/file/cloud-done';
import AlertWarning from 'material-ui/svg-icons/alert/warning';
import NavigationExpandMore from 'material-ui/svg-icons/navigation/expand-more';
import NavigationExpandLess from 'material-ui/svg-icons/navigation/expand-less';

const dateOptions = {
  weekday: "short", year: "numeric", month: "short",
  day: "numeric"
};

const timeOptions = {
  hour: "2-digit", minute: "2-digit"
};

export default class LastBackup extends Component {
  constructor(props) {
    super(props);
    this.state = {
      expanded: false,
      successful: true,
      files: ["/file1.exe", "/readme.txt"],
      startDate: new Date(2016, 11, 3, 7, 0),
      endDate: new Date(2016, 11, 3, 8, 15),
    };
  }

  toggleExpand = () => {
    this.setState({expanded: !this.state.expanded});
  };

  computeTimeElapseRange = () => {
    const identicalDate = this.state.startDate.getDate() === this.state.endDate.getDate();

    return this.state.startDate.toLocaleTimeString("en-us", timeOptions) + " " +
      (identicalDate ? "" : this.state.startDate.toLocaleString("en-us", dateOptions)) + " - " +
      this.state.endDate.toLocaleTimeString("en-us", timeOptions) + " " +
      this.state.endDate.toLocaleString("en-us", dateOptions);
  };

  render() {
    return (
      <Card expanded={this.state.expanded}>
        <CardHeader
          title="Last backup"
          subtitle={this.computeTimeElapseRange()}
          avatar={this.state.successful ? <FileCloudDone/> : <AlertWarning/>}
          actAsExpander={true}/>
        <CardActions>
          <FlatButton
            label={`Files (${this.state.files.length})`}
            icon={this.state.expanded ? <NavigationExpandLess/> : <NavigationExpandMore/>}
            onClick={this.toggleExpand}/>
        </CardActions>
        <CardText expandable={true}>
          <ul>
            {this.state.files.map(function (file, index) {
              return <li key={index}>{file}</li>;
            })}
          </ul>
        </CardText>
      </Card>
    );
  }
}