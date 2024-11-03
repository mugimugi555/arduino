/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"        # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncTCP"        # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "DHT sensor library" # DHT11やDHT22温湿度センサー用のライブラリ
arduino-cli lib install "NTPClient"           # NTP（Network Time Protocol）サーバーから時刻情報を取得するためのライブラリをインストール
arduino-cli lib install "LittleFS"            # ESP8266でLittleFSを使用するためのライブラリをインストール
arduino-cli lib install "Time"                # 時間関連の処理を行うためのライブラリをインストール

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

#include <ESP8266WiFi.h>       // ESP8266のWi-Fi機能を使用するためのライブラリ
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ
#include <ESPAsyncWebServer.h> // ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理を非同期で行い、複数のクライアントからのリクエストに同時に対応できるようにします。
#include <ArduinoJson.h>       // JSON形式のデータを簡単に扱うためのライブラリ
#include <NTPClient.h>         // NTP（Network Time Protocol）を使って時刻を取得するためのライブラリ
#include <WiFiUdp.h>           // UDP通信を行うためのライブラリ
#include <LittleFS.h>          // 小型ファイルシステム（LittleFS）を使用するためのライブラリ
#include <DHT.h>               // DHT温湿度センサーを使用するためのライブラリ
#include <TimeLib.h>           // 時間と日付を扱うためのライブラリ

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

// タスクを繰り返し実行する間隔（秒）。おそらくCSVファイルが肥大になるので、取得する間隔は大きいほうが良いです。
const long taskInterval = 1;

// NTPの設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1のタイムゾーン設定

// 時間の管理
const unsigned long ntpUpdateInterval = 3600000; // 1日ごとにNTPを更新

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

  // 温湿度センサーの開始
  dht.begin();

  // NTPクライアントの開始
  timeClient.begin();
  timeClient.update(); // 初回のNTP時間を取得

  // LittleFSの初期化
  setupLittleFS();

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

  // NTPの更新
  updateNtpTask();

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
// LittleFSの初期化
//----------------------------------------------------------------------------
void setupLittleFS() {

  // LittleFSの初期化
  if (!LittleFS.begin()) {
    Serial.println("LittleFSのマウントに失敗しました");
    return;
  }

  // LittleFSのフォーマット（要らないかも）
  if (LittleFS.format()) {
    Serial.println("LittleFSがフォーマットされました");
  } else {
    Serial.println("フォーマットに失敗しました");
  }

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
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {

  // CSVファイルをJSONで表示（１０件）
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = getCSVDataAsJSON();
    request->send(200, "application/json", jsonResponse);
  });

  // CSVファイルのダウンロード
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/data.csv", "text/plain", true);
    request->send(response); // レスポンスを送信
  });

  // CSVファイルの削除
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/data.csv")) {
      LittleFS.remove("/data.csv"); // ファイルを削除
      request->send(200, "text/plain", "ファイルが削除されました");
      Serial.println("ファイルが削除されました");
    } else {
      request->send(404, "text/plain", "ファイルが見つかりません");
    }
  });

  //
  server.begin();

}

//----------------------------------------------------------------------------
// CSVファイルをJSON文字列で表示
//----------------------------------------------------------------------------
String getCSVDataAsJSON() {

  String jsonStr = "[";
  File file = LittleFS.open("/data.csv", "r");
  if (!file) {
    Serial.println("ファイルを開けませんでした");
    return "[]"; // エラーがあった場合は空の配列を返す
  }

  const int numLinesToDisplay = 10; // 表示したい件数を指定する変数
  String lines[numLinesToDisplay];  // 配列のサイズを変数に基づいて定義
  int index = 0;

  // ファイルを最後まで読み込む
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) {
      // 最新の件数分の行を保存するために配列を更新
      lines[index % numLinesToDisplay] = line; // インデックスを指定された件数で割った余りを使って円環的に保存
      index++;
    }
  }

  // 下から指定した件数を逆順に表示
  for (int i = 0; i < numLinesToDisplay && index > i; i++) {
    String line = lines[(index - 1 - i) % numLinesToDisplay]; // 下から順に取得
    if (line.length() > 0) {
      // CSVの各行をJSON形式に変換
      String jsonLine = convertCSVLineToJSON(line);
      jsonStr += jsonLine + ",";
    }
  }

  // 最後のカンマを削除するために処理
  if (jsonStr.length() > 0) {
    jsonStr.remove(jsonStr.length() - 1);
  }

  //
  jsonStr += "]";

   // ファイルを閉じる
  file.close();

  //
  return jsonStr;

}

