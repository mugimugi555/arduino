/*

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"             # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"             # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncTCP"             # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ESPAsyncWebServer"       # ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理やレスポンスを非同期に行うことができ、複数のクライアントからのリクエストに同時に対応できます。
arduino-cli lib install "ArduinoJson"             # JSON形式のデータを簡単に作成、解析するためのライブラリ。API通信やデータの保存に役立ち、シリアライズやデシリアライズが簡単に行えます。
arduino-cli lib install "Adafruit BME280 Library" # Adafruit社のBME280センサー用ライブラリ。温度、湿度、気圧のデータ取得をサポートし、I2C通信を介してセンサーから情報を取得できます。

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_bme280/web_bme280.ino wifissid wifipasswd hostname

*/

#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理を非同期で行い、複数のクライアントからのリクエストに同時に対応できるようにします。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ち、データのシリアライズとデシリアライズが容易に行えます。
#include <Wire.h>              // I2C通信を使用するための標準ライブラリ。BME280などのI2Cデバイスと通信するために必要で、マスターとスレーブ間のデータ送受信を管理します。
#include <Adafruit_Sensor.h>   // Adafruit社製センサー用の統一インターフェースライブラリ。各種センサーからのデータ取得に役立ち、異なるセンサー間での互換性を提供します。
#include <Adafruit_BME280.h>   // BME280センサーのためのライブラリ。温度、湿度、気圧のデータ取得が簡単に行え、センサーの初期化やデータ読み取りを簡素化します。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// センサーとの接続方法
// BME280センサー      ESP8266
// VCC  <---------->  3.3V
// GND  <---------->  GND
// SDA  <---------->  D2 (GPIO 4)
// SCL  <---------->  D1 (GPIO 5)

Adafruit_BME280 bme;       // BME280オブジェクトの作成
AsyncWebServer server(80); // 非同期Webサーバーの初期化

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  
  //
  showSplash();

  // BME280センサーの初期化
  if (!bme.begin(0x76)) {
    Serial.println("BME280センサーが見つかりません。接続を確認してください。");
    while (1);
  }

  // WiFi接続
  connectToWiFi();

  // ルートURLへのハンドラを設定
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String jsonResponse = createJson();
    request->send(200, "application/json", jsonResponse);
  });

  // Webサーバーの開始
  server.begin();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  // 1秒ごとに情報を表示
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    Serial.println(createJson());
  }

  // ホスト名の更新
  MDNS.update();

}

//----------------------------------------------------------------------------
// スプラッシュ画面の表示
//----------------------------------------------------------------------------
void showSplash(){

  // figlet ESP8266
  Serial.println("");
  Serial.println("");
  Serial.println("===============================================");
  Serial.println("  _____ ____  ____  ___ ____   __    __");
  Serial.println("  | ____/ ___||  _ \\( _ )___ \\ / /_  / /_  ");
  Serial.println("  |  _| \\___ \\| |_) / _ \\ __) | '_ \\| '_ \\ ");
  Serial.println("  | |___ ___) |  __/ (_) / __/| (_) | (_) |");
  Serial.println("  |_____|____/|_|   \\___/_____|\\___/ \\___/ ");
  Serial.println("");
  Serial.println("===============================================");
  Serial.println("");

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
  Serial.println("");

}

//----------------------------------------------------------------------------
// Webサーバー系
//----------------------------------------------------------------------------

// 取得されるデータをJSON形式で生成
String createJson() {

  // 不快指数の計算
  float discomfortIndex = temperature + 0.36 * humidity + 41.2;

  // JSONオブジェクトを作成
  StaticJsonDocument<200> doc;
  doc["temperature"]     = bme.readTemperature();       // 温度
  doc["humidity"]        = bme.readHumidity();          // 湿度
  doc["discomfortIndex"] = discomfortIndex;             // 不快指数
  doc["pressure"]        = bme.readPressure() / 100.0F; // PaをhPaに変換
  doc["hostname"]        = hostname;                    // ホスト名
  doc["ipaddress"]       = WiFi.localIP().toString();   // IPアドレス

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

