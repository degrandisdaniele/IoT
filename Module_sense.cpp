
#include <Arduino_HTS221.h>
#include <PDM.h>
#include <Wire.h>


// Constants
#define SERIAL_BAUD_RATE 9600
#define TEMP_THRESHOLD 0.5
#define HUMIDITY_THRESHOLD 1.0
#define SOUND_HIGH_THRESHOLD 70  // dB threshold for high sound
#define SOUND_MEDIUM_THRESHOLD 50  // dB threshold for medium sound
#define SAMPLE_BUFFER_SIZE 256
#define PDM_CHANNELS 1
#define PDM_SAMPLE_RATE 16000

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[SAMPLE_BUFFER_SIZE];

// Number of samples read
volatile int samplesRead;

// Variables for sensor readings
float microphoneDB = 0;
float previousTemperature = 0;
float previousHumidity = 0;
unsigned long lastTempHumidityCheck = 0;
unsigned long lastI2CUpdate = 0;
#define SENSOR_CHECK_INTERVAL 1000  // Check temp/humidity every 1000ms
#define I2C_UPDATE_INTERVAL 1000    // Send I2C data every 1000ms

// Buffer for I2C data
byte i2cDataBuffer[12];  // 3 floats (4 bytes each)

// Function prototypes
void onPDMdata();
void updateLEDs(float soundDB);
void printSensorData(float temperature, float humidity);
float convertToDecibel(short soundValue);
void requestEvent();
void updateI2CData();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial);

  // Initialize LED pins
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  // Turn off all LEDs initially (they are active LOW)
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  // Initialize temperature and humidity sensor
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
  
  // Configure the PDM audio data receive callback
  PDM.onReceive(onPDMdata);
  
  // Initialize PDM for microphone
  if (!PDM.begin(PDM_CHANNELS, PDM_SAMPLE_RATE)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }
  
  // Initialize I2C as slave with address #8
  Wire.begin(8);                
  Wire.onRequest(requestEvent); // register event
  
  // Initialize I2C data buffer
  updateI2CData();
}

void loop() {
  // Process microphone data if samples are available
  if (samplesRead) {
    // Calculate RMS value for the sound samples
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += (long)sampleBuffer[i] * sampleBuffer[i];
    }
    
    // Calculate RMS and convert to decibels
    float rms = sqrt(sum / samplesRead);
    microphoneDB = convertToDecibel(rms);
    
    // Update LEDs based on sound level
    updateLEDs(microphoneDB);
    
    // Clear the read count
    samplesRead = 0;
  }
  
  // Check temperature and humidity at regular intervals without blocking
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempHumidityCheck >= SENSOR_CHECK_INTERVAL) {
    lastTempHumidityCheck = currentMillis;
    
    // Read temperature and humidity values
    float temperature = HTS.readTemperature();
    float humidity = HTS.readHumidity();

    // Check if there's a significant change in temperature or humidity
    if (abs(previousTemperature - temperature) >= TEMP_THRESHOLD || 
        abs(previousHumidity - humidity) >= HUMIDITY_THRESHOLD) {
      previousTemperature = temperature;
      previousHumidity = humidity;
      
      // Print detailed sensor values when there's a significant change
      Serial.println("Significant change detected!");
      printSensorData(temperature, humidity);
    } else {
      // Print regular sensor readings
      printSensorData(temperature, humidity);
    }
  }
  
  // Update I2C data buffer every second
  if (currentMillis - lastI2CUpdate >= I2C_UPDATE_INTERVAL) {
    lastI2CUpdate = currentMillis;
    updateI2CData();
  }
}

// PDM data callback function
void onPDMdata() {
  // Query the number of bytes available
  int bytesAvailable = PDM.available();
  
  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);
  
  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

// Function to convert raw sound value to decibels
float convertToDecibel(short soundValue) {
  // Prevent log of zero or negative
  if (soundValue <= 0) soundValue = 1;
  
  // Convert to decibels
  float dB = 20.0 * log10(soundValue / 1.0);
  
  // Ensure reasonable range (adjust as needed for your application)
  if (dB < 0) dB = 0;
  if (dB > 120) dB = 120;
  
  return dB;
}

// Function to update LEDs based on sound level in dB
void updateLEDs(float soundDB) {
  // Reset all LEDs (they are active LOW)
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  
  // Set LED based on sound level
  if (soundDB >= SOUND_HIGH_THRESHOLD) {
    // High sound level - Red LED
    digitalWrite(LEDR, LOW);
  } else if (soundDB >= SOUND_MEDIUM_THRESHOLD) {
    // Medium sound level - Blue LED
    digitalWrite(LEDB, LOW);
  } else {
    // Low sound level - Green LED
    digitalWrite(LEDG, LOW);
  }
}

// Function to print sensor data
void printSensorData(float temperature, float humidity) {
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Microphone  = ");
  Serial.print(microphoneDB);
  Serial.println(" dB");
  
  Serial.println(); // Print an empty line
}

// Function to update the I2C data buffer with current sensor values
void updateI2CData() {
  float temperature = HTS.readTemperature();
  float humidity = HTS.readHumidity();
  
  // Convert float values to bytes in the buffer
  memcpy(i2cDataBuffer, &temperature, 4);
  memcpy(i2cDataBuffer + 4, &humidity, 4);
  memcpy(i2cDataBuffer + 8, &microphoneDB, 4);
  
  Serial.println("I2C data updated");
}

// Function to handle I2C request events
void requestEvent() {
  // Send the data over I2C
  Wire.write(i2cDataBuffer, 12);
}