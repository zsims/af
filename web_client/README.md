# Web client
This is the web client for af. Built using TypeScript and React.

# Building

 1. Install dependencies: `npm install`
 2. Build into `./dist`: `npm run build`

A dev server can be started with: `npm start`. Once running, visit http://localhost:3000

# Testing

Tests are written using:

  * [Mocha](https://mochajs.org) as the framework and runner;
  * [Chai](http://chaijs.com/) as the assertion library; and
  * [Sinon](http://sinonjs.org/) for mocks and stubs; and
  * [Enzyme](https://github.com/airbnb/enzyme) for shallow rendering of React components

To run tests:

 * `npm test` to run all the tests once
 * `npm run test:watch` to run the tests on changes
 * `npm run test:xunit` to produce a `test_results.xml` file in XUnit format
