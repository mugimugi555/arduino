#include <ArduinoJson.h>
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

// タイマーの変数
unsigned long previousMillis = 0; // 前回のシリアル出力時刻
const long interval = 5000;       // 5秒間隔

//
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

//
void loop() {

  server.handleClient();
  MDNS.update();  // mDNSサービスの更新

  // 5秒ごとにシリアル出力
  outputSensorData();

}

//
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

//
void handleRoot() {

  // DHT センサーから温度と湿度を読み取る
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // センサーの読み取りエラーがないか確認
  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "text/plain", "Sensor reading failed");
    return;
  }

  // JSONオブジェクトの作成
  StaticJsonDocument<200> doc;  // 必要に応じてサイズを調整

  // データの設定
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;

  // クライアントにJSONレスポンスを送信
  String json;
  serializeJson(doc, json);  // JSONデータを文字列に変換
  server.send(200, "application/json", json);

}

// センサーのデータをシリアル出力する関数
void outputSensorData() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis; // 現在の時刻を保存

    // DHT センサーから温度と湿度を読み取る
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // センサーの読み取りエラーがないか確認
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Sensor reading failed");
      return;
    }

    // CSV形式でシリアル出力
    Serial.print("Temperature,Humidity\n"); // ヘッダー
    Serial.print(temperature);
    Serial.print(",");
    Serial.println(humidity);

  }

}
