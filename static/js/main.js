document.addEventListener('DOMContentLoaded', function() {
    // Elements
    const dataContainer = document.getElementById('data-container');
    const connectionStatus = document.getElementById('connection-status');
    const statusDot = connectionStatus.querySelector('.status-dot');
    const statusText = connectionStatus.querySelector('.status-text');
    const lastUpdate = document.getElementById('last-update');
    
    // Connect to Socket.IO server
    const socket = io();
    
    // Handle connection
    socket.on('connect', function() {
        console.log('Connected to server');
        statusDot.classList.remove('disconnected');
        statusDot.classList.add('connected');
        statusText.textContent = 'Connected';
    });
    
    // Handle disconnection
    socket.on('disconnect', function() {
        console.log('Disconnected from server');
        statusDot.classList.remove('connected');
        statusDot.classList.add('disconnected');
        statusText.textContent = 'Disconnected';
    });
    
    // Handle new data
    socket.on('new_data', function(data) {
        console.log('Received new data:', data);
        updateDataDisplay(data);
        updateLastUpdateTime();
    });
    
    // Function to update the data display
    function updateDataDisplay(data) {
        // Clear the container
        dataContainer.innerHTML = '';
        
        // If no data or empty object
        if (!data || Object.keys(data).length === 0) {
            dataContainer.innerHTML = '<div class="no-data-message">Waiting for data...</div>';
            return;
        }
        
        // Create a card for each data point
        for (const [key, value] of Object.entries(data)) {
            const card = document.createElement('div');
            card.className = 'data-card';
            
            const title = document.createElement('h3');
            title.textContent = formatKey(key);
            
            const dataValue = document.createElement('div');
            dataValue.className = 'data-value';
            dataValue.textContent = formatValue(value);
            
            card.appendChild(title);
            card.appendChild(dataValue);
            dataContainer.appendChild(card);
        }
    }
    
    // Format the key for display
    function formatKey(key) {
        // Convert camelCase or snake_case to Title Case with spaces
        return key
            .replace(/_/g, ' ')
            .replace(/([A-Z])/g, ' $1')
            .replace(/^./, str => str.toUpperCase())
            .trim();
    }
    
    // Format the value for display
    function formatValue(value) {
        if (typeof value === 'number') {
            // Round to 2 decimal places if it's a float
            return Number.isInteger(value) ? value : value.toFixed(2);
        }
        return value;
    }
    
    // Update the last update time
    function updateLastUpdateTime() {
        const now = new Date();
        const timeString = now.toLocaleTimeString();
        lastUpdate.textContent = `Last update: ${timeString}`;
    }
    
    // Initial fetch of data
    fetch('/api/data')
        .then(response => response.json())
        .then(data => {
            if (Object.keys(data).length > 0) {
                updateDataDisplay(data);
                updateLastUpdateTime();
            }
        })
        .catch(error => console.error('Error fetching initial data:', error));
});