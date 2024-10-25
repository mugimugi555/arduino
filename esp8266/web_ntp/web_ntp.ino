/*

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"       # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266WebServer"  # ESP8266上でWebサーバー機能を実装するためのライブラリ
arduino-cli lib install "ESP8266mDNS"       # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "WiFiClient"        # WiFi経由でのTCP/IP通信を行うためのクライアントライブラリ
arduino-cli lib install "WiFiUdp"           # UDP通信を使用するためのWiFiライブラリ
arduino-cli lib install "NTPClient"         # NTP（Network Time Protocol）を使用して、正確な時刻を取得するためのライブラリ
arduino-cli lib install "ArduinoJson"       # JSON形式のデータを簡単に作成、解析するためのライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266WebServer.h>  // ESP8266デバイスでWebサーバーを構築するためのライブラリ。HTTPリクエストの処理やWebページの提供が可能です。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <WiFiClient.h>        // WiFi経由でTCP/IP通信を行うためのクライアントライブラリ。サーバーと通信する際に利用します。
#include <WiFiUdp.h>           // UDP通信を行うためのWiFiライブラリ。簡単にデータの送受信が可能です。
#include <NTPClient.h>         // NTP（Network Time Protocol）サーバーから時刻情報を取得するためのライブラリ。正確な時刻を得るのに便利です。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID";   // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME";   // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// NTPクライアント設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000);  // JST (UTC+9)

// Webサーバー設定
ESP8266WebServer server(80);

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);

  //
  showSplash();

  // WiFi接続
  connectToWiFi();

  // NTPクライアント開始
  timeClient.begin();
  timeClient.update();

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);

  // Webサーバーの開始
  server.begin();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  // 1秒ごとに時間を表示
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    Serial.println(createJson());
  }

  // 1時間に1回NTPで同期
  if (currentMillis % 3600000 == 0) {
    timeClient.update();
    Serial.println("NTP time synced");
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

// ルートURLにアクセスした際の処理
void handleRoot() {

  // JSONオブジェクトを作成
  createJson();

  // HTTPレスポンスを送信
  String jsonResponse = createJson();
  server.send(200, "application/json", jsonResponse);

}

// 取得されるデータをJSON形式で生成
String createJson() {

  // NTPクライアントから現在のエポック時間を取得
  unsigned long epochTime = timeClient.getEpochTime();
  String currentDateTime = formatDateTime(epochTime);

  // JSONオブジェクトを作成
  StaticJsonDocument<200> doc;
  doc["datetime"]  = currentDateTime;           // yyyy-mm-dd hh:ii:ss
  doc["hostname"]  = hostname;                  // ホスト名
  doc["ipaddress"] = WiFi.localIP().toString(); // IPアドレス

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//----------------------------------------------------------------------------
// 日付関係
//----------------------------------------------------------------------------

//日付と時間をフォーマットする関数
String formatDateTime(unsigned long epochTime) {

  int year = 1970;
  int month, day, hour, minute, second;

  unsigned long days = epochTime / 86400L;
  unsigned long remainingSeconds = epochTime % 86400L;

  while ((days >= (isLeapYear(year) ? 366 : 365))) {
    days -= (isLeapYear(year) ? 366 : 365);
    year++;
  }

  month = 1;
  while (days >= daysInMonth(month, year)) {
    days -= daysInMonth(month, year);
    month++;
  }
  day = days + 1;

  hour = remainingSeconds / 3600;
  remainingSeconds %= 3600;
  minute = remainingSeconds / 60;
  second = remainingSeconds % 60;

  char dateTime[20];
  sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

  return String(dateTime);

}

// 閏年を判定する関数
bool isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 月の日数を取得する関数
int daysInMonth(int month, int year) {
  if (month == 2) return isLeapYear(year) ? 29 : 28;
  if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
  return 31;
}
