#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

// Webサーバー
ESP8266WebServer server(80);

// LEDピン
const int ledPin = 2;

void setup() {

  // シリアルモニタ初期化
  Serial.begin(115200);

  // LEDピンの初期化
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // 初期状態はオフ

  // WiFi接続
  connectToWiFi();

  // Webサーバーのルートパスにハンドラを設定
  server.on("/"   , handleRoot  );
  server.on("/on" , handleLEDOn );
  server.on("/off", handleLEDOff);
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {

  // クライアントリクエストの処理
  server.handleClient();
  MDNS.update();  // mDNSサービスの更新

}

// WiFi接続
void connectToWiFi() {

  WiFi.hostname(hostname);    // ホスト名を設定
  WiFi.begin(ssid, password); // Wi-Fi接続を開始

  // Wi-Fi接続が完了するまで待つ
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // ESP8266のIPアドレスを表示
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // サブネットマスク、ゲートウェイ、DNS、MACアドレスを表示
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

}

// ルートパスのハンドラ
void handleRoot() {

  // HTMLページを生成
  String html = "<html><body>";
  html += "<h1>ESP32 LED on / off</h1>";
  html += "<p><a href=\"/on\">Turn LED ON</a></p>";
  html += "<p><a href=\"/off\">Turn LED OFF</a></p>";
  html += "</body></html>";

  // レスポンス送信
  server.send(200, "text/html", html);

}

// LED ONハンドラ
void handleLEDOn() {

  digitalWrite(ledPin, HIGH);  // LEDオン
  server.send(200, "text/plain", "LED is ON");

}

// LED OFFハンドラ
void handleLEDOff() {

  digitalWrite(ledPin, LOW);  // LEDオフ
  server.send(200, "text/plain", "LED is OFF");

}
