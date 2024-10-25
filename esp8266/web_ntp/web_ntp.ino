// arduino-cli lib install "NTPClient"

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード
#include <WiFiClient.h>        // WiFiクライアント用ライブラリをインクルード
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID";   // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME";   // ESP8266のホスト名

// NTPクライアント設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000);  // JST (UTC+9)

// Webサーバー設定
ESP8266WebServer server(80);

// LEDピンとセンサー変数を定義
const int led = 2; // GPIOピン番号に置き換え
int sensorValue = 0;

//
void setup() {

  Serial.begin(115200);
  pinMode(led, OUTPUT);

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

//
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

}

//
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

// ルートURLにアクセスした際の処理
void handleRoot() {

  digitalWrite(led, HIGH); // LEDをONにする

  // JSONオブジェクトを作成
  createJson();

  // HTTPレスポンスを送信
  String jsonResponse = createJson();
  server.send(200, "application/json", jsonResponse);

  digitalWrite(led, LOW); // LEDをOFFにする

}

// GPSデータをJSON形式で生成
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

// 日付と時間をフォーマットする関数
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
