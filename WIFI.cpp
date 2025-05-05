#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

// WiFi network credentials
const char* ssid = "YourWiFiName";     // Replace with your WiFi network name
const char* password = "YourPassword";  // Replace with your WiFi password
const char* username = "YourUsername";  // Replace with your WiFi username (for enterprise networks)
bool isEnterpriseNetwork = false;       // Set to true if your network requires username+password

// Web server on port 80
WiFiServer server(80);

// Variables to store sensor data
float temperature = 0.0;
float humidity = 0.0;
int lightLevel = 0;

// Configuration settings
bool ledEnabled = true;
int sampleRate = 5; // in seconds
String deviceName = "Arduino Nano 33 IoT";

// Pin definitions
const int ledPin = LED_BUILTIN;
const int analogSensorPin = A0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Set LED pin as output
  pinMode(ledPin, OUTPUT);

  // Connect to WiFi network
  connectToWiFi();

  // Start the web server
  server.begin();
  Serial.println("Web server started");
  
  // Print the IP address
  printWiFiStatus();
}

void loop() {
  // Read sensor data
  readSensorData();
  
  // Update LED based on settings
  if (ledEnabled) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  // Check for client connections
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    
    // While the client is connected
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        // If we've reached the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so we can send a response
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            // Check if this is an API request
            if (currentLine.indexOf("GET /api/data") >= 0) {
              // Send JSON data
              sendJsonData(client);
            } else if (currentLine.indexOf("POST /api/config") >= 0) {
              // Handle configuration updates
              // This is a simplified version - in a real app, you'd parse the POST data
              ledEnabled = !ledEnabled;
              client.println("{\"status\":\"success\",\"message\":\"Configuration updated\"}");
            } else {
              // Send the HTML page
              sendHtmlPage(client);
            }
            
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          // Add character to currentLine
          currentLine += c;
        }
      }
    }
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
  
  // Wait according to sample rate
  delay(sampleRate * 1000);
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

void printWiFiStatus() {
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
  
  // Print where to go in the browser
  Serial.print("To access the server, enter \"http://");
  Serial.print(ip);
  Serial.println("/\" in your web browser");
}

void readSensorData() {
  // Read analog sensor (could be light, temperature, etc.)
  lightLevel = analogRead(analogSensorPin);
  
  // Simulate temperature and humidity readings
  // In a real application, you would use actual sensors
  temperature = 20.0 + (random(100) / 10.0);
  humidity = 40.0 + (random(200) / 10.0);
  
  // Print sensor data to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Light: ");
  Serial.println(lightLevel);
}

void sendJsonData(WiFiClient client) {
  // Create a JSON document
  StaticJsonDocument<200> doc;
  
  // Add sensor data
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["light"] = lightLevel;
  
  // Add configuration
  doc["ledEnabled"] = ledEnabled;
  doc["sampleRate"] = sampleRate;
  doc["deviceName"] = deviceName;
  
  // Serialize JSON to client
  String jsonString;
  serializeJson(doc, jsonString);
  client.println(jsonString);
}

