#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>  // mDNSライブラリのインクルード
#include <DHT.h>

#define DHTPIN 2      // DHTセンサーを接続するピン（GPIO2）
#define DHTTYPE DHT11 // DHTセンサーの種類（DHT11 または DHT22）
DHT dht(DHTPIN, DHTTYPE);

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP32のホスト名

// Webサーバーの設定
WebServer server(80);

void setup() {

  Serial.begin(115200);

  // DHTセンサーの初期化
  dht.begin();

  // WiFi接続
  connectToWiFi();

  // Webサーバー設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {

  server.handleClient();
  MDNS.update();  // mDNSサービスの更新

}

void connectToWiFi() {

  // WiFi接続処理
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // WiFi接続が完了するまで待つ
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // ホスト名の表示
  Serial.print("Hostname: http://");
  Serial.print(WiFi.getHostname());
  Serial.println(".local");

  // IPアドレスの表示
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // サブネットマスクの表示
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  // ゲートウェイIPの表示
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());

  // DNSサーバーIPの表示
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  // MACアドレスの表示
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  // mDNSレスポンダーの開始
  if (MDNS.begin(hostname)) {
    Serial.println("MDNS responder started");
  }

}

void handleRoot() {

  // DHTセンサーから温度と湿度を読み取る
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

  // クライアントにJSONレスポンスを送信
  server.send(200, "application/json", json);

}
