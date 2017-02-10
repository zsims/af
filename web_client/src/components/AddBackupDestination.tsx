import * as React from 'react';
import {observer, inject} from 'mobx-react';

import FlatButton from 'material-ui/FlatButton';
import RaisedButton from 'material-ui/RaisedButton';
import {List, ListItem} from 'material-ui/List';
import TextField from 'material-ui/TextField';
import FileFolder from 'material-ui/svg-icons/file/folder-open';
import Lens from 'material-ui/svg-icons/image/lens';
import {Step, Stepper, StepLabel} from 'material-ui/Stepper';

import {
  DestinationModel,
  DestinationType,
  DestinationSettings,
  TYPE_NULL,
  TYPE_DIRECTORY} from '../models/DestinationModel';
import DestinationStore from '../stores/DestinationStore';

class NullSettings extends React.Component<any, any> {
  render() {
    return <i>Nothing to configure</i>;
  }
}

export class DirectorySettings extends React.Component<{
  destinationSettingsUpdated: (destinationSettings: DestinationSettings) => void;
}, {path: string, pathError: string}> {
  constructor(props: any) {
    super(props);
    this.state = {
      path: '',
      pathError: ''
    };
  }

  onChangePath = (event: React.ChangeEvent<any>) => {
    this.updatePath(event.target.value);
  }

  updatePath(value: string) {
    if(value === null || value.length < 1) {
      this.setState({
        pathError: 'Full Path is Required',
        path: value
      });
      this.props.destinationSettingsUpdated(null);
    }
    else {
      this.setState({
        pathError: '',
        path: value
      });
      this.props.destinationSettingsUpdated({
        path: value
      });
    }
  }

  render() {
    return (
      <TextField
        value={this.state.path}
        floatingLabelText="Full Path to Directory"
        floatingLabelFixed={false}
        errorText={this.state.pathError}
        onChange={this.onChangePath} />
    );
  }
}

export interface IAddBackupDestinationState {
    stepIndex: number;
    destinationType: DestinationType;
    destinationSettings: DestinationSettings;
}

export interface IAddBackupDestinationProps {
    destinationStore: DestinationStore;
    router: any;
}

@observer
export class AddBackupDestination extends React.Component<IAddBackupDestinationProps, IAddBackupDestinationState> {
  constructor(props: any) {
    super(props);
    this.state = {
      stepIndex: 0,
      destinationType: TYPE_DIRECTORY,
      destinationSettings: null
    }
  }

  handleCreate = () => {
      this.props.destinationStore.createDestination(this.state.destinationType, this.state.destinationSettings);
      this.props.router.push('/settings');
  };

  handlePrev = () => {
    const {stepIndex} = this.state;
    if (stepIndex > 0) {
      this.setState({stepIndex: stepIndex - 1});
    }
  };

  handleSelectDirectoryType = () => {
      this.setState({
        stepIndex: this.state.stepIndex + 1,
        destinationType: TYPE_DIRECTORY,
        destinationSettings: null
      });
  };

  handleSelectNullType = () => {
      this.setState({
        stepIndex: this.state.stepIndex + 1,
        destinationType: TYPE_NULL,
        destinationSettings: {}
      });
  }

  onDestinationSettingsUpdated = (destinationSettings: DestinationSettings) => {
    this.setState({
      destinationSettings: destinationSettings
    });
  };

  canCreate() {
    return this.state.stepIndex === 1 && this.state.destinationSettings !== null;
  }

  canGoBack() {
    return this.state.stepIndex > 0;
  }

  renderDestinationType() {
    return (<List>
      <ListItem
        primaryText="Null"
        secondaryText="A store that doesn't persist data, useful for testing"
        leftIcon={<Lens/>}
        onTouchTap={this.handleSelectNullType} />
      <ListItem
        primaryText="Directory"
        secondaryText="A local or network mounted directory"
        leftIcon={<FileFolder/>}
        onTouchTap={this.handleSelectDirectoryType} />
    </List>);
  }

  getStepContent(stepIndex: number) {
    switch (stepIndex) {
      case 1:
        switch(this.state.destinationType) {
          case TYPE_NULL:
            return <NullSettings />
          case TYPE_DIRECTORY:
          default:
            return <DirectorySettings destinationSettingsUpdated={this.onDestinationSettingsUpdated} />
        }
      case 0:
      default:
        return this.renderDestinationType();
    }
  }

  render() {
    const {stepIndex} = this.state;
    return (
      <div>
        <Stepper activeStep={stepIndex}>
          <Step>
            <StepLabel>Destination Type</StepLabel>
          </Step>
          <Step>
            <StepLabel>Destination Settings</StepLabel>
          </Step>
        </Stepper>
        <div>
          <div>
            {this.getStepContent(stepIndex)}
            <div style={{ marginTop: 12 }}>
              <FlatButton
                label="Back"
                disabled={!this.canGoBack()}
                onTouchTap={this.handlePrev}
                style={{ marginRight: 12 }} />
                <FlatButton
                label="Create"
                disabled={!this.canCreate()}
                onTouchTap={this.handleCreate} />
            </div>
          </div>
        </div>
      </div>
    );
  }
}