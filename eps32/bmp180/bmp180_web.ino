#include <Wire.h>
#include <Adafruit_BMP085.h>
#define seaLevelPressure_hPa 1013.25
Adafruit_BMP085 bmp;

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "************";
const char* password = "*********";

WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);

  String message = "";
  
  message += "{";
  
  message += "\"temperature\":{\"value\":";
  message += bmp.readTemperature();
  message += ",\"unit\":\"*C\"}";

  message += ",";
    
  message += "\"pressure\":{\"value\":";
  message += bmp.readPressure() / 100.0F;
  message += ",\"unit\":\"hPa\"}";

  message += ",";
  
  message += "\"altitude\":{\"value\":";
  message += bmp.readAltitude();
  message += ",\"unit\":\"meters\"}";

  message += ",";  

  message += "\"sealevel\":{\"value\":";
  message += bmp.readSealevelPressure() / 100.0F;
  message += ",\"unit\":\"hPa\"}";

  message += ",";
    
  message += "\"altitude\":{\"value\":";
  message += bmp.readAltitude(seaLevelPressure_hPa * 100);
  message += ",\"unit\":\"meter\"}";

  message += "}";

  server.send(200, "text/plain", message );
  digitalWrite(led, 0);
    
}

void handleNotFound() {
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

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
  
}

void loop(void) {
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
