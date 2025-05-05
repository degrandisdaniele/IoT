#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YourWiFiName";     // Replace with your WiFi name
const char* password = "YourPassword";  // Replace with your WiFi password
const char* username = "";             // For enterprise networks only
bool isEnterpriseNetwork = false;      // Set to true for enterprise networks

// Server details
const char serverAddress[] = "your-server-address.com"; // Replace with your server address
const int serverPort = 3000;                           // Replace with your server port
const String apiEndpoint = "/api/data";                // Replace with your API endpoint

// Sensor pins
const int temperatureSensorPin = A0;
const int humiditySensorPin = A1;
const int lightSensorPin = A2;

// Data collection interval (milliseconds)
const unsigned long dataInterval = 10000; // 10 seconds
unsigned long lastDataTime = 0;

// Status LED
const int statusLed = LED_BUILTIN;

// WiFi client
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, serverPort);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  // Set LED pin as output
  pinMode(statusLed, OUTPUT);
  
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
    
    // Collect sensor data
    float temperature = readTemperature();
    float humidity = readHumidity();
    int lightLevel = readLightLevel();
    
    // Send data to server
    sendDataToServer(temperature, humidity, lightLevel);
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  
  // Check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // Indicate error with rapid blinking
    while (true) {
      digitalWrite(statusLed, HIGH);
      delay(100);
      digitalWrite(statusLed, LOW);
      delay(100);
    }
  }
  
  // Check firmware version
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the WiFi firmware");
  }
  
  // Connect to WiFi
  if (isEnterpriseNetwork && username != NULL && strlen(username) > 0) {
    // For enterprise networks
    WiFi.beginEnterprise(ssid, username, password);
  } else {
    // For regular networks
    WiFi.begin(ssid, password);
  }
  
  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(statusLed, !digitalRead(statusLed)); // Toggle LED
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(statusLed, HIGH); // LED on when connected
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi. Please check credentials.");
    digitalWrite(statusLed, LOW); // LED off when not connected
  }
}

float readTemperature() {
  // Read temperature sensor (example using analog sensor)
  int sensorValue = analogRead(temperatureSensorPin);
  
  // Convert to temperature (adjust formula based on your sensor)
  // This is an example for a TMP36 sensor
  float voltage = sensorValue * (3.3 / 1023.0);
  float temperature = (voltage - 0.5) * 100;
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  return temperature;
}

float readHumidity() {
  // Read humidity sensor (example using analog sensor)
  int sensorValue = analogRead(humiditySensorPin);
  
  // Convert to humidity (adjust formula based on your sensor)
  // This is a simplified example
  float humidity = map(sensorValue, 0, 1023, 0, 100);
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  return humidity;
}

int readLightLevel() {
  // Read light sensor
  int lightLevel = analogRead(lightSensorPin);
  
  Serial.print("Light level: ");
  Serial.println(lightLevel);
  
  return lightLevel;
}

void sendDataToServer(float temperature, float humidity, int lightLevel) {
  Serial.println("Sending data to server...");
  
  // Create JSON document
  StaticJsonDocument<200> jsonDoc;
  
  // Add sensor data
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["light"] = lightLevel;
  jsonDoc["device_id"] = "nano33iot_1"; // Unique identifier for your device
  jsonDoc["timestamp"] = millis(); // You might want to use a real timestamp
  
  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  
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