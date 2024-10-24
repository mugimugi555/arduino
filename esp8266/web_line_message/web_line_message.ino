#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266HTTPClient.h> // HTTPクライアントライブラリ
#include <ArduinoJson.h>       // JSONライブラリ

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名

// https://developers.line.biz/console/
const char* channelAccessToken = "CHANNELACCESSTOKEN"; // LINE Develop -> プロバイダーチャネル -> Messaging API設定 -> チャネルアクセストークン（長期）
const char* providerUserId     = "PROVIERUSERID"     ; // LINE Develop -> プロバイダーチャネル -> チャネル基本設定 -> ユーザーID

// PIRセンサー         ESP8266
// VCC  <---------->  5V (または3.3V)
// GND  <---------->  GND
// OUT  <---------->  D5 (GPIO 14)

#define PIR_PIN 5  // PIRセンサーのピン

//
void setup() {

  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  connectToWiFi();  // Wi-Fiに接続する関数を呼び出す

}

//
void loop() {

  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    sendMessage("人を検知しました！");
    delay(10000);  // 10秒間は再送信しない
  }

}

//
void connectToWiFi() {

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

}

//
String createJsonPayload(String message) {

  // JSONオブジェクトの作成
  StaticJsonDocument<200> doc; // ドキュメントのサイズは適宜変更
  doc["to"] = providerUserId;
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject messageObj = messages.createNestedObject();
  messageObj["type"] = "text";
  messageObj["text"] = message;

  // JSON文字列にシリアライズ
  String payload;
  serializeJson(doc, payload);

  return payload; // JSONペイロードを返す

}

void sendMessage(String message) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    WiFiClient client;  // WiFiClientのインスタンスを作成
    http.begin(client, "https://api.line.me/v2/bot/message/push");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(channelAccessToken));

    String payload = createJsonPayload(message); // JSONペイロードを作成

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
