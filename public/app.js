// Connect to Socket.IO server
const socket = io();

// DOM elements
const connectionStatus = document.getElementById('connection-status');
const temperatureValue = document.getElementById('temperature-value');
const humidityValue = document.getElementById('humidity-value');
const lightValue = document.getElementById('light-value');
const dataTypeSelect = document.getElementById('data-type');
const timeRangeSelect = document.getElementById('time-range');
const dataTableBody = document.getElementById('data-table-body');

// Initialize gauges
const temperatureGauge = new Chart(document.getElementById('temperature-gauge'), {
  type: 'doughnut',
  data: {
    datasets: [{
      data: [0, 100],
      backgroundColor: ['#3498db', '#ecf0f1'],
      borderWidth: 0
    }]
  },
  options: {
    circumference: 180,
    rotation: 270,
    cutout: '70%',
    plugins: {
      tooltip: { enabled: false },
      legend: { display: false }
    },
    animation: { duration: 500 }
  }
});

const humidityGauge = new Chart(document.getElementById('humidity-gauge'), {
  type: 'doughnut',
  data: {
    datasets: [{
      data: [0, 100],
      backgroundColor: ['#2ecc71', '#ecf0f1'],
      borderWidth: 0
    }]
  },
  options: {
    circumference: 180,
    rotation: 270,
    cutout: '70%',
    plugins: {
      tooltip: { enabled: false },
      legend: { display: false }
    },
    animation: { duration: 500 }
  }
});

const lightGauge = new Chart(document.getElementById('light-gauge'), {
  type: 'doughnut',
  data: {
    datasets: [{
      data: [0, 1023],
      backgroundColor: ['#f39c12', '#ecf0f1'],
      borderWidth: 0
    }]
  },
  options: {
    circumference: 180,
    rotation: 270,
    cutout: '70%',
    plugins: {
      tooltip: { enabled: false },
      legend: { display: false }
    },
    animation: { duration: 500 }
  }
});

// Initialize history chart
const historyChart = new Chart(document.getElementById('history-chart'), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Temperature',
      data: [],
      borderColor: '#3498db',
      backgroundColor: 'rgba(52, 152, 219, 0.1)',
      borderWidth: 2,
      fill: true,
      tension: 0.4
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        type: 'time',
        time: {
          unit: 'hour',
          displayFormats: {
            hour: 'HH:mm'
          }
        },
        title: {
          display: true,
          text: 'Time'
        }
      },
      y: {
        beginAtZero: false,
        title: {
          display: true,
          text: 'Temperature (°C)'
        }
      }
    },
    plugins: {
      legend: {
        display: false
      }
    }
  }
});

// Socket.IO event handlers
socket.on('connect', () => {
  connectionStatus.textContent = 'Connected';
  connectionStatus.style.color = '#2ecc71';
  fetchHistoricalData();
});

socket.on('disconnect', () => {
  connectionStatus.textContent = 'Disconnected';
  connectionStatus.style.color = '#e74c3c';
});

socket.on('new-data', (data) => {
  // Update gauges
  updateGauges(data);
  
  // Add to table
  addDataToTable(data);
  
  // Update chart if needed
  updateHistoryChart(data);
});

// Functions to update UI
function updateGauges(data) {
  // Update temperature gauge
  temperatureGauge.data.datasets[0].data[0] = data.temperature;
  temperatureGauge.data.datasets[0].data[1] = 100 - data.temperature;
  temperatureGauge.update();
  temperatureValue.textContent = `${data.temperature.toFixed(1)}°C`;
  
  // Update humidity gauge
  humidityGauge.data.datasets[0].data[0] = data.humidity;
  humidityGauge.data.datasets[0].data[1] = 100 - data.humidity;
  humidityGauge.update();
  humidityValue.textContent = `${data.humidity.toFixed(1)}%`;
  
  // Update light gauge
  lightGauge.data.datasets[0].data[0] = data.light;
  lightGauge.data.datasets[0].data[1] = 1023 - data.light;
  lightGauge.update();
  lightValue.textContent = data.light;
}

function addDataToTable(data) {
  // Create new row
  const row = document.createElement('tr');
  
  // Format date
  const date = new Date(data.created_at);
  const formattedDate = date.toLocaleTimeString();
  
  // Add cells
  row.innerHTML = `
    <td>${formattedDate}</td>
    <td>${data.temperature.toFixed(1)}°C</td>
    <td>${data.humidity.toFixed(1)}%</td>
    <td>${data.light}</td>
  `;
  
  // Add to table (at the top)
  dataTableBody.insertBefore(row, dataTableBody.firstChild);
  
  // Limit table rows
  if (dataTableBody.children.length > 10) {
    dataTableBody.removeChild(dataTableBody.lastChild);
  }
}

