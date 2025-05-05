#include <SPI.h>
#include <WiFiNINA.h>

// WiFi network credentials - will be set via Serial
String ssid = "";  
String password = "";
String username = "";
bool isEnterpriseNetwork = false;

// Connection status
bool isConnected = false;

// LED pin for connection status indication
const int statusLed = LED_BUILTIN;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  // Set LED pin as output
  pinMode(statusLed, OUTPUT);
  
  // Print welcome message
  Serial.println("Arduino Nano 33 IoT WiFi Status Monitor");
  Serial.println("--------------------------------------");
  
  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("ERROR: Communication with WiFi module failed!");
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
    Serial.println("WARNING: Please upgrade the WiFi firmware");
  }
  
  // Prompt for WiFi credentials
  promptForCredentials();
}

void loop() {
  // Check for serial input to change credentials
  checkSerialInput();
  
  // Check WiFi connection status
  checkWiFiStatus();
  
  // Update LED based on connection status
  updateStatusLed();
  
  // Delay before next check
  delay(1000);
}

void promptForCredentials() {
  Serial.println("\nEnter WiFi credentials:");
  
  Serial.print("SSID: ");
  while (ssid.length() == 0) {
    if (Serial.available()) {
      ssid = Serial.readStringUntil('\n');
      ssid.trim();
      Serial.println(ssid);
    }
  }
  
  Serial.print("Password: ");
  while (password.length() == 0) {
    if (Serial.available()) {
      password = Serial.readStringUntil('\n');
      password.trim();
      Serial.println("********"); // Don't show actual password
    }
  }
  
  Serial.print("Does your network require a username? (y/n): ");
  while (true) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();
      Serial.println(response);
      
      if (response.equalsIgnoreCase("y") || response.equalsIgnoreCase("yes")) {
        isEnterpriseNetwork = true;
        
        Serial.print("Username: ");
        while (username.length() == 0) {
          if (Serial.available()) {
            username = Serial.readStringUntil('\n');
            username.trim();
            Serial.println(username);
          }
        }
        break;
      } 
      else if (response.equalsIgnoreCase("n") || response.equalsIgnoreCase("no")) {
        isEnterpriseNetwork = false;
        break;
      }
      else {
        Serial.print("Please enter 'y' or 'n': ");
      }
    }
  }
  
  // Connect to WiFi with provided credentials
  connectToWiFi();
}

void connectToWiFi() {
  Serial.print("\nConnecting to WiFi network: ");
  Serial.println(ssid);
  
  // Disconnect if already connected
  WiFi.disconnect();
  delay(1000);
  
  int status = WL_IDLE_STATUS;
  
  if (isEnterpriseNetwork) {
    // For enterprise networks (WPA2-Enterprise)
    Serial.println("Using enterprise WiFi authentication");
    WiFi.beginEnterprise(ssid.c_str(), username.c_str(), password.c_str());
    
    // Wait for connection
    Serial.print("Waiting for connection");
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(1000);
      Serial.print(".");
      timeout++;
    }
    Serial.println();
  } 
  else {
    // For regular WPA/WPA2 networks
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(1000);
      Serial.print(".");
      timeout++;
    }
    Serial.println();
  }
  
  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    printWiFiStatus();
  } 
  else {
    isConnected = false;
    Serial.println("Failed to connect to WiFi. Please check credentials.");
    Serial.println("Type 'reset' to enter new credentials.");
  }
}

void checkSerialInput() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.equalsIgnoreCase("reset")) {
      Serial.println("Resetting WiFi credentials...");
      ssid = "";
      password = "";
      username = "";
      isEnterpriseNetwork = false;
      promptForCredentials();
    }
    else if (input.equalsIgnoreCase("status")) {
      printWiFiStatus();
    }
    else if (input.equalsIgnoreCase("scan")) {
      scanNetworks();
    }
    else if (input.equalsIgnoreCase("help")) {
      printHelp();
    }
  }
}

void checkWiFiStatus() {
  // Update connection status
  if (WiFi.status() == WL_CONNECTED) {
    if (!isConnected) {
      isConnected = true;
      Serial.println("WiFi connection established!");
      printWiFiStatus();
    }
  } 
  else {
    if (isConnected) {
      isConnected = false;
      Serial.println("WiFi connection lost!");
    }
  }
}

void updateStatusLed() {
  if (isConnected) {
    // Solid light when connected
    digitalWrite(statusLed, HIGH);
  } 
  else {
    // Blink when disconnected
    digitalWrite(statusLed, (millis() / 500) % 2);
  }
}

void printWiFiStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi");
    return;
  }
  
  // Print the SSID of the network
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print the IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  // Print MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
  
  // Print encryption type
  Serial.print("Encryption Type: ");
  printEncryptionType(WiFi.encryptionType());
  Serial.println();
}

void scanNetworks() {
  Serial.println("Scanning for networks...");
  
  // Scan for nearby networks
  int numNetworks = WiFi.scanNetworks();
  
  if (numNetworks == 0) {
    Serial.println("No networks found");
  } 
  else {
    Serial.print("Found ");
    Serial.print(numNetworks);
    Serial.println(" networks:");
    
    for (int i = 0; i < numNetworks; i++) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(") ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm) - ");
      printEncryptionType(WiFi.encryptionType(i));
      Serial.println();
    }
  }
  
  Serial.println();
}

void printEncryptionType(int type) {
  switch (type) {
    case ENC_TYPE_WEP:
      Serial.print("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.print("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.print("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.print("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.print("Auto");
      break;
    default:
      Serial.print("Unknown");
      break;
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void printHelp() {
  Serial.println("\nAvailable commands:");
  Serial.println("  reset  - Reset WiFi credentials");
  Serial.println("  status - Display current WiFi status");
  Serial.println("  scan   - Scan for available networks");
  Serial.println("  help   - Show this help message");
}