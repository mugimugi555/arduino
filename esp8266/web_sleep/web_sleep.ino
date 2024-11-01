#include <ESP8266WiFi.h>          // ESP8266用のWiFi機能を提供するライブラリ
#include <ESP8266WebServer.h>     // ESP8266上でWebサーバーを構築するためのライブラリ
#include <ArduinoJson.h>          // JSONデータを作成・解析するためのライブラリ
#include <DHT.h>                  // DHTセンサーライブラリ

/*
通常動作（WiFi接続）: 約 70mA ~ 250mA
ESP8266がフル稼働している状態で、WiFi接続が維持され、CPUやその他の機能がフルに利用可能です。WiFi信号の強さや、データ転送の頻度により、消費電力が変動します。

Modem Sleepモード: 約 15mA ~ 20mA
WiFi接続を維持したまま、送受信が不要な間にRFモジュールを停止します。CPUは通常通り動作しており、WiFiを待機状態にすることで消費電力を抑えますが、すぐにネットワークに再接続が可能です。

Light Sleepモード: 約 0.9mA ~ 1.2mA
WiFi接続を維持しつつ、CPUや一部の周辺機能を停止して消費電力を抑えます。ネットワークからのデータ受信の際に自動的に再開できるため、応答性を残しつつも消費電力が大幅に低減されます。

Deep Sleepモード: 約 20µA ~ 100µA
最も消費電力が少ないモードで、CPU、WiFi、RFモジュールの全てが停止し、リセットピンかタイマーの信号で再起動が必要です。主にセンサーの測定やデータ収集を間欠的に行うバッテリー駆動のアプリケーションに適していますが、起動のたびにWiFi接続が必要になります。
*/

// WiFi接続情報
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

ESP8266WebServer server(80);
DHT dht(D4, DHT11); // DHT11センサーのデータピンをD4に接続

float temperature = 0.0;
float humidity = 0.0;

void setup() {
  Serial.begin(115200);

  // WiFiに接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // WebサーバールートURLへのハンドラ
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");

  // DHTセンサーの初期化
  dht.begin();
}

void loop() {
  // Webサーバーのクライアント処理
  server.handleClient();

  // 5秒ごとにDHT11センサーの値を取得
  delay(5000);
  ESP.deepSleep(5000 * 1000, WAKE_RF_DEFAULT);  // Light Sleepモード

  // 温度と湿度の測定
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

// JSONデータを返すWebサーバーのハンドラ
void handleRoot() {
  // JSONオブジェクトを作成
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["unit"] = "Celsius";

  // JSONデータをシリアライズして送信
  String jsonData;
  serializeJson(jsonDoc, jsonData);
  server.send(200, "application/json", jsonData);
}
