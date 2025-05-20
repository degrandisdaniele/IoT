#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "iPhone di Daniele";     // Replace with your WiFi network name
const char* password = "123456789";  // Replace with your WiFi password
const char* username = "";             // For enterprise networks only
bool isEnterpriseNetwork = false;      // Set to true for enterprise networks

// Server details
const char serverAddress[] = "deep-dek-flask-app-f92d90f30348.herokuapp.com"; // Updated server address
const int serverPort = 443;                           // Updated to standard HTTP port
const String apiEndpoint = "/api/data";                // Updated API endpoint

// I2C settings
#define SLAVE_ADDRESS 8
#define DATA_SIZE 12  // 3 floats (4 bytes each)

// Buffer to store received I2C data
byte dataBuffer[DATA_SIZE];

// Variables to store parsed sensor values
float temperature = 0.0;
float humidity = 0.0;
float soundLevel = 0.0;
float battery = 74.0;  // Default battery level (could be read from a sensor)

// Data collection interval (milliseconds)
const unsigned long dataInterval = 1000; // 1 seconds
unsigned long lastDataTime = 0;

// Status LED
const int statusLed = LED_BUILTIN;

// WiFi client
// WiFiClient wifi; // Commenta o rimuovi questa riga
WiFiSSLClient wifi; // Usa WiFiSSLClient per HTTPS
HttpClient client = HttpClient(wifi, serverAddress, serverPort);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Set LED pin as output
  pinMode(statusLed, OUTPUT);

  // Initialize I2C as master
  Wire.begin();
  Serial.println("Connected to I2C bus as master");
  Serial.println("Reading data from slave device at address 8");

  // Connect to WiFi
  connectToWiFi();
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi();
  }

  // Check if it's time to collect and send data
  unsigned long currentTime = millis();
  if (currentTime - lastDataTime >= dataInterval) {
    lastDataTime = currentTime;

    // Read sensor data from I2C
    readSensorDataFromI2C();
    
    // Send data to server
    sendDataToServer();
  }
}

void connectToWiFi() {
    Serial.print("Connecting to WiFi network: ");
    Serial.println(ssid);
    
    // Check for the WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("Communication with WiFi module failed!");
      // Don't continue
      while (true);
    }
  
    // Check firmware version
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
      Serial.println("Please upgrade the firmware");
    }
  
    // Attempt to connect to WiFi network
    int status = WL_IDLE_STATUS;
    
    if (isEnterpriseNetwork) {
      // For enterprise networks (WPA2-Enterprise)
      Serial.println("Using enterprise WiFi authentication");
      WiFi.beginEnterprise(ssid, username, password);
      
      // Wait for connection
      Serial.print("Waiting for connection");
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
      }
    } else {
      // For regular WPA/WPA2 networks
      while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, password);
        
        // Wait 10 seconds for connection
        delay(10000);
      }
    }
    
    Serial.println("Connected to WiFi");
    // Stampa l'indirizzo IP dell'Arduino
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

void readSensorDataFromI2C() {
  // Request data from the slave device
  Wire.requestFrom(SLAVE_ADDRESS, DATA_SIZE);
  
  // Check if the correct amount of data is available
  if (Wire.available() == DATA_SIZE) {
    // Read the data into the buffer
    for (int i = 0; i < DATA_SIZE; i++) {
      dataBuffer[i] = Wire.read();
    }
    
    // Parse the received bytes back into float values
    memcpy(&temperature, dataBuffer, 4);
    memcpy(&humidity, dataBuffer + 4, 4);
    memcpy(&soundLevel, dataBuffer + 8, 4);
    
    // Print the values to Serial
    Serial.println("\n--- Sensor Readings from I2C ---");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    Serial.print("Sound Level: ");
    Serial.print(soundLevel);
    Serial.println(" dB");
    Serial.println("----------------------------------");
  } else {
    Serial.println("Error: Did not receive expected amount of data from I2C");
    // Use default values if I2C read fails
    temperature = 25.0;
    humidity = 50.0;
    soundLevel = 30.0;
  }
}

// Modified function to send data to the server
void sendDataToServer() {
  Serial.println("Sending data to server...");

  // Create JSON document
  StaticJsonDocument<200> jsonDoc;

  // Add sensor data
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["sound"] = soundLevel;
  jsonDoc["battery"] = battery;
  jsonDoc["device_id"] = "beehive_simulator_1";
  jsonDoc["timestamp"] = millis();

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Print the JSON on the serial console
  Serial.print("JSON to send: ");
  Serial.println(jsonString);

  // Send HTTP POST request
  client.beginRequest();
  client.post(apiEndpoint);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", jsonString.length());
  client.beginBody();
  client.print(jsonString);
  client.endRequest();

  // Get response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  // Indicate success or failure
  if (statusCode >= 200 && statusCode < 300) {
    // Success - blink LED quickly 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(statusLed, HIGH);
      delay(100);
      digitalWrite(statusLed, LOW);
      delay(100);
    }
    digitalWrite(statusLed, HIGH); // Back to solid on
  } else {
    // Error - blink LED slowly 2 times
    for (int i = 0; i < 2; i++) {
      digitalWrite(statusLed, HIGH);
      delay(500);
      digitalWrite(statusLed, LOW);
      delay(500);
    }
    digitalWrite(statusLed, HIGH); // Back to solid on
  }
}