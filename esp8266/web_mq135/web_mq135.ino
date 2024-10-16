#include <ESP8266WiFi.h>       // ESP8266用のWiFiライブラリをインクルード
#include <WiFiClient.h>        // WiFiクライアント用ライブラリをインクルード
#include <ESP8266WebServer.h>  // Webサーバー機能のためのライブラリをインクルード
#include <ESP8266mDNS.h>       // mDNS機能を使用するためのライブラリをインクルード

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;

// センサー値を格納するための変数
int sensorValue;

// ポート80でWebサーバーを作成
ESP8266WebServer server(80);

// LEDの接続ピンを定義
const int led = 13;

// セットアップ関数
void setup(void) {
  
  // LEDピンを出力モードに設定
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // シリアル通信を115200ボーで開始
  Serial.begin(115200);
  
  // Wi-Fi接続
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ESP8266のIPアドレスを表示
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // mDNSサービスを開始
  if (MDNS.begin(hostname)) {
    Serial.println("MDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  
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