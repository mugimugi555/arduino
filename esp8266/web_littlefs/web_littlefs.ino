/*
arduino-cli lib install "ESP8266WiFi"        # ESP8266用のWiFi機能を提供するライブラリをインストール
arduino-cli lib install "ESP8266mDNS"        # ESP8266用のmDNS機能を提供するライブラリをインストール
arduino-cli lib install "ESP8266WebServer"   # ESP8266上でWebサーバーを構築するためのライブラリをインストール
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを作成・解析するためのライブラリをインストール
arduino-cli lib install "NTPClient"          # NTP（Network Time Protocol）サーバーから時刻情報を取得するためのライブラリをインストール
arduino-cli lib install "LittleFS"           # ESP8266でLittleFSを使用するためのライブラリをインストール
arduino-cli lib install "DHT sensor library" # DHTセンサー（温度・湿度センサー）用のライブラリをインストール
arduino-cli lib install "Time"               # 時間関連の処理を行うためのライブラリをインストール
*/

#include <ESP8266WiFi.h>       // ESP8266のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESP8266WebServer.h>  // ESP8266デバイスでWebサーバーを構築するためのライブラリ。HTTPリクエストの処理やWebページの提供が可能です。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <NTPClient.h>         // NTP（Network Time Protocol）サーバーから時刻情報を取得するためのライブラリ。正確な時刻を得るのに便利です。
#include <WiFiUdp.h>           // UDP通信を行うためのWiFiライブラリ。簡単にデータの送受信が可能です。
#include <LittleFS.h>          // フラッシュメモリをファイルシステムとして使用するためのライブラリ。データの保存に役立ちます。
#include <DHT.h>               // DHTセンサー（温度・湿度センサー）を使用するためのライブラリ。センサーのデータを簡単に取得できます。
#include <TimeLib.h>           // 時間関連の操作を行うためのライブラリ。時刻の管理や計算に使用します。

// WiFiの設定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// センサーとの接続方法
// DHT11センサー       ESP8266
// VCC  <---------->  3.3V または 5V
// GND  <---------->  GND
// DATA <---------->  D4 (GPIO 2)

#define DHTPIN D4       // DHTセンサーの接続ピン
#define DHTTYPE DHT11   // DHT11センサーを使用
DHT dht(DHTPIN, DHTTYPE);

// NTPの設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1のタイムゾーン設定

unsigned long lastNTPUpdate  = 0; // NTP更新時間
unsigned long lastSensorRead = 0; // センサー読み取り時間
const unsigned long sensorReadInterval = 2000; // 2秒ごとのセンサー読み取り
const unsigned long ntpUpdateInterval  = 3600000; // 1時間ごとのNTP更新

ESP8266WebServer server(80); // Webサーバーをポート80で作成

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  dht.begin();

  //
  showSplash();

  // Wi-Fi接続
  connectToWiFi();

  // NTPクライアントの開始
  timeClient.begin();
  timeClient.update(); // 初回のNTP時間を取得
  lastNTPUpdate = millis(); // NTPの初回更新時間を記録

  // LittleFSの初期化
  initializeLittleFS();

  // Webサーバーの開始
  initializeWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  unsigned long currentMillis = millis();

  // 2秒ごとにセンサーの値を取得してファイルに書き込む
  if (currentMillis - lastSensorRead >= sensorReadInterval) {
    readAndSaveSensorData();
    lastSensorRead = currentMillis;
  }

  // 1時間ごとにNTPの時刻を更新する
  if (currentMillis - lastNTPUpdate >= ntpUpdateInterval) {
    timeClient.update();
    lastNTPUpdate = currentMillis;
  }

  // クライアントリクエストを処理
  server.handleClient();

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

  Serial.print("Connecting to ");
  Serial.print(ssid);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  // mDNSサービスの開始
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  Serial.println("");
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
  Serial.println("-----------------------------------------------");
  Serial.println("");

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void initializeLittleFS() {

  // LittleFSの初期化
  if (!LittleFS.begin()) {
    Serial.println("LittleFSのマウントに失敗しました");
    return;
  }

  /*
  if (LittleFS.format()) {
    Serial.println("LittleFSがフォーマットされました");
  } else {
    Serial.println("フォーマットに失敗しました");
  }
  */

  // CSVファイルの存在を確認し、無ければ作成
  if (!LittleFS.exists("/data.csv")) {
    File file = LittleFS.open("/data.csv", "w"); // "w"は新規作成モード
    if (!file) {
      Serial.println("CSVファイルの作成に失敗しました");
    } else {
      //file.println("datetime,temperature,humidity,discomfort_index"); // ヘッダーを書き込む
      file.close();
      Serial.println("CSVファイルを作成しました");
    }
  }

}

