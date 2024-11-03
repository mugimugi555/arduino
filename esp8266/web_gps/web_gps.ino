/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"       # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266mDNS"       # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "ArduinoJson"       # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "TinyGPSPlus"       # GPSモジュールからの位置情報を扱うためのライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_gps/web_gps.ino wifissid wifipasswd hostname

*****************************************************************************/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理を非同期で行い、複数のクライアントからのリクエストに同時に対応できるようにします。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <SoftwareSerial.h>    // ソフトウェアシリアル通信を可能にするライブラリ。GPSモジュールなど、追加のシリアルポートが必要なデバイスに使用します。
#include <TinyGPS++.h>         // GPSモジュールから位置情報を取得するためのライブラリ。緯度、経度、速度などの情報を簡単に扱えます。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// センサーとの接続方法
// E108 GN02Dセンサー  ESP8266
// VCC  <---------->  3.3V または 5V
// GND  <---------->  GND
// RX   <---------->  D5 (GPIO 14)  // ESP8266のTXと接続
// TX   <---------->  D6 (GPIO 12)  // ESP8266のRXと接続

TinyGPSPlus gps;
SoftwareSerial ss(D1, D2);  // D1=RX, D2=TX

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 1;

// ポート80で非同期Webサーバーを初期化
AsyncWebServer server(80);

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  // RXRTの通信をGPSモジュールのボーレートに合わせる
  ss.begin(9600);

  // 起動画面の表示
  showStartup();

  // WiFi接続
  connectToWiFi();

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

  // ホスト名の更新
  updateMdnsTask();

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  /*
  // GPSデータの処理
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      sendGpsData();
    }
  }
  */

}

//----------------------------------------------------------------------------
// 起動画面の表示
//----------------------------------------------------------------------------
void showStartup() {

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

  // ボード名を表示
  Serial.print("Board         : ");
  Serial.println(ARDUINO_BOARD);

  // CPUの周波数を表示
  Serial.print("CPU Frequency : ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  // フラッシュサイズを表示
  Serial.print("Flash Size    : ");
  Serial.print(ESP.getFlashChipSize() / 1024);
  Serial.println(" KB");

  // 空きヒープメモリを表示
  Serial.print("Free Heap     : ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" B");

  // フラッシュ速度を取得
  Serial.print("Flash Speed   : ");
  Serial.print(ESP.getFlashChipSpeed() / 1000000);
  Serial.println(" MHz");

  // チップIDを取得
  Serial.print("Chip ID       : ");
  Serial.println(ESP.getChipId());

  // SDKバージョンを取得
  Serial.print("SDK Version   : ");
  Serial.println(ESP.getSdkVersion());

  Serial.println("===============================================");
  Serial.println("");

}

//----------------------------------------------------------------------------
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  Serial.print("Connected to ");
  Serial.println(ssid);

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
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {

  // ルートへのアクセス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = createJson();
    request->send(200, "application/json", jsonResponse);
  });

  // Webサーバーを開始
  server.begin();

}

// 取得されるデータをJSON形式で生成
String createJson() {

  //
  gps.encode(ss.read());

  // JSONオブジェクトを作成
  StaticJsonDocument<256> doc;
  doc["latitude"]   = gps.location.lat();
  doc["longitude"]  = gps.location.lng();
  doc["altitude"]   = gps.altitude.meters();
  doc["speed"]      = gps.speed.kmph();
  doc["satellites"] = gps.satellites.value();

  // yyyy-mm-dd hh:ii:ss の日付と時刻を生成
  String date = String(gps.date.year()) + "-" +
                (gps.date.month() < 10 ? "0" : "") + String(gps.date.month()) + "-" +
                (gps.date.day()   < 10 ? "0" : "") + String(gps.date.day());

  String time = (gps.time.hour()   < 10 ? "0" : "") + String(gps.time.hour())   + ":" +
                (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + ":" +
                (gps.time.second() < 10 ? "0" : "") + String(gps.time.second());
  doc["datetime"] = date + " " + time;

  //
  doc["status"]    = 1;                         // ステータス (正常の場合は1)
  doc["message"]   = "正常に取得できました。";      // メッセージ (データ取得が成功したことを示す)
  doc["hostname"]  = hostname;                  // ホスト名 (デバイスの名前)
  doc["ipaddress"] = WiFi.localIP().toString(); // IPアドレス (デバイスのネットワークアドレス)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// taskInterval 秒ごとに情報を表示する関数
void fetchAndShowTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;
    Serial.println(createJson());
  }

}

// 0.5秒ごとにホスト名を更新する関数
void updateMdnsTask() {

  static unsigned long lastMdnsMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMdnsMillis >= 500) {
    lastMdnsMillis = currentMillis;
    MDNS.update();
  }

}
