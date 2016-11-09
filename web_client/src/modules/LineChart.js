import React, {Component} from 'react';
import ReactDOM from 'react-dom';

import Chart from 'chart.js'

export default class LineChart extends Component {
  constructor(props) {
    super(props);
    this.state = {
      data: {
        labels: ["January", "February", "March", "April", "May", "June", "July"],
        datasets: [
          {
            label: "Total backup size",
            lineTension: 0.1,
            backgroundColor: "rgba(75,192,192,0.4)",
            borderColor: "rgba(75,192,192,1)",
            pointBorderColor: "rgba(75,192,192,1)",
            pointBackgroundColor: "#fff",
            pointHoverBackgroundColor: "rgba(75,192,192,1)",
            pointHoverBorderColor: "rgba(220,220,220,1)",
            pointHoverBorderWidth: 2,
            pointRadius: 1,
            pointHitRadius: 10,
            data: [65, 59, 80, 81, 56, 55, 40]
          }
        ]
      },
      options: {
        maintainAspectRatio: false,
        responsive: true,
        scales: {
          yAxes: [{
            ticks: {
              beginAtZero: true
            }
          }]
        }
      }
    };
  }

  initializeChart = () => {
    var chart = ReactDOM.findDOMNode(this);
    this.state.chart = new Chart(chart, {
      type: 'line',
      data: this.state.data,
      options: this.state.options,
    });
  };

  componentDidMount = () => {
    this.initializeChart();
  };

  componentWillUnmount = () => {
    this.state.chart.destroy();
  };

  render() {
    return (
      <canvas height="200"></canvas>
    );
  }
}