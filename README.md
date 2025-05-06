          
# Arduino IoT Data Logger Project

This project implements an IoT data logging system using Arduino Nano 33 IoT boards to collect sensor data and transmit it to a server for storage and visualization.

## Project Overview

The system consists of the following components:

1. **Arduino Nano 33 IoT** - Collects sensor data and transmits it to a server
2. **Web Server** - Receives, stores, and visualizes the sensor data
3. **Simulators** - Python scripts that simulate Arduino devices for testing

## Hardware Requirements

- Arduino Nano 33 IoT board
- Temperature sensor (connected to analog pin A0)
- Optional: Humidity sensor, light sensor, microphone

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

- **WiFiDataLogger.ino** - Main Arduino sketch for data logging
- **WIFI.cpp** - Implementation of WiFi connectivity and web server functionality
- **WIFI_Flask.cpp** - Implementation for connecting to a Flask server
- **example_of_sensor.cpp** - Example code for reading temperature and humidity sensors
- **example_of_microphone.cpp** - Example code for using the onboard PDM microphone
- **arduino_simulator.py** - Python script to simulate Arduino data transmission
- **beehive_simulator.py** - Python script to simulate beehive monitoring data
- **static/js/main.js** - JavaScript for the web interface

## How It Works

### Arduino Data Collection

1. The Arduino connects to a WiFi network using credentials specified in the code
2. It periodically reads sensor data (temperature, and optionally humidity and light)
3. The data is formatted as JSON
4. The data is sent to a server via HTTP POST requests
5. LED indicators show the status of data transmission

```
Data Flow: Sensors → Arduino → WiFi → Server
```

### Server Communication

The Arduino sends data to a server with the following format:

```json
{
  "temperature": 23.5,
  "device_id": "nano33iot_1",
  "timestamp": 1234567890
}
```

Additional fields like humidity, light, and sound can be included depending on the sensors used.

### Web Interface

The project includes a web interface that:
- Displays real-time sensor data
- Allows configuration of the Arduino device
- Visualizes data trends over time

## Setup Instructions

1. **Configure WiFi Settings**:
   - Open the Arduino sketch (WiFiDataLogger.ino or WIFI_Flask.cpp)
   - Update the WiFi credentials:
     ```cpp
     const char* ssid = "YourWiFiName";
     const char* password = "YourPassword";
     ```

2. **Configure Server Settings**:
   - Update the server address and port:
     ```cpp
     const char serverAddress[] = "192.168.1.9";
     const int serverPort = 3000;
     const String apiEndpoint = "/api/data";
     ```

3. **Upload to Arduino**:
   - Connect your Arduino Nano 33 IoT board
   - Upload the sketch using the Arduino IDE

4. **Run the Server**:
   - Set up a server to receive the data (not included in this repository)
   - Alternatively, use the simulator scripts for testing

## Using the Simulators

For testing without physical hardware, you can use the provided Python simulators:

```bash
python arduino_simulator.py
```

or

```bash
python beehive_simulator.py
```

These scripts simulate sensor data and send it to the configured server.

## Examples

### Reading Temperature and Humidity

The project includes example code for reading temperature and humidity from the HTS221 sensor on the Nano 33 IoT:

```cpp
float temperature = HTS.readTemperature();
float humidity = HTS.readHumidity();
```

### Using the Microphone

The project includes example code for using the onboard PDM microphone:

```cpp
PDM.onReceive(onPDMdata);
PDM.begin(1, 16000);
```

## Troubleshooting

- **WiFi Connection Issues**: Check your WiFi credentials and ensure the network is accessible
- **Server Connection Issues**: Verify the server address and port are correct
- **Sensor Reading Issues**: Check sensor connections and power

## License

This project is open source and available under the MIT License.

## Contributing

Contributions to this project are welcome. Please feel free to submit a Pull Request.

        Too many current requests. Your queue position is 1. Please wait for a while or switch to other models for a smoother experience.
