#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <ESP8266mDNS.h>  // mDNSライブラリのインクルード

//
#define DHTPIN 2      // DHTセンサーを接続するピン（GPIO2: D4ピン）
#define DHTTYPE DHT11 // DHTセンサーの種類（DHT11 または DHT22）
DHT dht(DHTPIN, DHTTYPE);

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;  // ホスト名を指定

//
ESP8266WebServer server(80);

//
void setup() {

  //
  Serial.begin(115200);

  //
  connectToWiFi();

  // Webサーバー設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

}

//
void loop() {
  server.handleClient();
  MDNS.update();  // mDNSサービスの更新
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
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // センサーの読み取りエラーがないか確認
  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "text/plain", "Sensor reading failed");
    return;
  }

  // JSON形式でデータを送信
  String json = "{\"temperature\":";
  json += String(temperature);
  json += ",\"humidity\":";
  json += String(humidity);
  json += "}";

  //
  server.send(200, "application/json", json);

}
