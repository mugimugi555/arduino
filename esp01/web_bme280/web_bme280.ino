//----------------------------------------------------------------------------
// ライブラリのインストールとコンパイルコマンド
//----------------------------------------------------------------------------
// arduino-cli lib install "Adafruit BME280 Library"
// arduino-cli lib install "Adafruit BusIO"
// arduino-cli lib install "Adafruit Unified Sensor"
// arduino-cli lib install "ArduinoJson"
// bash upload_esp01_web.sh web_bme280/web_bme280.ino wifissid wifipasswd hostname

//----------------------------------------------------------------------------
// ライブラリのインクルード
//----------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>      // ESP8266用のWi-Fiライブラリ
#include <ESP8266WebServer.h> // ESP8266用のWebサーバーライブラリ
#include <ESP8266mDNS.h>      // ESP8266用のmDNSライブラリ
#include <ArduinoJson.h>      // ArduinoJsonライブラリをインクルード

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

//----------------------------------------------------------------------------
// GPIO接続方法
//----------------------------------------------------------------------------

// ESP01とBME280センサーの接続:
// SCL (クロック信号) -> GPIO0 (D3ピン)
// SDA (データ信号)   -> GPIO2 (D4ピン)
// VCC -> 3.3V
// GND -> GND

// I2Cのピン設定
#define I2C_SCL 0  // GPIO0 (D3ピン)
#define I2C_SDA 2  // GPIO2 (D4ピン)

// BME280オブジェクトの作成
Adafruit_BME280 bme;

// Webサーバーのポート番号
ESP8266WebServer server(80);

// タイマーの変数
unsigned long previousMillis = 0; // 前回のシリアル出力時刻
const long interval = 5000;       // 5秒間隔

//----------------------------------------------------------------------------
// 初期設定
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  // I2Cピンの設定
  Wire.begin(I2C_SDA, I2C_SCL);

  // BME280センサーの初期化
  if (!bme.begin(0x76)) {
    Serial.println("BME280センサーが見つかりません。接続を確認してください。");
    while (1);
  }

  connectToWiFi();   // WiFi接続
  setupWebServer();  // Webサーバーの設定

}

//----------------------------------------------------------------------------
// メインループ
//----------------------------------------------------------------------------
void loop() {

  server.handleClient(); // クライアントからのリクエストを処理
  outputSensorData();   // 5秒ごとにLEDの状態をシリアル出力
  MDNS.update();         // mDNSサービスの更新

}

//----------------------------------------------------------------------------
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // mDNSサービスの開始
  Serial.println("");
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.println("===============================================");
  Serial.println("              Network Details                  ");
  Serial.println("===============================================");
  Serial.print("WebServer    : http://");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname     : http://");
  Serial.print(hostname);
  Serial.println(".local");
  Serial.print("IP address   : ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask  : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP   : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP       : ");
  Serial.println(WiFi.dnsIP());
  Serial.print("MAC address  : ");
  Serial.println(WiFi.macAddress());
  Serial.println("===============================================");

}

//----------------------------------------------------------------------------
// Webサーバー設定
//----------------------------------------------------------------------------
void setupWebServer() {

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

}

//----------------------------------------------------------------------------
// ルートパス処理関数
//----------------------------------------------------------------------------
void handleRoot() {

  // センサーのデータをJSON形式で取得
  String jsonResponse = getSensorDataJson();

  // レスポンスをクライアントに送信
  server.send(200, "application/json", jsonResponse);
  Serial.println(jsonResponse);

}

//----------------------------------------------------------------------------
// センサーのデータをJSON形式で取得する関数
//----------------------------------------------------------------------------
String getSensorDataJson() {

  // 温度、湿度、気圧の値を取得
  float temperature = bme.readTemperature();
  float humidity    = bme.readHumidity();
  float pressure    = bme.readPressure() / 100.0F; // PaをhPaに変換

  // JSON形式のレスポンスを作成
  StaticJsonDocument<200> jsonDoc;      // JSONドキュメントを作成
  jsonDoc["temperature"] = temperature; // 温度を設定
  jsonDoc["humidity"]    = humidity;    // 湿度を設定
  jsonDoc["pressure"]    = pressure;    // 気圧を設定
  jsonDoc["sensor"]      = "bme280";    // センサー名
  jsonDoc["hostname"]    = hostname;    // ホスト名
  jsonDoc["ipaddress"]   = WiFi.localIP().toString(); // IPアドレス

  // JSON形式の文字列を返す
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse); // JSONドキュメントを文字列にシリアル化

  return jsonResponse;

}

//----------------------------------------------------------------------------
// センサーのデータをシリアル出力する関数
//----------------------------------------------------------------------------
void outputSensorData() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis; // 現在の時刻を保存

    // センサーのデータをJSON形式で取得しシリアル出力
    String json = getSensorDataJson();
    Serial.println(json);

  }

}
