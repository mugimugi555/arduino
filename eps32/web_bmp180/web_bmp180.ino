#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid     = "**********";
const char* password = "**********";

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
  message += bmp.readPressure() / 100.0f;
  message += ",\"unit\":\"hPa\"}";

  message += ",";
      
  message += "\"altitude\":{\"value\":";
  message += bmp.readAltitude();
  message += ",\"unit\":\"meters\"}";

  message += ",";
    
  message += "\"sealevel\":{\"value\":";
  message += bmp.readSealevelPressure() / 100.0f;
  message += ",\"unit\":\"hPa\"}";

  message += "}";

  server.send(200, "text/plain", message );
  digitalWrite(led, 0);
    
}

void setup(void) {

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  // MAC address
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  Serial.println("");
  Serial.print("MAC Address => ");
  Serial.println(baseMacChr);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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

  // http://esp32.local/
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

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
