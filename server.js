const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const http = require('http');
const socketIo = require('socket.io');

// Initialize Express app
const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));

// Initialize database
const db = new sqlite3.Database(path.join(__dirname, 'sensor_data.db'));

// Create tables if they don't exist
db.serialize(() => {
  db.run(`
    CREATE TABLE IF NOT EXISTS sensor_data (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      device_id TEXT,
      temperature REAL,
      humidity REAL,
      light INTEGER,
      timestamp INTEGER,
      created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `);
});

// API endpoint to receive data from Arduino
app.post('/api/data', (req, res) => {
  const { temperature, humidity, light, device_id, timestamp } = req.body;
  
  // Validate data
  if (temperature === undefined || humidity === undefined || light === undefined) {
    return res.status(400).json({ error: 'Missing required sensor data' });
  }
  
  // Insert data into database
  const stmt = db.prepare(`
    INSERT INTO sensor_data (device_id, temperature, humidity, light, timestamp)
    VALUES (?, ?, ?, ?, ?)
  `);
  
  stmt.run(
    device_id || 'unknown',
    temperature,
    humidity,
    light,
    timestamp || Date.now(),
    function(err) {
      if (err) {
        console.error('Database error:', err);
        return res.status(500).json({ error: 'Failed to save data' });
      }
      
      // Emit the new data to all connected clients
      io.emit('new-data', {
        id: this.lastID,
        device_id: device_id || 'unknown',
        temperature,
        humidity,
        light,
        timestamp: timestamp || Date.now(),
        created_at: new Date().toISOString()
      });
      
      res.status(201).json({ 
        success: true, 
        message: 'Data saved successfully',
        id: this.lastID
      });
    }
  );
  
  stmt.finalize();
});

// API endpoint to get historical data
app.get('/api/data', (req, res) => {
  const limit = parseInt(req.query.limit) || 100;
  const device = req.query.device_id;
  
  let query = 'SELECT * FROM sensor_data';
  let params = [];
  
  if (device) {
    query += ' WHERE device_id = ?';
    params.push(device);
  }
  
  query += ' ORDER BY timestamp DESC LIMIT ?';
  params.push(limit);
  
  db.all(query, params, (err, rows) => {
    if (err) {
      console.error('Database error:', err);
      return res.status(500).json({ error: 'Failed to retrieve data' });
    }
    
    res.json(rows);
  });
});

// Socket.IO connection handling
io.on('connection', (socket) => {
  console.log('New client connected');
  
  socket.on('disconnect', () => {
    console.log('Client disconnected');
  });
});

// Start server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});