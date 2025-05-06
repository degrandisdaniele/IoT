document.addEventListener('DOMContentLoaded', function() {
    // Connessione Socket.IO
    const socket = io();
    const connectionStatusDot = document.querySelector('#connection-status .status-dot');
    const connectionStatusText = document.querySelector('#connection-status .status-text');
    const lastUpdateElement = document.getElementById('last-update');
    
    // Menu a tendina
    const menuButton = document.getElementById('menuButton');
    const dropdownMenu = document.getElementById('dropdownMenu');
    
    menuButton.addEventListener('click', function() {
        if (dropdownMenu.style.display === 'block') {
            dropdownMenu.style.display = 'none';
        } else {
            dropdownMenu.style.display = 'block';
        }
    });
    
    // Chiudi il menu quando si clicca altrove
    document.addEventListener('click', function(event) {
        if (!menuButton.contains(event.target) && !dropdownMenu.contains(event.target)) {
            dropdownMenu.style.display = 'none';
        }
    });
    
    // Inizializzazione dei grafici
    const temperatureCtx = document.getElementById('temperatureChart').getContext('2d');
    const humidityCtx = document.getElementById('humidityChart').getContext('2d');
    const soundCtx = document.getElementById('soundChart').getContext('2d');
    
    // Configurazione comune per i grafici
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
                        text: 'Orario'
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
    
    // Grafico Temperatura
    const temperatureChart = new Chart(temperatureCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Temperatura (°C)',
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
                        text: 'Temperatura (°C)'
                    }
                }
            }
        }
    });
    
    // Grafico Umidità
    const humidityChart = new Chart(humidityCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Umidità (%)',
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
                        text: 'Umidità (%)'
                    }
                }
            }
        }
    });
    
    // Grafico Volume Sonoro
    const soundChart = new Chart(soundCtx, {
        ...chartConfig,
        data: {
            datasets: [{
                label: 'Volume Sonoro (dB)',
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
                        text: 'Volume Sonoro (dB)'
                    }
                }
            }
        }
    });
    
    // Controllo livello miele
    const honeySlider = document.getElementById('honey-slider');
    const honeyLevel = document.getElementById('honey-level');
    const honeyAmount = document.getElementById('honey-amount');
    
    honeySlider.addEventListener('input', function() {
        const value = this.value;
        honeyLevel.style.height = value + '%';
        honeyAmount.textContent = (value / 100 * 5).toFixed(1); // Massimo 5 kg
    });
    
    // Simulazione dati per i grafici
    function addData(chart, value) {
        const now = new Date();
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(value);
        
        // Limita il numero di punti dati
        if (chart.data.labels.length > 30) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
        }
        
        chart.update();
    }
    
    // Connessione Socket.IO
    socket.on('connect', () => {
        console.log('Connesso al server');
        connectionStatusDot.classList.remove('disconnected');
        connectionStatusDot.classList.add('connected');
        connectionStatusText.textContent = 'Connesso';
    });
    
    socket.on('disconnect', () => {
        console.log('Disconnesso dal server');
        connectionStatusDot.classList.remove('connected');
        connectionStatusDot.classList.add('disconnected');
        connectionStatusText.textContent = 'Disconnesso';
    });
    
    socket.on('new_data', (data) => {
        console.log('Dati ricevuti:', data);
        
        const now = new Date();
        lastUpdateElement.textContent = `Ultimo aggiornamento: ${now.toLocaleTimeString()}`;
        
        // Aggiorna i grafici con i nuovi dati
        if (data.hasOwnProperty('temperature')) {
            addData(temperatureChart, data.temperature);
        }
        
        if (data.hasOwnProperty('humidity')) {
            addData(humidityChart, data.humidity);
        }
        
        if (data.hasOwnProperty('sound')) {
            addData(soundChart, data.sound);
        }
        
        // Aggiorna lo stato della batteria se disponibile
        if (data.hasOwnProperty('battery')) {
            const batteryLevel = document.getElementById('battery-level');
            const batteryPercentage = document.getElementById('battery-percentage');
            
            batteryLevel.style.width = data.battery + '%';
            batteryPercentage.textContent = data.battery;
        }
    });
    
    // Simulazione dati per test (rimuovere in produzione)
    function simulateData() {
        const temperature = 20 + Math.random() * 10;
        const humidity = 40 + Math.random() * 30;
        const sound = 30 + Math.random() * 20;
        
        addData(temperatureChart, temperature);
        addData(humidityChart, humidity);
        addData(soundChart, sound);
        
        setTimeout(simulateData, 5000);
    }
    
    // Avvia la simulazione (rimuovere in produzione)
    simulateData();
});