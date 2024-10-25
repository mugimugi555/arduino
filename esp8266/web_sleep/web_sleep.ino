#include <ESP8266WiFi.h>          // ESP8266用のWiFi機能を提供するライブラリ
#include <ESP8266WebServer.h>     // ESP8266上でWebサーバーを構築するためのライブラリ
#include <ArduinoJson.h>          // JSONデータを作成・解析するためのライブラリ
#include <DHT.h>                  // DHTセンサーライブラリ

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
