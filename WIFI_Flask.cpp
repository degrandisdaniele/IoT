#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "Redmi Note 9";     // Replace with your WiFi network name
const char* password = "12345678a";  // Replace with your WiFi password
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

// RGB LED pins
const int redPin = 4;
const int greenPin = 2; // Corrected green pin to 4 as per your request
const int bluePin = 3;

// Variables to store parsed sensor values
const int batteryPin = A0;            // Analog input pin
const float arduinoRefVoltage = 3.33;  // Nano 33 IoT ADC reference voltage
const float resistorFactor = (332.4 + 508.4) / 332.4;  // ~2.54 for 910kΩ and 590kΩ resistors
// Variables to store parsed sensor values
float temperature = 0.0;
float humidity = 0.0;
float soundLevel = 0.0;
float battery = 74.0;  // Default battery level (could be read from a sensor)

// Data collection interval (milliseconds)
const unsigned long dataInterval = 1000; // 1 seconds
unsigned long lastDataTime = 0;

// Status LED
// const int statusLed = LED_BUILTIN; // Rimosso
const int internetStatusLedPin = 5; // LED for internet connection status AND server status

// WiFi client
// WiFiClient wifi; // Commenta o rimuovi questa riga
WiFiSSLClient wifi; // Usa WiFiSSLClient per HTTPS
HttpClient client = HttpClient(wifi, serverAddress, serverPort);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  unsigned long startMillis = millis();
  while (!Serial && (millis() - startMillis < 3000));

  // Set LED pin as output
  // pinMode(statusLed, OUTPUT); // Rimosso
  pinMode(internetStatusLedPin, OUTPUT); // Set internet status LED pin as output
  digitalWrite(internetStatusLedPin, LOW); // Start with LED OFF

  // Set RGB LED pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initialize I2C as master
  Wire.begin();
  Serial.println("Connected to I2C bus as master");
  Serial.println("Reading data from slave device at address 8");

  int currentBatteryLevel = readBatteryLevel();
  updateLedStatus(currentBatteryLevel);
  // Connect to WiFi
  connectToWiFi();
}

void loop() {
  // Real-time check of WiFi connection status for LED
  if (WiFi.status() == WL_CONNECTED) {
    // If we are not in the middle of server communication blinking, keep it HIGH for WiFi connected
    // The server communication blinking in sendDataToServer will temporarily override this
    digitalWrite(internetStatusLedPin, HIGH); 
  } else {
    digitalWrite(internetStatusLedPin, LOW); // Internet is NOT connected, LED OFF
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi(); 
  }

  // Check if it's time to collect and send data
  unsigned long currentTime = millis();
  if (currentTime - lastDataTime >= dataInterval) {
    lastDataTime = currentTime;

    // Read sensor data from I2C
    readSensorDataFromI2C();
    
    // Read battery level and update LED
    int batteryPercentage = readBatteryLevel();
    updateLedStatus(batteryPercentage); // Update LED based on battery

    // Send data to server
    sendDataToServer(); // This function also calls readBatteryLevel()
  }
}