void sendHtmlPage(WiFiClient client) {
  // Send the HTML web page
  client.println("<!DOCTYPE html>");
  client.println("<html lang='en'>");
  client.println("<head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Arduino Nano 33 IoT Dashboard</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }");
  client.println(".container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }");
  client.println("h1 { color: #0066cc; }");
  client.println(".card { background-color: #f9f9f9; border-radius: 4px; padding: 15px; margin-bottom: 15px; }");
  client.println(".data-row { display: flex; justify-content: space-between; margin-bottom: 10px; }");
  client.println(".data-label { font-weight: bold; }");
  client.println(".data-value { color: #0066cc; }");
  client.println(".settings { margin-top: 20px; border-top: 1px solid #ddd; padding-top: 20px; }");
  client.println(".btn { background-color: #0066cc; color: white; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer; }");
  client.println(".btn:hover { background-color: #0055aa; }");
  client.println(".switch { position: relative; display: inline-block; width: 60px; height: 34px; }");
  client.println(".switch input { opacity: 0; width: 0; height: 0; }");
  client.println(".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }");
  client.println(".slider:before { position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }");
  client.println("input:checked + .slider { background-color: #0066cc; }");
  client.println("input:checked + .slider:before { transform: translateX(26px); }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='container'>");
  client.println("<h1>Arduino Nano 33 IoT Dashboard</h1>");
  
  client.println("<div class='card'>");
  client.println("<h2>Sensor Data</h2>");
  client.println("<div class='data-row'><span class='data-label'>Temperature:</span> <span id='temp' class='data-value'>--</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Humidity:</span> <span id='humidity' class='data-value'>--</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Light Level:</span> <span id='light' class='data-value'>--</span></div>");
  client.println("</div>");
  
  client.println("<div class='settings card'>");
  client.println("<h2>Settings</h2>");
  client.println("<div class='data-row'>");
  client.println("<span class='data-label'>LED Control:</span>");
  client.println("<label class='switch'>");
  client.println("<input type='checkbox' id='ledToggle'>");
  client.println("<span class='slider'></span>");
  client.println("</label>");
  client.println("</div>");
  
  client.println("<div class='data-row'>");
  client.println("<span class='data-label'>Sample Rate (seconds):</span>");
  client.println("<input type='number' id='sampleRate' min='1' max='60' value='5'>");
  client.println("</div>");
  
  client.println("<div class='data-row'>");
  client.println("<span class='data-label'>Device Name:</span>");
  client.println("<input type='text' id='deviceName'>");
  client.println("</div>");
  
  client.println("<button class='btn' id='saveSettings'>Save Settings</button>");
  client.println("</div>");
  
  client.println("</div>"); // End container
  
  client.println("<script>");
  client.println("// Function to fetch data from the Arduino");
  client.println("function fetchData() {");
  client.println("  fetch('/api/data')");
  client.println("    .then(response => response.json())");
  client.println("    .then(data => {");
  client.println("      document.getElementById('temp').textContent = data.temperature + '°C';");
  client.println("      document.getElementById('humidity').textContent = data.humidity + '%';");
  client.println("      document.getElementById('light').textContent = data.light;");
  client.println("      document.getElementById('ledToggle').checked = data.ledEnabled;");
  client.println("      document.getElementById('sampleRate').value = data.sampleRate;");
  client.println("      document.getElementById('deviceName').value = data.deviceName;");
  client.println("    })");
  client.println("    .catch(error => console.error('Error fetching data:', error));");
  client.println("}");
  
  client.println("// Function to save settings");
  client.println("document.getElementById('saveSettings').addEventListener('click', function() {");
  client.println("  const settings = {");
  client.println("    ledEnabled: document.getElementById('ledToggle').checked,");
  client.println("    sampleRate: parseInt(document.getElementById('sampleRate').value),");
  client.println("    deviceName: document.getElementById('deviceName').value");
  client.println("  };");
  
  client.println("  fetch('/api/config', {");
  client.println("    method: 'POST',");
  client.println("    headers: {");
  client.println("      'Content-Type': 'application/json'");
  client.println("    },");
  client.println("    body: JSON.stringify(settings)");
  client.println("  })");
  client.println("  .then(response => response.json())");
  client.println("  .then(data => {");
  client.println("    alert('Settings saved successfully!');");
  client.println("  })");
  client.println("  .catch(error => {");
  client.println("    console.error('Error saving settings:', error);");
  client.println("    alert('Error saving settings');");
  client.println("  });");
  client.println("});");
  
  client.println("// Fetch data immediately and then every 5 seconds");
  client.println("fetchData();");
  client.println("setInterval(fetchData, 5000);");
  client.println("</script>");
  
  client.println("</body>");
  client.println("</html>");
}