//----------------------------------------------------------------------------
// センサーのデータをCSV形式で取得してファイルに書き込む
//----------------------------------------------------------------------------
void readAndSaveSensorData() {

  // センサーからのデータを取得
  float temperature     = dht.readTemperature();
  float humidity        = dht.readHumidity();
  float discomfortIndex = calculateDiscomfortIndex(temperature, humidity);

  // NTPから時間を取得
  String formattedTime = timeClient.getFormattedTime();

  // TimeLibを使ってDATETIME形式に変換
  time_t now = timeClient.getEpochTime();
  String datetime = String(year(now)) + "-" + String(month(now))  + "-" + String(day(now)) + " " +
                    String(hour(now)) + ":" + String(minute(now)) + ":" + String(second(now));

  // CSV形式で書き込む
  File file = LittleFS.open("/data.csv", "a"); // "a"は追記モード
  if (file) {
    if (!isnan(temperature) && !isnan(humidity)) {

      //
      file.print(datetime); // DATETIME形式の時間を追加
      file.print(",");
      file.print(temperature);
      file.print(",");
      file.print(humidity);
      file.print(",");
      file.print(discomfortIndex); // 不快指数も書き込む
      file.print(",");
      file.print(hostname); // ホスト名
      file.print(",");
      file.println(WiFi.localIP().toString()); // IPアドレス

    } else {
      Serial.println("センサーからのデータの取得に失敗しました");
    }
    file.close(); // ファイルを閉じる
  } else {
    Serial.println("ファイルを開けませんでした");
  }

  // JSON形式に整形
  StaticJsonDocument<256> doc;
  doc["datetime"]         = datetime; // DATETIME形式
  doc["temperature"]      = temperature;
  doc["humidity"]         = humidity;
  doc["discomfort_index"] = discomfortIndex; // 不快指数も追加
  doc["hostname"]         = hostname;                  // ホスト名
  doc["ipaddress"]        = WiFi.localIP().toString(); // IPアドレス
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println( jsonString );

  //
  deleteOldData();

}

// 不快指数を計算する関数
float calculateDiscomfortIndex(float temperature, float humidity) {
  return (temperature + humidity) / 2; // 簡単な不快指数の計算
}

//
void deleteOldData() {

  // 現在の時刻を取得
  time_t now = timeClient.getEpochTime(); // NTPから取得した時刻を使う必要があります
  time_t oneWeekAgo = now - (7 * 24 * 60 * 60); // 1週間前のタイムスタンプ

  // 元のCSVファイルを開く
  File originalFile = LittleFS.open("/data.csv", "r");
  if (!originalFile) {
    Serial.println("元のファイルが見つかりません");
    return;
  }

  // 新しいファイルを作成
  File newFile = LittleFS.open("/new_data.csv", "w");
  if (!newFile) {
    Serial.println("新しいファイルの作成に失敗しました");
    originalFile.close();
    return;
  }

  // 元のファイルを行ごとに読み込む
  while (originalFile.available()) {

    String line = originalFile.readStringUntil('\n');

    // 行の先頭から日時を取得 (仮に最初のカンマまでが日時とする)
    String datetime = line.substring(0, line.indexOf(','));

    // タイムスタンプを取得して比較
    time_t timestamp = getTimestampFromDateTime(datetime); // 時刻を取得する関数を実装する必要があります

    // 1週間より新しいデータのみ新しいファイルに書き込む
    if (timestamp > oneWeekAgo) {
      newFile.println(line);
    }

  }

  // ファイルを閉じる
  originalFile.close();
  newFile.close();

  // 元のファイルを削除し、新しいファイルをリネーム
  LittleFS.remove("/data.csv");
  LittleFS.rename("/new_data.csv", "/data.csv");

  //Serial.println("古いデータを削除しました");

}