void connectToWiFi() {

    Serial.print("Connecting to WiFi network: ");
    Serial.println(ssid);
    // digitalWrite(internetStatusLedPin, LOW); // LED is OFF before attempting to connect or stays OFF if already so
    
    // Check for the WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("Communication with WiFi module failed!");
      digitalWrite(internetStatusLedPin, LOW); // Ensure LED is OFF
      // Don't continue
      while (true);
    }
  
    // Check firmware version
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
      Serial.println("Please upgrade the firmware");
      digitalWrite(internetStatusLedPin, LOW); // Ensure LED is OFF
    }
  
    // Attempt to connect to WiFi network
    int status = WL_IDLE_STATUS;
    unsigned long blinkPreviousMillis = 0;
    bool ledState = LOW;

    if (isEnterpriseNetwork) {
      // For enterprise networks (WPA2-Enterprise)
      Serial.println("Using enterprise WiFi authentication");
      WiFi.beginEnterprise(ssid, username, password);
      
      // Wait for connection and blink LED
      Serial.print("Waiting for connection");
      while (WiFi.status() != WL_CONNECTED) {
        unsigned long currentMillis = millis();
        if (currentMillis - blinkPreviousMillis >= 500) { // Blink every 500ms
          blinkPreviousMillis = currentMillis;
          ledState = !ledState;
          digitalWrite(internetStatusLedPin, ledState);
        }
        Serial.print(".");
        // Add a small delay to allow WiFi processes to run, but not too long to miss blinks
        delay(50); 
      }
    } else {
      // For regular WPA/WPA2 networks
      while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, password);
        
        // Wait 10 seconds for connection, blinking LED
        unsigned long connectionAttemptStartTime = millis();
        while (millis() - connectionAttemptStartTime < 10000 && status != WL_CONNECTED) {
            unsigned long currentMillis = millis();
            if (currentMillis - blinkPreviousMillis >= 500) { // Blink every 500ms
                blinkPreviousMillis = currentMillis;
                ledState = !ledState;
                digitalWrite(internetStatusLedPin, ledState);
            }
            status = WiFi.status(); // Check status inside the loop
            delay(50); // Small delay
        }
        if (status != WL_CONNECTED) {
            Serial.println("Connection attempt failed, retrying...");
        }
      }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        digitalWrite(internetStatusLedPin, HIGH); // Turn ON internet status LED solid
        // Stampa l'indirizzo IP dell'Arduino
        IPAddress ip = WiFi.localIP();
        Serial.print("IP Address: ");
        Serial.println(ip);
    } else {
        Serial.println("Failed to connect to WiFi after multiple attempts.");
        digitalWrite(internetStatusLedPin, LOW); // Ensure LED is OFF if connection ultimately fails
    }
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
// Function to update RGB LED status based on battery percentage
void updateLedStatus(int batteryPercentage) {
  if (batteryPercentage > 75) { // Green for > 75%
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  } else if (batteryPercentage > 25) { // Orange for > 25% and <= 75%
    digitalWrite(redPin, HIGH);    // Red ON
    digitalWrite(greenPin, HIGH);   // Green ON (to make Orange)
    digitalWrite(bluePin, LOW);
  } else { // Red for <= 25%
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
}

// Add this function definition with your other functions
int readBatteryLevel() {
  // Take multiple readings for better accuracy
  const int numReadings = 10;
  long sum = 0;
  
  for(int i = 0; i < numReadings; i++) {
    sum += analogRead(batteryPin);
    delay(2); // Short delay between readings
  }
  
  int rawValue = sum / numReadings;
  
  // Convert analog reading to voltage at pin A0
  float voltageAtPin = (rawValue / 1015.0) * arduinoRefVoltage;
  
  // Calculate actual battery voltage using the resistor divider ratio
  float batteryVoltage = voltageAtPin * resistorFactor;
  
  // Convert voltage to percentage
  float minVoltage = 6.5;  // Empty battery voltage (minimum for Arduino)
  float maxVoltage = 8.4;  // Full battery voltage
  float percentage = ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;
  
  // Constrain to valid range
  percentage = constrain(percentage, 0.0, 100.0);
  
  Serial.print("Raw ADC: ");
  Serial.print(rawValue);
  Serial.print(", Voltage at A0: ");
  Serial.print(voltageAtPin);
  Serial.print("V, Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.print("V, Battery Percentage: ");
  Serial.print(percentage);
  Serial.println("%");
  
  return percentage;
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
  int currentBatteryLevel = readBatteryLevel();
  jsonDoc["battery"] = currentBatteryLevel;
  updateLedStatus(currentBatteryLevel); // Update LED after reading battery for server send
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

  // Indicate success or failure using internetStatusLedPin
  if (statusCode >= 200 && statusCode < 300) {
    // Success - blink LED quickly 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(internetStatusLedPin, HIGH);
      delay(100);
      digitalWrite(internetStatusLedPin, LOW);
      delay(100);
    }
    // After blinking, if WiFi is still connected, it should go back to HIGH in the next loop() iteration
    // Or set it high here if you want immediate feedback before loop() runs again
    if(WiFi.status() == WL_CONNECTED) digitalWrite(internetStatusLedPin, HIGH); 
    else digitalWrite(internetStatusLedPin, LOW);

  } else {
    // Error - blink LED slowly 2 times
    for (int i = 0; i < 2; i++) {
      digitalWrite(internetStatusLedPin, HIGH);
      delay(500);
      digitalWrite(internetStatusLedPin, LOW);
      delay(500);
    }
    // After blinking, restore LED state based on WiFi connection
    if(WiFi.status() == WL_CONNECTED) digitalWrite(internetStatusLedPin, HIGH); 
    else digitalWrite(internetStatusLedPin, LOW);
  }
}