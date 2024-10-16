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
