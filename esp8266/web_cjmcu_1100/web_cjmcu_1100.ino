#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

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
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  //
  Serial.begin(115200);

  // Wi-Fi接続
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ESP8266のIPアドレスを表示
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // mDNSサービスを開始
  if (MDNS.begin(hostname)) {
    Serial.println("MDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

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