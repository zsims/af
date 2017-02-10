import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as TestUtils from 'react-addons-test-utils';
import { Router } from 'react-router';

import 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';
import { shallow, mount, CommonWrapper } from 'enzyme';

import getMuiTheme from 'material-ui/styles/getMuiTheme';
import { ListItem } from 'material-ui/List';

import DestinationStore from '../stores/DestinationStore';
import { AddBackupDestination, IAddBackupDestinationState, DirectorySettings } from './AddBackupDestination';
import { 
  DestinationModel,
  DestinationSettings,
  DestinationType,
  TYPE_NULL,
  TYPE_DIRECTORY } from '../models/DestinationModel';

describe('<AddBackupDestination />', () => {
  const muiTheme = getMuiTheme();
  const mountWithContext = (node: React.ReactElement<any>) => mount(node, {
    context: { muiTheme },
    childContextTypes: { muiTheme: React.PropTypes.object },
  });
  var destinationStoreStub:  DestinationStore;
  var routerStub: Router;
  const currentState = (wrapper: CommonWrapper<any, {}>) => wrapper.state() as IAddBackupDestinationState;

  beforeEach(() => {
    destinationStoreStub = {} as DestinationStore;
    routerStub = {} as Router;
  });

  it('Select directory type moves to step 2', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;

    // Act
    instance.handleSelectDirectoryType();

    // Assert
    wrapper.update();
    const state = currentState(wrapper);
    expect(state.destinationType).to.equal(TYPE_DIRECTORY);
    expect(state.stepIndex).to.equal(1);
  });

  it('Select null type moves to step 2', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;

    // Act
    instance.handleSelectNullType();

    // Assert
    wrapper.update();
    const state = currentState(wrapper);
    expect(state.destinationType).to.equal(TYPE_NULL);
    expect(state.stepIndex).to.equal(1);
  });

  it('Back disabled on first page', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;

    // Act
    const canGoBack = instance.canGoBack();

    // Assert
    expect(canGoBack).to.be.false;
  });

  it('Back enabled on second page', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;
    instance.setState({
      stepIndex: 1
    });

    // Act
    const canGoBack = instance.canGoBack();

    // Assert
    expect(canGoBack).to.be.true;
  });

  // Disabled, as 'touchTap' is not a simulatable event yet
  xit('Click directory type changes page', () => {
    // Arrange
    const wrapper = mountWithContext(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);

    // Act
    const listItems = wrapper.find(ListItem);
    expect(listItems.length).to.equal(2);
    const directoryItem = listItems.first();
    directoryItem.simulate('touchTap');

    // Assert
    wrapper.update();
    const state = currentState(wrapper);
    expect(state.destinationType).to.equal(TYPE_DIRECTORY);
    expect(state.stepIndex).to.equal(1);
  });

  it('Create disabled if not setup', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;

    // Act
    const canCreate = instance.canCreate();

    // Assert
    expect(canCreate).to.be.false;
  });

  it('Create enabled if setup', () => {
    // Arrange
    const wrapper = shallow(<AddBackupDestination destinationStore={destinationStoreStub} router={routerStub} />);
    const instance = wrapper.instance() as AddBackupDestination;
    instance.setState({
      stepIndex: 1,
      destinationSettings: {}
    });

    // Act
    const canCreate = instance.canCreate();

    // Assert
    expect(canCreate).to.be.true;
  });
});

describe('<DirectorySettings />', () => {
  const currentState = (wrapper: CommonWrapper<any, {}>) => wrapper.state() as any;

  it('updatePath sets error if path missing', function() {
    // Arrange
    const callbackSpy= sinon.spy();
    const wrapper = shallow(<DirectorySettings destinationSettingsUpdated={callbackSpy} />);
    const instance = wrapper.instance() as DirectorySettings;

    // Act
    instance.updatePath('');

    // Assert
    wrapper.update();
    var state = currentState(wrapper);
    expect(state.pathError).to.not.be.empty;
  });

  it('updatePath builds model on success', () => {
    // Arrange
    const callbackSpy= sinon.spy();
    const wrapper = shallow(<DirectorySettings destinationSettingsUpdated={callbackSpy} />);
    const instance = wrapper.instance() as DirectorySettings;
    const path = '/home/satvik';

    // Act
    instance.updatePath(path);

    // Assert
    wrapper.update();
    var state = currentState(wrapper);
    expect(state.pathError).to.be.empty;
    expect(state.path).to.be.equal(path);
    expect(callbackSpy.called).to.be.ok;
  });
});