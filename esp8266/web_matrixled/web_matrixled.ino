#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Max72xxPanel.h>

const char* ssid = "your-SSID"; // Wi-FiのSSID
const char* password = "your-PASSWORD"; // Wi-Fiのパスワード

// MAX7219設定
const int pinCS = D8; // CSピンの指定
Max72xxPanel matrix = Max72xxPanel(pinCS, 4, 1);

AsyncWebServer server(80);

String message = "";      // 表示するメッセージ
int scrollPosition = 0;   // スクロール位置

void setup() {
  Serial.begin(115200);   // シリアル通信の初期化
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi接続中...");
  }
  Serial.println("WiFiに接続しました");

  // MAX7219の初期設定
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

  // Webからのメッセージ受け取り処理
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
  // シリアル通信からのメッセージ受け取り処理
  if (Serial.available() > 0) {
    message = Serial.readStringUntil('\n');  // シリアルからのメッセージを受信
    scrollPosition = matrix.width();         // スクロール位置を初期化
  }

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

