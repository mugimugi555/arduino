#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード

// 湿度センサーのピン設定
const int moisturePin = A0;  // アナログピンに接続

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名

ESP8266WebServer server(80); // Webサーバーのポート設定

void setup() {

  Serial.begin(115200); // シリアル通信を開始

  connectToWiFi(); // WiFiに接続

  server.on("/", handleRoot); // ルートURLのハンドラ設定
  server.begin(); // サーバーを開始
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient(); // クライアントからのリクエストを処理
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

  // Display the esp8266's hostname
  Serial.print("Hostname: http://");
  Serial.print(WiFi.getHostname());
  Serial.println(".local");

  // Display the esp8266's IP address
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

  // Display the esp8266's MAC address
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Start mDNS responder
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

}

// ルートURLへのリクエストを処理する関数
void handleRoot() {

  // 湿度センサーの値を取得
  int moistureValue = analogRead(moisturePin);
  
  // パーセントで計算
  float moisturePercent = map(moistureValue, 0, 1024, 0, 100);
  
  // JSON形式でデータを作成
  String json = "{\"moisture\":";
  json += String(moisturePercent);
  json += "}";

  // クライアントにJSONデータを送信
  server.send(200, "application/json", json);
  
  // シリアル出力
  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

}
