#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード
#include <WiFiClient.h>        // WiFiクライアント用ライブラリをインクルード

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名

// センサー値を格納するための変数
int sensorValue;

// ポート80でWebサーバーを作成
ESP8266WebServer server(80);

// LEDの接続ピンを定義
const int led = 13;

// セットアップ関数
void setup(void) {
  
  // シリアル通信を115200ボーで開始
  Serial.begin(115200);
  
  // LEDピンを出力モードに設定
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  //
  connectToWiFi();
  
  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);

  // 存在しないURLへのハンドラを設定
  server.onNotFound(handleNotFound);

  // Webサーバーの開始
  server.begin();

  // サーバー開始メッセージ
  Serial.println("HTTP server started");

}

// メインループ
void loop(void) {
  server.handleClient(); // クライアントからのリクエストを処理
  MDNS.update(); // mDNSサービスの更新
}

//
void connectToWiFi() {

  // Connect to WiFi
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // Display the esp8266's hostname
  Serial.print("Hostname: http://");
  Serial.print(WiFi.getHostname());
  Serial.println(".local");

  // Display the esp8266's IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display subnet mask
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  // Display gateway IP
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());

  // Display DNS server IP
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  // Display the esp8266's MAC address
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Start mDNS responder
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

}

// ルートURLにアクセスした際の処理
void handleRoot() {

  //
  digitalWrite(led, 1); // LEDをONにする

  // アナログピン0からセンサー値を読み取る
  sensorValue = analogRead(0);
  Serial.print("airquality = ");
  Serial.print(sensorValue, DEC);
  Serial.println(" PPM");

  // JSON形式でメッセージを開始
  String message = "";
  message += '{';
  message += '"airquality":{"product":"mq135","value":';
  message += analogRead(0);
  message += ',"unit":"ppm"}';
  message += '}';

  // HTTPレスポンスを送信
  server.send(200, "application/json", message );

  // LEDをOFFにする
  digitalWrite(led, 0);

}

// 存在しないURLにアクセスした際の処理
void handleNotFound() {

  // LEDをONにする
  digitalWrite(led, 1);

  // エラーメッセージ
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  // 引数の詳細を追加
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  // 404エラーレスポンスを送信
  server.send(404, "text/plain", message);

  // LEDをOFFにする
  digitalWrite(led, 0);

}
