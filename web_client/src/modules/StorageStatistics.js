import React, {Component} from 'react';
import {Card, CardActions, CardHeader, CardText} from 'material-ui/Card';
import FlatButton from 'material-ui/FlatButton';

import ActionTrendingUp from 'material-ui/svg-icons/action/trending-up';
import NavigationExpandMore from 'material-ui/svg-icons/navigation/expand-more';
import NavigationExpandLess from 'material-ui/svg-icons/navigation/expand-less';

import LineChart from './LineChart';

export default class StorageStatistics extends Component {
  constructor(props) {
    super(props);
    this.state = {
      expanded: false,
      backupTitle: "Total backup size: 125Gb",
      fileCount: "15,000 files",
    };
  }

  toggleExpand = () => {
    this.setState({expanded: !this.state.expanded});
  };

  render() {
    return (
      <Card expanded={this.state.expanded}>
        <CardHeader
          title={this.state.backupTitle}
          subtitle={this.state.fileCount}
          avatar={<ActionTrendingUp/>}
          actAsExpander={true}/>
        <CardActions>
          <FlatButton
            label="Detail statistics"
            icon={this.state.expanded ? <NavigationExpandLess/> : <NavigationExpandMore/>}
            onClick={this.toggleExpand}/>
        </CardActions>
        <CardText expandable={true}>
          <LineChart/>
        </CardText>
      </Card>
    );
  }
}