function updateHistoryChart(newData) {
  const currentDataType = dataTypeSelect.value;
  const currentTimeRange = timeRangeSelect.value;
  
  // Only update chart if it's showing the current data type
  if (historyChart.data.datasets[0].label.toLowerCase().includes(currentDataType)) {
    // Add new data point
    historyChart.data.labels.push(new Date(newData.created_at));
    historyChart.data.datasets[0].data.push(newData[currentDataType]);
    
    // Limit data points based on time range
    const cutoffTime = getCutoffTime(currentTimeRange);
    
    // Filter out old data
    const newLabels = [];
    const newData = [];
    
    for (let i = 0; i < historyChart.data.labels.length; i++) {
      if (historyChart.data.labels[i] >= cutoffTime) {
        newLabels.push(historyChart.data.labels[i]);
        newData.push(historyChart.data.datasets[0].data[i]);
      }
    }
    
    historyChart.data.labels = newLabels;
    historyChart.data.datasets[0].data = newData;
    
    historyChart.update();
  }
}

// Fetch historical data from API
function fetchHistoricalData() {
  const dataType = dataTypeSelect.value;
  const timeRange = timeRangeSelect.value;
  
  fetch(`/api/data?limit=1000`)
    .then(response => response.json())
    .then(data => {
      // Filter data based on time range
      const cutoffTime = getCutoffTime(timeRange);
      const filteredData = data.filter(item => new Date(item.created_at) >= cutoffTime);
      
      // Update chart
      updateChartWithHistoricalData(filteredData, dataType);
      
      // Update table with most recent entries
      updateTableWithHistoricalData(data.slice(0, 10));
      
      // Update gauges with most recent data
      if (data.length > 0) {
        updateGauges(data[0]);
      }
    })
    .catch(error => {
      console.error('Error fetching historical data:', error);
    });
}

function updateChartWithHistoricalData(data, dataType) {
  // Clear existing data
  historyChart.data.labels = [];
  historyChart.data.datasets[0].data = [];
  
  // Set chart title based on data type
  let title = 'Temperature (°C)';
  let color = '#3498db';
  
  if (dataType === 'humidity') {
    title = 'Humidity (%)';
    color = '#2ecc71';
  } else if (dataType === 'light') {
    title = 'Light Level';
    color = '#f39c12';
  }
  
  historyChart.options.scales.y.title.text = title;
  historyChart.data.datasets[0].label = title;
  historyChart.data.datasets[0].borderColor = color;
  historyChart.data.datasets[0].backgroundColor = `${color}1A`; // 10% opacity
  
  // Add data points (in chronological order)
  data.reverse().forEach(item => {
    historyChart.data.labels.push(new Date(item.created_at));
    historyChart.data.datasets[0].data.push(item[dataType]);
  });
  
  // Update chart
  historyChart.update();
}

function updateTableWithHistoricalData(data) {
  // Clear existing rows
  dataTableBody.innerHTML = '';
  
  // Add new rows
  data.forEach(item => {
    const row = document.createElement('tr');
    
    // Format date
    const date = new Date(item.created_at);
    const formattedDate = date.toLocaleTimeString();
    
    // Add cells
    row.innerHTML = `
      <td>${formattedDate}</td>
      <td>${item.temperature.toFixed(1)}°C</td>
      <td>${item.humidity.toFixed(1)}%</td>
      <td>${item.light}</td>
    `;
    
    // Add to table
    dataTableBody.appendChild(row);
  });
}

function getCutoffTime(timeRange) {
  const now = new Date();
  
  switch (timeRange) {
    case '1h':
      return new Date(now - 60 * 60 * 1000);
    case '6h':
      return new Date(now - 6 * 60 * 60 * 1000);
    case '24h':
      return new Date(now - 24 * 60 * 60 * 1000);
    case '7d':
      return new Date(now - 7 * 24 * 60 * 60 * 1000);
    default:
      return new Date(now - 24 * 60 * 60 * 1000);
  }
}

// Event listeners
dataTypeSelect.addEventListener('change', () => {
  fetchHistoricalData();
});

timeRangeSelect.addEventListener('change', () => {
  fetchHistoricalData();
});