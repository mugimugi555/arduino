#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// WiFi SSID and password
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;

// LED pin
const int led = 13;

// Web server on port 80
WebServer server(80);

void setup(void) {

  Serial.begin(115200);

  // Set up the LED
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  //
  connectToWiFi();

  // Define the root handler
  server.on("/", handleRoot);

  // Start the HTTP server
  server.begin();
  Serial.println("HTTP server started");

  // Initialize BMP085 sensor
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

}

void loop(void) {

  // Handle client requests
  server.handleClient();

}

void connectToWiFi() {

  // Connect to WiFi
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // Display the ESP32's hostname
  Serial.print("Hostname: http://");
  Serial.print(WiFi.getHostname());
  Serial.println(".local");

  // Display the ESP32's IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display subnet mask
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  // Display gateway IP
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());

  // Display DNS server IP
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  // Display the ESP32's MAC address
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Start mDNS responder
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

}

void handleRoot() {

  // Turn on the LED
  digitalWrite(led, 1);

  // Create and send the JSON response
  String message = String('{"temperature":{"value":') + bmp.readTemperature()               + ',"unit":"*C"},'     +
                   String('"pressure":{"value":')     + bmp.readPressure() / 100.0f         + ',"unit":"hPa"},'    +
                   String('"altitude":{"value":')     + bmp.readAltitude()                  + ',"unit":"meters"},' +
                   String('"sealevel":{"value":')     + bmp.readSealevelPressure() / 100.0f + ',"unit":"hPa"}}';

  // Send the response to the client
  server.send(200, "text/plain", message);

  // Turn off the LED
  digitalWrite(led, 0);

}

