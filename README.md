


          
# IoT Sensor Module WIFI Project

## Overview
This project implements an IoT data logging system using Arduino Nano 33 IoT boards to collect sensor data and transmit it to a server for storage and visualization.

## Project Overview
The system consists of two main components:
1. A sensor module that collects environmental data (temperature, humidity, and sound levels)
2. A WiFi module that transmits the collected data to a server
3. Simulators - Python scripts that simulate Arduino devices for testing

## Hardware Requirements
- Arduino Nano 33 IoT board
- HTS221 temperature and humidity sensor
- PDM microphone
- I2C connection between modules
- Optional: Additional sensors (light sensor)

## Software Requirements
- Arduino IDE with the following libraries:
  - WiFiNINA
  - ArduinoHttpClient
  - ArduinoJson
  - PDM (for microphone functionality)
  - Arduino_HTS221 (for humidity/temperature sensor)
- Python 3.x (for running simulators)
- Web browser

## Project Structure
- **Module_sense.cpp** - Responsible for collecting sensor data
- **WIFI_Flask.cpp** - Implementation for connecting to a Flask server
- **WiFiDataLogger.ino** - Main Arduino sketch for data logging
- **app.py** - Flask server implementation
- **arduino_simulator.py** - Python script to simulate Arduino data transmission
- **beehive_simulator.py** - Python script to simulate beehive monitoring data
- **static/js/main.js** - JavaScript for the web interface

## Software Components

### Module_sense.cpp
This module is responsible for collecting sensor data:
- Temperature and humidity using the HTS221 sensor
- Sound levels using the PDM microphone
- Provides visual feedback via RGB LED based on sound levels
- Acts as an I2C slave (address 8) to provide sensor data when requested

### WIFI_Flask.cpp
This module handles data transmission:
- Connects to a WiFi network
- Reads sensor data from the sensor module via I2C
- Formats data into JSON
- Sends data to a server via HTTP POST requests
- Provides status feedback via LED

## How It Works
### Arduino Data Collection
1. The Arduino connects to a WiFi network using credentials specified in the code
2. It periodically reads sensor data (temperature, humidity, and sound)
3. The data is formatted as JSON
4. The data is sent to a server via HTTP POST requests
5. LED indicators show the status of data transmission

Data Flow: Sensors → Arduino → WiFi → Server

### Server Communication
The Arduino sends data to a server with the following format:
```json
{
  "temperature": 25.93,
  "humidity": 69.24,
  "sound": 35.11,
  "battery": 74.0,
  "device_id": "beehive_simulator_1",
  "timestamp": 1746541866349
}
```