//
time_t getTimestampFromDateTime(String dateTime) {

  int year, month, day, hour, minute, second;

  // 文字列から各要素を抽出する（例: "2024-10-28 14:30:00"）
  sscanf(dateTime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

  // tmElements_t構造体の初期化
  tmElements_t tm;
  tm.Year = year - 1970; // TimeLibでは1970年からの年数を指定
  tm.Month = month;
  tm.Day = day;
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = second;

  // タイムスタンプを作成
  return makeTime(tm);

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

// Webサーバーの初期化
void initializeWebServer() {

  //
  server.on("/", []() {
    server.send(200, "application/json", getCSVDataAsJSON());
  });

  //
  server.on("/data.csv", []() {
    File file = LittleFS.open("/data.csv", "r");
    if (!file) {
      server.send(404, "text/plain", "ファイルが見つかりません");
      return;
    }
    server.streamFile(file, "text/csv");
    file.close();
  });

  //
  server.on("/delete", []() {
    if (LittleFS.exists("/data.csv")) {
      LittleFS.remove("/data.csv"); // ファイルを削除
      server.send(200, "text/plain", "ファイルが削除されました");
      Serial.println("ファイルが削除されました");
    } else {
      server.send(404, "text/plain", "ファイルが見つかりません");
    }
  });

  //
  server.begin();

}

// CSVファイルのデータをJSON形式で取得
String getCSVDataAsJSON() {

  String jsonOutput = "[";
  File file = LittleFS.open("/data.csv", "r");
  if (!file) {
    Serial.println("ファイルを開けませんでした");
    return "[]"; // エラーがあった場合は空の配列を返す
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) {
      // CSVの各行をJSON形式に変換
      String jsonLine = convertCSVLineToJSON(line);
      jsonOutput += jsonLine + ",";
    }
  }

  // 最後のカンマを削除し、配列を閉じる
  if (jsonOutput.endsWith(",")) {
    jsonOutput.remove(jsonOutput.length() - 1);
  }
  jsonOutput += "]";

  file.close();
  return jsonOutput;

}

// CSVの行をJSON形式に変換
String convertCSVLineToJSON(String line) {

  //
  int firstCommaIndex  = line.indexOf(',');
  int secondCommaIndex = line.indexOf(',', firstCommaIndex + 1);
  int thirdCommaIndex  = line.indexOf(',', secondCommaIndex + 1);

  //
  String datetime        = line.substring(0, firstCommaIndex);
  String temperature     = line.substring(firstCommaIndex + 1, secondCommaIndex);
  String humidity        = line.substring(secondCommaIndex + 1, thirdCommaIndex);
  String discomfortIndex = line.substring(thirdCommaIndex + 1);

  // JSON形式に整形
  StaticJsonDocument<256> doc;
  doc["datetime"]         = datetime; // DATETIME形式
  doc["temperature"]      = temperature.toFloat();
  doc["humidity"]         = humidity.toFloat();
  doc["discomfort_index"] = discomfortIndex.toFloat(); // 不快指数も追加
  doc["hostname"]         = hostname;                  // ホスト名
  doc["ipaddress"]        = WiFi.localIP().toString(); // IPアドレス
  String jsonString;
  serializeJson(doc, jsonString);

  return jsonString;

}
