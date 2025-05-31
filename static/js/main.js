document.addEventListener('DOMContentLoaded', function() {
    // Socket.IO Connection
    const socket = io();
    const connectionStatusDot = document.querySelector('#connection-status .status-dot');
    const connectionStatusText = document.querySelector('#connection-status .status-text');
    const lastUpdateElement = document.getElementById('last-update');
    
    // Dropdown menu
    const menuButton = document.getElementById('menuButton');
    const dropdownMenu = document.getElementById('dropdownMenu');
    
    menuButton.addEventListener('click', function() {
        if (dropdownMenu.style.display === 'block') {
            dropdownMenu.style.display = 'none';
        } else {
            dropdownMenu.style.display = 'block';
        }
    });
    
    // Close menu when clicking elsewhere
    document.addEventListener('click', function(event) {
        if (!menuButton.contains(event.target) && !dropdownMenu.contains(event.target)) {
            dropdownMenu.style.display = 'none';
        }
    });
    
    // Charts initialization
    const temperatureCtx = document.getElementById('temperatureChart').getContext('2d');
    const humidityCtx = document.getElementById('humidityChart').getContext('2d');
    const soundCtx = document.getElementById('soundChart').getContext('2d');
    
    // Common chart configuration
    const chartConfig = {
        type: 'line',
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: {
                duration: 200
            },
            scales: {
                x: {
                    type: 'time',
                    time: {
                        unit: 'minute',
                        displayFormats: {
                            minute: 'HH:mm'
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
                        display: true
                    }
                }
            }
        }
    };
    
    // Temperature Chart
    const temperatureChart = new Chart(temperatureCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Temperature (°C)',
                data: [],
                borderColor: 'rgb(255, 99, 132)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                tension: 0.1,
                fill: true
            }]
        },
        options: {
            ...chartConfig.options,
            scales: {
                ...chartConfig.options.scales,
                y: {
                    ...chartConfig.options.scales.y,
                    title: {
                        ...chartConfig.options.scales.y.title,
                        text: 'Temperature (°C)'
                    }
                }
            }
        }
    });
    
    // Humidity Chart
    const humidityChart = new Chart(humidityCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Humidity (%)',
                data: [],
                borderColor: 'rgb(54, 162, 235)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                tension: 0.1,
                fill: true
            }]
        },
        options: {
            ...chartConfig.options,
            scales: {
                ...chartConfig.options.scales,
                y: {
                    ...chartConfig.options.scales.y,
                    title: {
                        ...chartConfig.options.scales.y.title,
                        text: 'Humidity (%)'
                    }
                }
            }
        }
    });
    
    // Sound Volume Chart
    const soundChart = new Chart(soundCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Sound Volume (dB)',
                data: [],
                borderColor: 'rgb(75, 192, 192)',
                backgroundColor: 'rgba(75, 192, 192, 0.2)',
                tension: 0.1,
                fill: true
            }]
        },
        options: {
            ...chartConfig.options,
            scales: {
                ...chartConfig.options.scales,
                y: {
                    ...chartConfig.options.scales.y,
                    title: {
                        ...chartConfig.options.scales.y.title,
                        text: 'Sound Volume (dB)'
                    }
                }
            }
        }
    });
    
    // Honey level control
    const honeySlider = document.getElementById('honey-slider');
    const honeyLevel = document.getElementById('honey-level');
    const honeyAmount = document.getElementById('honey-amount');
    
    honeySlider.addEventListener('input', function() {
        const value = this.value;
        honeyLevel.style.height = value + '%';
        honeyAmount.textContent = (value / 100 * 5).toFixed(1); // Maximum 5 kg
    });
    
    // Chart data simulation
    function addData(chart, value) {
        const now = new Date();
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(value);
        
        // Limit the number of data points
        if (chart.data.labels.length > 30) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
        }
        
        chart.update();
    }
    
    // Socket.IO Connection
    socket.on('connect', () => {
        console.log('Connected to server');
        connectionStatusDot.classList.remove('disconnected');
        connectionStatusDot.classList.add('connected');
        connectionStatusText.textContent = 'Connected';
    });
    
    socket.on('disconnect', () => {
        console.log('Disconnected from server');
        connectionStatusDot.classList.remove('connected');
        connectionStatusDot.classList.add('disconnected');
        connectionStatusText.textContent = 'Disconnected';
    });
    
    socket.on('new_data', (data) => {
        console.log('Data received:', data);
        
        const now = new Date();
        lastUpdateElement.textContent = `Last update: ${now.toLocaleTimeString()}`;
        
        // Update charts with new data
        if (data.hasOwnProperty('temperature')) {
            addData(temperatureChart, data.temperature);
        }
        
        if (data.hasOwnProperty('humidity')) {
            addData(humidityChart, data.humidity);
        }
        
        if (data.hasOwnProperty('sound')) {
            addData(soundChart, data.sound);
        }
        
        // Update battery status if available
        if (data.hasOwnProperty('battery')) {
            const batteryLevel = document.getElementById('battery-level');
            const batteryPercentage = document.getElementById('battery-percentage');
            
            batteryLevel.style.width = data.battery + '%';
            batteryPercentage.textContent = data.battery;
        }
    });
    
});