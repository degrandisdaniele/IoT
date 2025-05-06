#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "iPhone di Daniele";     // Replace with your WiFi network name
const char* password = "123456789";  // Replace with your WiFi password
const char* username = "";             // For enterprise networks only
bool isEnterpriseNetwork = false;      // Set to true for enterprise networks

// Server details
const char serverAddress[] = "192.168.1.9"; // Replace with your server address
const int serverPort = 3000;                           // Replace with your server port
const String apiEndpoint = "/api/data";                // Replace with your API endpoint

// Sensor pins
const int temperatureSensorPin = A0;
// Rimuovi gli altri pin dei sensori se non usati
// const int humiditySensorPin = A1;
// const int lightSensorPin = A2;

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

    // Collect sensor data (solo temperatura)
    float temperature = readTemperature();
    // Rimuovi la lettura degli altri sensori
    // float humidity = readHumidity();
    // int lightLevel = readLightLevel();

    // Send data to server (solo temperatura)
    sendDataToServer(temperature);
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

// Rimuovi le funzioni readHumidity() e readLightLevel() se non usate
/*
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
*/

// Modifica la funzione per accettare solo la temperatura
void sendDataToServer(float temperature) {
  Serial.println("Sending data to server...");

  // Create JSON document
  StaticJsonDocument<100> jsonDoc; // Riduci la dimensione se invii meno dati

  // Add sensor data (solo temperatura)
  jsonDoc["temperature"] = temperature;
  // Rimuovi gli altri dati
  // jsonDoc["humidity"] = humidity;
  // jsonDoc["light"] = lightLevel;
  jsonDoc["device_id"] = "nano33iot_1"; // Unique identifier for your device
  jsonDoc["timestamp"] = millis(); // You might want to use a real timestamp

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Stampa il JSON sulla console seriale
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