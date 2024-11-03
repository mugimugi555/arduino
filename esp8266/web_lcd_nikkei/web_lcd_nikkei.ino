/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESPAsyncTCP"        # ESP8266用の非同期TCP通信を提供するライブラリ。非同期で複数のクライアントと通信できるようにし、WebSocketやHTTPサーバーのバックエンドで使用されます。
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを簡単に作成、解析するためのライブラリ。API通信やセンサーデータの整理、保存に役立ちます。
arduino-cli lib install "LiquidCrystal I2C"  # I2C接続のLCDディスプレイを制御するためのライブラリ。データやメッセージをディスプレイ上に表示するのに使用します。

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266HTTPClient.h> // HTTP通信を行うためのライブラリ。APIからデータを取得したり、サーバーにデータを送信する際に使用します。
#include <WiFiClientSecure.h>  // HTTPS通信を行うためのライブラリ。セキュアな接続を確立し、暗号化されたデータを送受信するために使用します。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <LiquidCrystal_I2C.h> // I2C接続のLCDディスプレイを制御するためのライブラリ。センサーデータやステータス情報を表示する際に使用します。

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワード、ホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える

//
const char* tickerNikkei   = "^N225";
const char* tickerExchange = "USDJPY=X";

//
// [ESP8266] <---> [LCD]
// 3.3V <--------> VCC
// GND <---------> GND
// GPIO 4 (D2) <-> SDA
// GPIO 5 (D1) <-> SCL

// LCD初期化（I2Cアドレスは通常0x27または0x3F）
LiquidCrystal_I2C lcd(0x27, 16, 2);

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 1 * 60 * 60; //１時間ごとに更新

//
float nikkeiAverage = 0;
float exchangeRate  = 0;

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  lcd.init();
  lcd.begin(16, 2); // 16カラム、2行のLCDディスプレイの場合
  lcd.backlight();

  // 起動画面の表示
  showStartupScreen();

  // WiFi接続
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting...");
  connectToWiFi();
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("WiFi Connected");

  //
  fetchAndDisplayData();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowDataTask();

}

//----------------------------------------------------------------------------
// 起動画面の表示
//----------------------------------------------------------------------------
void showStartupScreen() {

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

  WiFi.begin(ssid, password);

  Serial.print("Connected to ");
  Serial.println(ssid);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("===============================================");
  Serial.println("              Network Details                  ");
  Serial.println("===============================================");
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
// 株価の取得関数
//----------------------------------------------------------------------------

// api から取得
float fetchPrice(String ticker) {

  float price = 0;

  //
  Serial.print("Start fetch ");
  Serial.print(ticker);
  Serial.print(" => ");

  //
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start fetch ");
  lcd.setCursor(0, 1);
  lcd.print(ticker);

  //
  WiFiClientSecure client;
  client.setInsecure();  // セキュリティ証明書の検証を無効化

  // サーバーに接続
  if (!client.connect("query1.finance.yahoo.com", 443)) {
    Serial.println("Connection failed");
    return 0;
  }

  // リクエスト送信
  // https://query1.finance.yahoo.com/v8/finance/chart/^N225?interval=1d
  client.print("GET /v8/finance/chart/");
  client.print( urlEncode( ticker ) );
  client.println("?interval=1d HTTP/1.1");
  client.println("Host: query1.finance.yahoo.com");
  client.println("Accept: application/json");
  client.println("Accept-Language: ja;q=0.7");
  client.println("Cache-Control: max-age=0");
  client.println("Priority: u=0, i");
  client.println("Sec-CH-UA-Mobile: ?0");
  client.println("Sec-CH-UA-Platform: \"Linux\"");
  client.println("Sec-Fetch-Dest: document");
  client.println("Sec-Fetch-Mode: navigate");
  client.println("Sec-Fetch-Site: none");
  client.println("Sec-Fetch-User: ?1");
  client.println("Sec-GPC: 1");
  client.println("Upgrade-Insecure-Requests: 1");
  client.println("User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36");
  client.println();

  // サーバーからのレスポンスを読み込む
  String searchStr = "regularMarketPrice\":";
  while (client.connected() || client.available()) {

    // データが利用可能な場合は読み取る
    if (client.available()) {

      String line = client.readStringUntil('\n');  // 一行ずつ読み込む

      // 探したい文字列の開始位置を取得
      int startIndex = line.indexOf(searchStr);
      if (startIndex != -1) {

        // 開始位置に探した文字列の長さを加算して、値の開始位置を取得
        startIndex += searchStr.length();

        // カンマの位置を探して、値の終了位置を取得
        int endIndex = line.indexOf(",", startIndex);

        // 値部分を切り出して浮動小数点数に変換
        String valueStr = line.substring(startIndex, endIndex);
        price = valueStr.toFloat();
        break;

      }

    }

  }

  client.stop();

  Serial.println(price);

  return price;

}

// URLエンコード関数
String urlEncode(String str) {

  String encodedString = "";

  char c;
  char buf[3];
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);

    // URLで使用できない文字をエンコードする
    if (isalnum(c)) {
      encodedString += c;  // 英数字はそのまま
    } else {
      sprintf(buf, "%%%02X", c);  // %XXの形式でエンコード
      encodedString += buf;
    }

  }

  return encodedString;

}

//----------------------------------------------------------------------------
// LCD表示
//----------------------------------------------------------------------------

//
void displayData(float nikkei, float exchange) {

  //
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nikkei:");
  lcd.print(nikkei);

  //
  lcd.setCursor(0, 1);
  lcd.print("USDJPY:");
  lcd.print(exchange);

}

//
void fetchAndDisplayData() {

  //
  nikkeiAverage = fetchPrice(tickerNikkei);
  exchangeRate  = fetchPrice(tickerExchange);
  displayData(nikkeiAverage, exchangeRate);

  //
  Serial.println(createJson());

}

//----------------------------------------------------------------------------
// 取得されるデータをJSON形式で生成
//----------------------------------------------------------------------------
String createJson() {

  //
  StaticJsonDocument<200> doc;

  //
  doc["nikkei"]    = nikkeiAverage;
  doc["usd_jpy"]   = exchangeRate;

  //
  doc["status"]    = 1;                         // ステータス (正常の場合は1)
  doc["message"]   = "正常に取得できました。";      // メッセージ (データ取得が成功したことを示す)
  doc["ipaddress"] = WiFi.localIP().toString(); // IPアドレス (デバイスのネットワークアドレス)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// 1秒ごとに情報を表示する関数
void fetchAndShowDataTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;
    fetchAndDisplayData();
  }

}
