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

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  //
  showSplash();

  // WiFi接続
  connectToWiFi();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  //
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    sendMessage("人を検知しました！");
    delay(10000);  // 10秒間は再送信しない
  }

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

//
String createJson(String message) {

  // JSONオブジェクトの作成
  StaticJsonDocument<200> doc; // ドキュメントのサイズは適宜変更
  doc["to"] = providerUserId;
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject messageObj = messages.createNestedObject();
  messageObj["type"] = "text";
  messageObj["text"] = message;

  // JSON文字列にシリアライズ
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
