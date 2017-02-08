import * as React from 'react';
import {observer, inject} from 'mobx-react';

import FlatButton from 'material-ui/FlatButton';
import RaisedButton from 'material-ui/RaisedButton';
import {List, ListItem, makeSelectable} from 'material-ui/List';
import TextField from 'material-ui/TextField';
import FileFolder from 'material-ui/svg-icons/file/folder-open';
import Lens from 'material-ui/svg-icons/image/lens';
import {Step, Stepper, StepLabel} from 'material-ui/Stepper';

import {
  IDestinationModel,
  NullDestinationModel,
  DirectoryDestinationModel,
  TYPE_NULL,
  TYPE_DIRECTORY} from '../models/DestinationModel';
import DestinationStore from '../stores/DestinationStore';

class NullSettings extends React.Component<{
  destinationUpdated: (destination: IDestinationModel) => void;
}, any> {
  constructor(props: any) {
    super(props);
    props.destinationUpdated(new NullDestinationModel());
  }

  render() {
    return <i>Nothing to configure</i>;
  }
}

class DirectorySettings extends React.Component<{
  destination: DirectoryDestinationModel,
  destinationUpdated: (destination: IDestinationModel) => void;
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
      this.props.destinationUpdated(null);
      return;
    }
    else {
      this.setState({
        pathError: '',
        path: value
      });
      var destination = new DirectoryDestinationModel();
      destination.settings.path = value;
      this.props.destinationUpdated(destination);
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

interface AddBackupDestinationState {
    stepIndex: number;
    destinationType: string;
    destination?: IDestinationModel
}

interface AddBackupDestinationProps {
    destinationStore: DestinationStore;
    router: any;
}

@inject('destinationStore') @observer
export default class AddBackupDestination extends React.Component<AddBackupDestinationProps, AddBackupDestinationState> {
  constructor(props: any) {
    super(props);
    this.state = {
      stepIndex: 0,
      destinationType: TYPE_DIRECTORY,
      destination: null
    }
  }

  handleCreate = () => {
      this.props.destinationStore.createDestination(this.state.destination);
      this.props.router.push('/settings');
  };

  handlePrev = () => {
    const {stepIndex} = this.state;
    if (stepIndex > 0) {
      this.setState({stepIndex: stepIndex - 1});
    }
  };

  handleSelectDirectoryType = () => {
      const {stepIndex} = this.state;
      this.setState({
        stepIndex: stepIndex + 1,
        destination: null,
        destinationType: TYPE_DIRECTORY,
      });
  };

  handleSelectNullType = () => {
      const {stepIndex} = this.state;
      this.setState({
        stepIndex: stepIndex + 1,
        destination: null,
        destinationType: TYPE_NULL
      });
  }

  onDestinationUpdated = (destination: IDestinationModel) => {
    this.setState({
      destination: destination
    });
  };

  canCreate() {
    return this.state.stepIndex === 1 && this.state.destination !== null;
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
            return <NullSettings destinationUpdated={this.onDestinationUpdated} />
          case TYPE_DIRECTORY:
          default:
            return <DirectorySettings destination={new DirectoryDestinationModel()} destinationUpdated={this.onDestinationUpdated} />
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
                disabled={stepIndex === 0}
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