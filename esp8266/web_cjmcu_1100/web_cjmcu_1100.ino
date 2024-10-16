#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;

int sensorValue;
const int led = 13;

//
ESP8266WebServer server(80);

//
void setup(void) {
  
  //
  Serial.begin(115200);

  //
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  //
  connectToWiFi();

  //
  server.on("/", handleRoot);

  //
  server.onNotFound(handleNotFound);

  //
  server.begin();
  Serial.println("HTTP server started");
  
}

//
void loop(void) {
  server.handleClient();
  MDNS.update();
}

//
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

//
void handleRoot() {

  //
  digitalWrite(led, 1);

  //
  sensorValue = analogRead(0);
  Serial.print("formaldehyde = ");
  Serial.print(sensorValue, DEC);
  Serial.println(" PPM");

  //
  String message = '';
  message += '{';
  message += '"formaldehyde":{"product":"cjmcu-1100","value":';
  message += analogRead(0);
  message += ',"unit":"ppm"}';
  message += '}';

  //
  server.send(200, "application/json", message );
  digitalWrite(led, 0);

}

//
void handleNotFound() {

  //
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);

}
