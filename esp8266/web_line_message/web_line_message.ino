/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"       # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266mDNS"       # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "ArduinoJson"       # JSON形式のデータを簡単に作成、解析するためのライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_line_message/web_line_message.ino wifissid wifipasswd hostname

*****************************************************************************/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESP8266HTTPClient.h> // HTTPリクエストを送信するためのクライアント機能を提供するライブラリ。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// https://developers.line.biz/console/
const char* providerUserId     = "PROVIERUSERID"     ; // LINE Develop -> プロバイダーチャネル -> チャネル基本設定 -> ユーザーID
const char* channelAccessToken = "CHANNELACCESSTOKEN"; // LINE Develop -> プロバイダーチャネル -> Messaging API設定 -> チャネルアクセストークン（長期）

// センサーとの接続方法
// PIRセンサー         ESP8266
// VCC  <---------->  5V (または3.3V)
// GND  <---------->  GND
// OUT  <---------->  D5 (GPIO 14)

#define PIR_PIN 5  // PIRセンサーのピン

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 10;

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  //
  pinMode(PIR_PIN, INPUT);

  // 起動画面の表示
  showStartup();

  // WiFi接続
  connectToWiFi();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

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
// LINE API送信関係
//----------------------------------------------------------------------------
String createJson(String message) {

  // JSONオブジェクトの作成
  StaticJsonDocument<200> doc; // ドキュメントのサイズは適宜変更

  doc["to"] = providerUserId;

  JsonArray messages = doc.createNestedArray("messages");
  JsonObject messageObj = messages.createNestedObject();

  messageObj["type"] = "text";
  messageObj["text"] = message;

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//
void sendMessage(String message) {

  //
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    WiFiClient client;  // WiFiClientのインスタンスを作成
    http.begin(client, "https://api.line.me/v2/bot/message/push");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(channelAccessToken));

    String payload = createJson(message); // JSONペイロードを作成

    // メッセージ送信
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString(); // レスポンスを取得
      Serial.println(httpResponseCode); // レスポンスコードを表示
      Serial.println(response); // レスポンスを表示
    } else {
      Serial.print("Error on sending message: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // 終了処理
  } else {
    Serial.println("WiFi not connected");
  }

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// 10秒ごとに情報を表示する関数
void fetchAndShowTask() {

  int pirState = digitalRead(PIR_PIN);

  //
  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {

    if (pirState == HIGH) {
      sendMessage("人を検知しました！");
    }

    lastTaskMillis = currentMillis;

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
