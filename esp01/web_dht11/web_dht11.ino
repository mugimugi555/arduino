//----------------------------------------------------------------------------
// ライブラリのインストールとコンパイルコマンド
//----------------------------------------------------------------------------
// arduino-cli lib install "DHT sensor library"
// arduino-cli lib install "ArduinoJson"
// bash upload_esp01_web.sh web_dht11/web_dht11.ino wifissid wifipasswd hostname

//----------------------------------------------------------------------------
// ライブラリのインクルード
//----------------------------------------------------------------------------
#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード
#include <DHT.h>               // 温湿度センサーライブラリをインクルード
#include <ArduinoJson.h>       // ArduinoJsonライブラリをインクルード

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------
// WiFi SSID、パスワード、ホスト名の設定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

// DHTセンサーの設定
#define DHTPIN 2      // DHTセンサーを接続するピン（GPIO2: ESP-01で使用可能なピン）
#define DHTTYPE DHT11 // DHTセンサーの種類（DHT11 または DHT22）
DHT dht(DHTPIN, DHTTYPE);

// タイマーの変数
unsigned long previousMillis = 0; // 前回のシリアル出力時刻
const long interval = 5000;       // 5秒間隔

ESP8266WebServer server(80); // Webサーバーのインスタンスを作成

//----------------------------------------------------------------------------
// 初期設定
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200); // シリアル通信を開始
  dht.begin();          // DHTセンサーの初期化
  connectToWiFi();      // WiFi接続の開始
  setupWebServer();     // Webサーバーの設定

}

//----------------------------------------------------------------------------
// メインループ
//----------------------------------------------------------------------------
void loop() {

  server.handleClient();  // クライアントからの接続を処理
  MDNS.update();          // mDNSサービスの更新
  outputSensorData();     // 5秒ごとにセンサーのデータを出力

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

  server.on("/", handleRoot); // ルートパスにハンドラを設定
  server.begin();             // Webサーバー開始
  Serial.println("HTTP server started");

}

//----------------------------------------------------------------------------
// ルートパス処理関数
//----------------------------------------------------------------------------
void handleRoot() {
  // センサーのデータをJSON形式で取得
  String jsonResponse = getSensorDataJson();

  // レスポンスをクライアントに送信
  if (jsonResponse.isEmpty()) {
    server.send(500, "text/plain", "Sensor reading failed");
  } else {
    server.send(200, "application/json", jsonResponse);
  }
}

//----------------------------------------------------------------------------
// センサーのデータをJSON形式で取得する関数
//----------------------------------------------------------------------------
String getSensorDataJson() {
  // DHT センサーから温度と湿度を読み取る
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // センサーの読み取りエラーがないか確認
  if (isnan(temperature) || isnan(humidity)) {
    return ""; // エラーがあった場合は空文字を返す
  }

  // JSON形式のレスポンスを作成
  StaticJsonDocument<200> jsonDoc;      // JSONドキュメントを作成
  jsonDoc["temperature"] = temperature; // 温度を設定
  jsonDoc["humidity"]    = humidity;    // 湿度を設定

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
    if (!json.isEmpty()) {
      Serial.println(json);
    } else {
      Serial.println("Sensor reading failed");
    }
  }
}
