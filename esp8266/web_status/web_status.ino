/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"             # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"             # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncTCP"             # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ESPAsyncWebServer"       # ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理やレスポンスを非同期に行うことができ、複数のクライアントからのリクエストに同時に対応できます。
arduino-cli lib install "ArduinoJson"             # JSON形式のデータを簡単に作成、解析するためのライブラリ。API通信やデータの保存に役立ち、シリアライズやデシリアライズが簡単に行えます。

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_bme280/web_bme280.ino wifissid wifipasswd hostname

*****************************************************************************/

#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。
#include <ESPAsyncWebServer.h> // 非同期Webサーバーライブラリ。
#include <ArduinoJson.h>       // ArduinoJsonライブラリ。
#include <Wire.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

//
const long taskInterval = 1; // タスクを繰り返し実行する間隔（秒）
unsigned long previousMillis = 0; // 最後の更新時間を記録

AsyncWebServer server(80); // ポート80で非同期Webサーバーを初期化

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);

  // 起動画面の表示
  showSplash();

  // WiFi接続
  connectToWiFi();

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  unsigned long currentMillis = millis();

  // タスクを毎秒実行
  if (currentMillis - previousMillis >= taskInterval * 1000 ) {
    previousMillis = currentMillis;
    Serial.println(createJson());
  }

  // ホスト名の更新
  MDNS.update();

}

//----------------------------------------------------------------------------
// 起動画面の表示
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
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {

  // ルーtへのアクセス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = createJson();
    request->send(200, "application/json", jsonResponse);
  });

  // Webサーバーを開始
  server.begin();

}

//----------------------------------------------------------------------------
// 取得されるデータをJSON形式で生成
//----------------------------------------------------------------------------
String createJson() {

  StaticJsonDocument<512> doc; // JSONドキュメントのサイズを設定

  // Wi-Fi情報を追加
  doc["SSID"]                = WiFi.SSID();
  doc["RSSI (dBm)"]          = WiFi.RSSI();
  doc["IP Address"]          = WiFi.localIP().toString();
  doc["MAC Address"]         = WiFi.macAddress();
  doc["Board"]               = ARDUINO_BOARD;                 // ボード名を表示
  doc["CPU Frequency (MHz)"] = ESP.getCpuFreqMHz();           // CPUの周波数を表示
  doc["Flash Size (KB)"]     = ESP.getFlashChipSize() / 1024; // フラッシュサイズを表示
  doc["Free Heap (B)"]       = ESP.getFreeHeap();             // 空きヒープメモリを表示
  doc["Flash Speed (MHz)"]   = ESP.getFlashChipSpeed();       // フラッシュ速度を取得
  doc["Chip ID"]             = ESP.getChipId();               // チップIDを取得
  doc["SDK Version"]         = ESP.getSdkVersion();           // SDKバージョンを取得
  doc["status"]              = 1;                             // ステータス (正常の場合は1)
  doc["message"]             = "正常に取得できました。";          // メッセージ (データ取得が成功したことを示す)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}