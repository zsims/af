import * as React from 'react';
import { shallow } from 'enzyme';
import App from './App';
import { expect } from 'chai';
import 'mocha';

describe('App', () => {
  it('renders without crashing', () => {
    // Act
    const wrapper = shallow(<App />);

    // Assert
    //See https://github.com/airbnb/enzyme
    expect(wrapper).not.to.be.a('string');
  })
});