#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> // BME280用のライブラリ

Adafruit_BME280 bme; // BME280オブジェクトの作成

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP32のホスト名

// LEDピン
const int led = 13;

// Webサーバー (ポート80で動作)
WebServer server(80);

void setup(void) {

  Serial.begin(115200);

  // LEDの初期化
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // WiFi接続
  connectToWiFi();

  // ルートハンドラの定義
  server.on("/", handleRoot);

  // HTTPサーバーの開始
  server.begin();
  Serial.println("HTTP server started");

  // BME280センサーの初期化
  if (!bme.begin(0x76)) {  // I2Cアドレス0x76を使用
    Serial.println("BME280センサーが見つかりません。接続を確認してください。");
    while (1) {}
  }

}

void loop(void) {

  // クライアントからのリクエストを処理
  server.handleClient();

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
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

}

void handleRoot() {

  // LEDをオンにする
  digitalWrite(led, 1);

  // センサーからデータを読み取る
  float temperature = bme.readTemperature(); // 温度 (°C)
  float pressure = bme.readPressure() / 100.0f; // 気圧 (hPa)
  float humidity = bme.readHumidity(); // 湿度 (%)

  // JSON形式でレスポンスを作成
  String message = String('{"temperature":{"value":') + temperature  + ',"unit":"*C"},'  +
                   String('"pressure":{"value":')    + pressure     + ',"unit":"hPa"},' +
                   String('"humidity":{"value":')    + humidity     + ',"unit":"%"}'    + "}";

  // JSONレスポンスをクライアントに送信
  server.send(200, "application/json", message);

  // シリアルモニタにCSV形式でデータを出力
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.println(humidity);

  // LEDをオフにする
  digitalWrite(led, 0);

}
