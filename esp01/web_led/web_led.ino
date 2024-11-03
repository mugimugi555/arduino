//----------------------------------------------------------------------------
// ライブラリのインストールとコンパイルコマンド
//----------------------------------------------------------------------------
// arduino-cli lib install "ArduinoJson"
// bash upload_esp01_web.sh web_led/web_led.ino wifissid wifipasswd hostname

//----------------------------------------------------------------------------
// ライブラリのインクルード
//----------------------------------------------------------------------------
#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード
#include <ArduinoJson.h>       // ArduinoJsonライブラリをインクルード

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

// LEDピン
const int ledPin = 2;

// Webサーバー
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

  // LEDピンの初期化
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // 初期状態はオフ

  connectToWiFi();   // WiFi接続
  setupWebServer();  // Webサーバーの設定

}

//----------------------------------------------------------------------------
// メインループ
//----------------------------------------------------------------------------
void loop() {
  server.handleClient(); // クライアントリクエストの処理
  outputLEDState();      // 5秒ごとにLEDの状態をシリアル出力
  MDNS.update();         // mDNSサービスの更新
}

//----------------------------------------------------------------------------
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {

  WiFi.hostname(hostname); // ホスト名を設定
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

  // Webサーバーのルートパスにハンドラを設定
  server.on("/", handleRoot);
  server.on("/on", handleLEDOn);
  server.on("/off", handleLEDOff);
  server.begin(); // Webサーバーを開始
  Serial.println("HTTP server started");

}

//----------------------------------------------------------------------------
// ルートパス処理関数
//----------------------------------------------------------------------------
void handleRoot() {

  // HTMLページを生成
  String html = "<html><body>";
  html += "<h1>ESP1 LED on / off</h1>";
  html += "<p><a href='/on'>Turn LED ON</a></p>";
  html += "<p><a href='/off'>Turn LED OFF</a></p>";
  html += "</body></html>";

  // レスポンス送信
  server.send(200, "text/html", html);

}

//----------------------------------------------------------------------------
// LED処理関数
//----------------------------------------------------------------------------

// LED ONハンドラ
void handleLEDOn() {
  digitalWrite(ledPin, HIGH);  // LEDオン
  server.send(200, "text/plain", "LED is ON");
  outputLEDStateJson();
}

// LED OFFハンドラ
void handleLEDOff() {
  digitalWrite(ledPin, LOW);  // LEDオフ
  server.send(200, "text/plain", "LED is OFF");
  outputLEDStateJson();
}

//----------------------------------------------------------------------------
// LED状態出力関数
//----------------------------------------------------------------------------

void outputLEDState() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // 現在の時刻を保存
    outputLEDStateJson();
  }

}

void outputLEDStateJson() {

  // LEDの状態を取得
  String ledState = digitalRead(ledPin) == HIGH ? "ON" : "OFF";

  // JSON形式で出力
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["ledState"]  = ledState; // LEDの状態
  jsonDoc["hostname"]  = hostname; // ホスト名
  jsonDoc["ipaddress"] = WiFi.localIP().toString(); // IPアドレス

  String json;
  serializeJson(jsonDoc, json);
  Serial.println(json); // LEDの状態をシリアル出力

}

