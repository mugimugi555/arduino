/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"       # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266mDNS"       # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "ESPAsyncTCP"       # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ESPAsyncWebServer" # ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理やレスポンスを非同期に行うことができ、複数のクライアントからのリクエストに同時に対応できます。
arduino-cli lib install "ArduinoJson"       # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "WiFiUdp"           # UDP通信を使用するためのWiFiライブラリ
arduino-cli lib install "NTPClient"         # NTP（Network Time Protocol）を使用して、正確な時刻を取得するためのライブラリ
arduino-cli lib install "Time"              # 時間や日付の計算、フォーマットを行うためのライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // 非同期Webサーバーライブラリ。
#include <ESP8266HTTPClient.h> // HTTP通信を行うためのライブラリ。APIからデータを取得したり、サーバーにデータを送信する際に使用します。
#include <WiFiClientSecure.h>  // HTTPS通信を行うためのライブラリ。セキュアな接続を確立し、暗号化されたデータを送受信するために使用します。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Wire.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// HX711ピン設定
#define LOAD_CELL_DOUT_PIN  7  // D5
#define LOAD_CELL_SCK_PIN   6  // D4
HX711 scale;


// 定形外郵便の重量（規格内・規格外）と料金
int regularPostWeight[6]   = {50,100,150,250,500,1000};  // 規格内の重量
int regularPostCost[6]     = {140,180,270,320,510,750};                                // 規格内の料金
int irregularPostWeight[8] = {50,100,150,250,500,1000,2000,4000};  // 規格外の重量
int irregularPostCost[8]   = {260,290,390,450,660,920,1350,1750};

// LCD設定
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2Cアドレスは環境に応じて変更

// ポート80で非同期Webサーバーを初期化
AsyncWebServer server(80);

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 1;

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  //
  scale.begin(LOAD_CELL_DOUT_PIN, LOAD_CELL_SCK_PIN);

  // LCD初期化
  lcd.init();
  lcd.backlight();

  // 起動画面の表示
  showStartup();

  //
  createJson();

  // WiFi接続
  /*
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting...");
  connectToWiFi();
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("WiFi Connected");

  //
  setupWebServer();
  */

  //
  //setupCosts();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

  // ホスト名の更新
  updateMdnsTask();

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

//----------------------------------------------------------------------------
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {

  // ルートへのアクセス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonResponse = createJson();
    request->send(200, "application/json", jsonResponse);
  });

  // Webサーバーを開始
  server.begin();

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

/*
void setupCosts() {

  WiFiClientSecure client;
  client.setInsecure();  // セキュリティ証明書の検証を無効化

  // サーバーに接続
  if (!client.connect("www.post.japanpost.jp", 443)) {
    Serial.println("Connection failed");
    return;
  }

  // リクエストの送信
  client.println("GET /send/fee/kokunai/one_two.html HTTP/1.1");  // 取得したいページのパスを指定
  client.println("Host: www.post.japanpost.jp");
  client.println("Connection: close");
  client.println();

  bool isRegular = false;
  bool isIrregular = false;
  int regularIndex = 0;
  int irregularIndex = 0;
  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');

    // 規格内・規格外のセクション開始を検出
    if (line.indexOf("規格内") != -1) {
      isRegular = true;
      isIrregular = false;
    }
    if (line.indexOf("規格外") != -1) {
      isRegular = false;
      isIrregular = true;
    }

    // 金額の行を抽出
    int yenPos = line.indexOf("円");
    if (yenPos != -1) {
      int strongStart = line.indexOf("<strong>");
      int strongEnd = line.indexOf("</strong>");
      if (strongStart != -1 && strongEnd != -1 && strongEnd > strongStart) {
        String costStr = line.substring(strongStart + 8, strongEnd);
        costStr.replace(",", "");  // カンマを削除
        int cost = costStr.toInt();  // 金額を整数に変換

        // 規格内の料金を格納
        if (isRegular && regularIndex < 6) {
          regularPostCost[regularIndex++] = cost;
        }
        // 規格外の料金を格納
        else if (isIrregular && irregularIndex < 7) {
          irregularPostCost[irregularIndex++] = cost;
        }
      }
    }
  }

  client.stop();
  Serial.println("Connection closed");

  // 配列内容を確認
  Serial.println("定形外規格内料金:");
  for (int i = 0; i < 6; i++) {
    Serial.println(regularPostCost[i]);
  }

  Serial.println("定形外規格外料金:");
  for (int i = 0; i < 7; i++) {
    Serial.println(irregularPostCost[i]);
  }

}
*/

// 重量に応じた料金を取得
int getCostRegular(float weight) {

  if (weight <= 50) return regularPostCost[0];
  else if (weight <=  100) return regularPostCost[1];
  else if (weight <=  150) return regularPostCost[2];
  else if (weight <=  250) return regularPostCost[3];
  else if (weight <=  500) return regularPostCost[4];
  else if (weight <= 1000) return regularPostCost[5];
  else return 0; // 規格外対応は省略

}

// 重量に応じた料金を取得
int getCostIrregular(float weight) {

  if (weight <= 50) return irregularPostCost[0];
  else if (weight <=  100) return irregularPostCost[1];
  else if (weight <=  150) return irregularPostCost[2];
  else if (weight <=  250) return irregularPostCost[3];
  else if (weight <=  500) return irregularPostCost[4];
  else if (weight <= 1000) return irregularPostCost[5];
  else if (weight <= 2000) return irregularPostCost[6];
  else if (weight <= 4000) return irregularPostCost[7];
  else return 0; // 規格外対応は省略

}

// JSON形式で重量と料金を返す
String createJson() {

  // 重量測定
  float weight = scale.get_units(10); // 10回平均を取る
  Serial.print("Weight: ");
  Serial.println(weight);

  // LCD表示
  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(weight);
  lcd.print(" g");

  // 料金表示
  lcd.setCursor(0, 1);
  lcd.print("Cost: ");
  int costRegular = getCostRegular(weight);
  lcd.print(costRegular);
  int costIrregular = getCostIrregular(weight);
  lcd.print(",");
  lcd.print(costIrregular);

  // JSONドキュメントのサイズを指定（必要に応じて増減）
  StaticJsonDocument<128> doc;

  // JSONにデータを追加
  doc["weight"] = weight;
  doc["costRegular"] = costRegular;
  doc["costIrregular"] = costIrregular;

  // JSON文字列にシリアライズ
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  // 結果をシリアルモニタに表示
  Serial.println(jsonResponse);

  return jsonResponse;

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// taskInterval 秒ごとに情報を表示する関数
void fetchAndShowTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;
    Serial.println(createJson());
  }

}

// 0.5秒ごとにホスト名を更新する関数
void updateMdnsTask() {

  static unsigned long lastMdnsMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMdnsMillis >= 500) {
    lastMdnsMillis = currentMillis;
    MDNS.update();
  }

}