// 指定行のCSVをJSON文字列に変換
String convertCSVLineToJSON(String line) {

  //
  int firstCommaIndex  = line.indexOf(',');
  int secondCommaIndex = line.indexOf(',', firstCommaIndex  + 1);
  int thirdCommaIndex  = line.indexOf(',', secondCommaIndex + 1);

  //
  String datetime        = line.substring(0, firstCommaIndex);
  String temperature     = line.substring(firstCommaIndex  + 1, secondCommaIndex);
  String humidity        = line.substring(secondCommaIndex + 1, thirdCommaIndex);
  String discomfortIndex = line.substring(thirdCommaIndex  + 1);

  // JSON形式に整形
  StaticJsonDocument<256> doc;
  doc["datetime"]         = datetime; // DATETIME形式
  doc["temperature"]      = temperature.toFloat();
  doc["humidity"]         = humidity.toFloat();
  doc["discomfort_index"] = discomfortIndex.toFloat(); // 不快指数も追加
  doc["status"]           = 1;                         // ステータス (正常の場合は1)
  doc["message"]          = "正常に取得できました。";      // メッセージ (データ取得が成功したことを示す)
  doc["hostname"]         = hostname;                  // ホスト名 (デバイスの名前)
  doc["ipaddress"]        = WiFi.localIP().toString(); // IPアドレス (デバイスのネットワークアドレス)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//----------------------------------------------------------------------------
// センサーのデータを読み込み、CSV形式ファイルに書き込む
//----------------------------------------------------------------------------
String readAndSaveSensorData() {

  // センサーからのデータを取得
  float temperature     = dht.readTemperature();                // 温度
  float humidity        = dht.readHumidity();                   // 湿度
  float discomfortIndex = temperature + 0.36 * humidity + 41.2; // 不快指数の計算

  //
  // NTPから時間を取得
  String formattedTime = timeClient.getFormattedTime();

  // TimeLibを使ってDATETIME形式に変換
  time_t now = timeClient.getEpochTime();
  String datetime = String(year(now)) + "-" + String(month(now))  + "-" + String(day(now)) + " " +
                    String(hour(now)) + ":" + String(minute(now)) + ":" + String(second(now));

  // CSV形式で書き込む
  File file = LittleFS.open("/data.csv", "a"); // "a"は追記モード
  if (file) {

    //if (!isnan(temperature) && !isnan(humidity)) {

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

    //} else {
    //  Serial.println("センサーからのデータの取得に失敗しました");
    //}

    file.close(); // ファイルを閉じる

  } else {
    Serial.println("ファイルを開けませんでした");
  }

  // JSON形式に整形
  StaticJsonDocument<256> doc;
  doc["datetime"]         = datetime;                  // DATETIME形式
  doc["temperature"]      = temperature;               // 温度 (摂氏)
  doc["humidity"]         = humidity;                  // 湿度 (%)
  doc["discomfortIndex"]  = discomfortIndex;           // 不快指数 (相対的な快適さを示す指標)
  doc["status"]           = 1;                         // ステータス (正常の場合は1)
  doc["message"]          = "正常に取得できました。";      // メッセージ (データ取得が成功したことを示す)
  doc["hostname"]         = hostname;                  // ホスト名 (デバイスの名前)
  doc["ipaddress"]        = WiFi.localIP().toString(); // IPアドレス (デバイスのネットワークアドレス)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//----------------------------------------------------------------------------
// CSVで古いデーターを削除
//----------------------------------------------------------------------------
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
  tm.Year   = year - 1970; // TimeLibでは1970年からの年数を指定
  tm.Month  = month;
  tm.Day    = day;
  tm.Hour   = hour;
  tm.Minute = minute;
  tm.Second = second;

  // タイムスタンプを作成
  return makeTime(tm);

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// 1秒ごとに情報を表示する関数
void fetchAndShowTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;

    // 古いデータの削除
    deleteOldData();

    // データーの取得と表示
    Serial.println(readAndSaveSensorData());

  }

}

// １日ごとにNTPを更新する関数
void updateNtpTask() {

  static unsigned long lastMdnsMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMdnsMillis >= ntpUpdateInterval) {
    lastMdnsMillis = currentMillis;
    timeClient.update();
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
