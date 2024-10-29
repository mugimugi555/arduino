#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Max72xxPanel.h>

const char* ssid = "your-SSID"; // Wi-FiのSSID
const char* password = "your-PASSWORD"; // Wi-Fiのパスワード

// MAX7219設定
const int pinCS = D8; // CSピンの指定
Max72xxPanel matrix = Max72xxPanel(pinCS, 4, 1);

AsyncWebServer server(80);

String message = ""; // 表示するメッセージ
int scrollPosition = 0; // スクロール位置

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi接続中...");
  }
  Serial.println("WiFiに接続しました");

  // MAX7219設定
  matrix.setIntensity(3); // LEDの明るさ
  matrix.setRotation(0, 1); // LEDマトリックスの回転設定（必要に応じて調整）

  // Webサーバーの設定
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>"
                  "<form action=\"/message\" method=\"post\">"
                  "<label>表示するメッセージ:</label>"
                  "<input type=\"text\" name=\"text\"/>"
                  "<input type=\"submit\" value=\"送信\"/>"
                  "</form></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/message", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text", true)) {
      message = request->getParam("text", true)->value();
      scrollPosition = matrix.width();
      request->send(200, "text/html", "<html><body><h2>メッセージが送信されました</h2><a href=\"/\">戻る</a></body></html>");
    } else {
      request->send(400, "text/html", "メッセージがありません");
    }
  });

  server.begin();
  Serial.println("Webサーバーが開始されました");
}

void loop() {
  // スクロール処理
  matrix.fillScreen(LOW);
  matrix.setCursor(scrollPosition, 0);
  matrix.print(message);

  matrix.write();

  // 文字が収まりきらない場合にスクロール
  if (--scrollPosition < -((int)message.length() * 8)) {
    scrollPosition = matrix.width();
  }

  delay(100); // スクロール速度を調整
}
