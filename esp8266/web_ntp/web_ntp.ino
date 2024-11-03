/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"       # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266mDNS"       # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "ESPAsyncTCP"       # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ESPAsyncWebServer" # ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理やレスポンスを非同期に行うことができ、複数のクライアントからのリクエストに同時に対応できます。
arduino-cli lib install "ArduinoJson"       # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "WiFiUdp"           # UDP通信を使用するためのWiFiライブラリ
arduino-cli lib install "NTPClient"         # NTP（Network Time Protocol）を使用して、正確な時刻を取得するためのライブラリ
arduino-cli lib install "Time"              # 時間や日付の計算、フォーマットを行うためのライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // 非同期Webサーバーライブラリ。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <WiFiUdp.h>           // UDP通信を行うためのWiFiライブラリ。簡単にデータの送受信が可能です。
#include <NTPClient.h>         // NTP（Network Time Protocol）サーバーから時刻情報を取得するためのライブラリ。正確な時刻を得るのに便利です。
#include <TimeLib.h>           // TimeLibライブラリをインクルード

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// タイムゾーン（秒単位）、更新間隔（ミリ秒単位）、NTPサーバー名の定数
const char* NTP_SERVER     = "pool.ntp.org"; // NTPサーバー
const long TIMEZONE_OFFSET = 9 * 3600;       // JST (UTC+9)
const int UPDATE_INTERVAL  = 60000;          // 1分（60000ミリ秒）

// NTPClientインスタンスの生成
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, NTP_SERVER, TIMEZONE_OFFSET, UPDATE_INTERVAL);

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

  // 起動画面の表示
  showStartup();

  // WiFi接続
  connectToWiFi();

  // NTPクライアント開始
  ntpClient.begin();
  ntpClient.update();
  setTime(ntpClient.getEpochTime());

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

  //
  syncNtpTask();

  // ホスト名の更新
  updateMdnsTask();

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

  // JSONオブジェクトを作成
  StaticJsonDocument<200> doc;

  // NTP情報
  doc["datetime"]        = formatDatetime();
  doc["ntp_server"]      = NTP_SERVER;
  doc["timezone_offset"] = TIMEZONE_OFFSET;
  doc["update_interval"] = UPDATE_INTERVAL;
  doc["epoch_time"]      = ntpClient.getEpochTime();
  doc["formatted_time"]  = ntpClient.getFormattedTime();
  //doc["time_offset"]     = ntpClient.getTimeOffset();
  //doc["ntp_server"]      = ntpClient.getPoolServerName();
  //doc["update_interval"] = ntpClient.getUpdateInterval();

  // 基本情報
  doc["hostname"]        = hostname;
  doc["ipaddress"]       = WiFi.localIP().toString();
  doc["status"]          = 1;
  doc["message"]         = "正常に取得できました。";

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

// DATETIME形式のStringを返す関数
String formatDatetime() {

  String datetime = String(year()) + "-" +
                    (month()  < 10 ? "0" + String(month())  : String(month()))  + "-" +
                    (day()    < 10 ? "0" + String(day())    : String(day()))    + " " +
                    (hour()   < 10 ? "0" + String(hour())   : String(hour()))   + ":" +
                    (minute() < 10 ? "0" + String(minute()) : String(minute())) + ":" +
                    (second() < 10 ? "0" + String(second()) : String(second()));

  return datetime;

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

// 1時間ごとにNTPと同期する関数
void syncNtpTask() {

  static unsigned long lastSyncMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastSyncMillis >= 3600000) {
    lastSyncMillis = currentMillis;
    ntpClient.update();
    setTime(ntpClient.getEpochTime());
    Serial.println("NTP time synced");
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
