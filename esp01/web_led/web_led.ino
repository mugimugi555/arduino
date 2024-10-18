#include <WiFi.h>
#include <WebServer.h>

// WiFi設定
const char* ssid = "WIFISSID";
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME";  // ESP32のホスト名

// Webサーバー
WebServer server(80);

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
  server.on("/", handleRoot);
  server.on("/on", handleLEDOn);
  server.on("/off", handleLEDOff);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // クライアントリクエストの処理
  server.handleClient();
}

// WiFi接続
void connectToWiFi() {

  WiFi.begin(ssid, password);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

// ルートパスのハンドラ
void handleRoot() {

  // HTMLページを生成
  String html = "<html><body>";
  html += "<h1>ESP32 DHT Sensor Data</h1>";
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
