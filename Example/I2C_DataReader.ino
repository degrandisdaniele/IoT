#include <Wire.h>

// Constants
#define SERIAL_BAUD_RATE 9600
#define SLAVE_ADDRESS 8
#define DATA_SIZE 12  // 3 floats (4 bytes each)

// Buffer to store received data
byte dataBuffer[DATA_SIZE];

// Variables to store parsed sensor values
float temperature = 0.0;
float humidity = 0.0;
float soundLevel = 0.0;

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial);
  
  Serial.println("I2C Data Reader Starting...");
  
  // Initialize I2C as master
  Wire.begin();
  
  Serial.println("Connected to I2C bus as master");
  Serial.println("Reading data from slave device at address 8");
  Serial.println("---------------------------------------------");
}

void loop() {
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
    Serial.println("\n--- Sensor Readings ---");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    Serial.print("Sound Level: ");
    Serial.print(soundLevel);
    Serial.println(" dB");
    Serial.println("----------------------");
  } else {
    Serial.println("Error: Did not receive expected amount of data");
  }
  
  // Wait before requesting data again
  delay(1000);
}