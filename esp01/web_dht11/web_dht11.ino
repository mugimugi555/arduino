#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード
#include <DHT.h>

#define DHTPIN 2      // DHTセンサーを接続するピン（GPIO2: ESP-01で使用可能なピン）
#define DHTTYPE DHT11 // DHTセンサーの種類（DHT11 または DHT22）
DHT dht(DHTPIN, DHTTYPE);

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

ESP8266WebServer server(80);

void setup() {

  Serial.begin(115200);

  // DHTセンサーの初期化
  dht.begin();

  // WiFi接続の開始
  connectToWiFi();

  // Webサーバー設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // mDNS responderの初期化
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  }

}

void loop() {

  server.handleClient();
  MDNS.update();  // mDNSサービスの更新

}

// WiFiに接続する関数
void connectToWiFi() {

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // ESP01のIPアドレスなどをシリアルモニタに表示
  Serial.print("Hostname: http://");
  Serial.print(hostname);
  Serial.println(".local");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

}

// ルートパスにアクセスした際の処理
void handleRoot() {

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

  server.send(200, "application/json", json);

}
