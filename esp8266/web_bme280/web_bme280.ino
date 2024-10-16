#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h> // ESP8266用のWi-Fiライブラリ
#include <ESP8266WebServer.h> // ESP8266用のWebサーバーライブラリ

// Wi-FiのSSIDとパスワード
const char* ssid = "YOUR_SSID"; // 自分のWi-FiのSSIDに置き換えてください
const char* password = "YOUR_PASSWORD"; // 自分のWi-Fiのパスワードに置き換えてください

// BME280オブジェクトの作成
Adafruit_BME280 bme;

// Webサーバーのポート番号
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Wi-Fi接続の初期化
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi!");

  // BME280センサーの初期化
  if (!bme.begin(0x76)) {
    Serial.println("BME280センサーが見つかりません。接続を確認してください。");
    while (1);
  }

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // クライアントからのリクエストを処理
}

// ルートURLへのリクエストハンドラ
void handleRoot() {
  // 温度、湿度、気圧の値を取得
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F; // PaをhPaに変換

  // JSON形式のレスポンスを作成
  String message = String("{\"temperature\":") + temperature + 
                   String(",\"humidity\":") + humidity + 
                   String(",\"pressure\":") + pressure + "}";

  // レスポンスをクライアントに送信
  server.send(200, "application/json", message);
}
