#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <WiFiClient.h>        // WiFiクライアント用ライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード

// WiFi SSIDとパスワードを定義。これらが未定義の場合、デフォルト値を設定。
const char* ssid = "WIFISSID";
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME";  // ホスト名を指定

int sensorValue; // センサー値を格納するための変数

ESP8266WebServer server(80); // ポート80でWebサーバーを作成

const int led = 13; // LEDの接続ピンを定義

// ルートURLにアクセスした際の処理
void handleRoot() {
  digitalWrite(led, 1); // LEDをONにする

  sensorValue = analogRead(0); // アナログピン0からセンサー値を読み取る
  Serial.print("airquality = "); // シリアルモニタにメッセージを表示
  Serial.print(sensorValue, DEC); // 読み取ったセンサー値を表示
  Serial.println(" PPM"); // 単位を表示

  String message = ""; // 返送するメッセージを初期化
  
  message += "{"; // JSON形式でメッセージを開始
  
  message += "\"airquality\":{\"product\":\"mq135\",\"value\":"; // センサー情報を追加
  message += analogRead(0); // センサーの値を追加
  message += ",\"unit\":\"ppm\"}"; // 単位を追加

  message += "}"; // JSON形式でメッセージを終了
  
  server.send(200, "application/json", message ); // HTTPレスポンスを送信
  digitalWrite(led, 0); // LEDをOFFにする
}

// 存在しないURLにアクセスした際の処理
void handleNotFound() {
  digitalWrite(led, 1); // LEDをONにする
  String message = "File Not Found\n\n"; // エラーメッセージを初期化
  message += "URI: "; // リクエストURIを追加
  message += server.uri(); // URIを追加
  message += "\nMethod: "; // HTTPメソッドを追加
  message += (server.method() == HTTP_GET) ? "GET" : "POST"; // メソッドを表示
  message += "\nArguments: "; // 引数を追加
  message += server.args(); // 引数の数を追加
  message += "\n"; // 改行

  // 引数の詳細を追加
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; // 引数名と値を表示
  }
  server.send(404, "text/plain", message); // 404エラーレスポンスを送信
  digitalWrite(led, 0); // LEDをOFFにする
}

// セットアップ関数
void setup(void) {
  
  pinMode(led, OUTPUT); // LEDピンを出力モードに設定
  digitalWrite(led, 0); // LEDをOFFにする

  Serial.begin(115200); // シリアル通信を115200ボーで開始

  WiFi.mode(WIFI_STA); // WiFiモードをステーションに設定
  WiFi.begin(ssid, password); // WiFi接続を開始
  Serial.println("");

  // WiFi接続待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // 500ms待機
    Serial.print("."); // 接続中のドットを表示
  }
  Serial.println("");
  Serial.print("Connected to "); // 接続完了メッセージを表示
  Serial.println(ssid); // 接続したSSIDを表示
  Serial.print("IP address: "); // IPアドレスを表示するメッセージ
  Serial.println(WiFi.localIP()); // ESP8266のIPアドレスを表示

  // mDNSを開始
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started"); // mDNS開始メッセージ
  }

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);

  // 存在しないURLへのハンドラを設定
  server.onNotFound(handleNotFound);

  server.begin(); // Webサーバーの開始
  Serial.println("HTTP server started"); // サーバー開始メッセージ
}

// メインループ
void loop(void) {
  server.handleClient(); // クライアントからのリクエストを処理
  MDNS.update(); // mDNSサービスの更新
}
