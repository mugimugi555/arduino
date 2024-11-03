/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"        # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncTCP"        # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "DHT sensor library" # DHT11やDHT22温湿度センサー用のライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <WiFiClient.h>
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <LiquidCrystal_I2C.h> // I2C接続のLCDディスプレイを制御するためのライブラリ。センサーデータやステータス情報を表示する際に使用します。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える

//
// [ESP8266] <---> [LCD]
// 3.3V <--------> VCC
// GND <---------> GND
// GPIO 4 (D2) <-> SDA
// GPIO 5 (D1) <-> SCL

// LCD初期化（I2Cアドレスは通常0x27または0x3F）
LiquidCrystal_I2C lcd(0x27, 16, 2);

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 24 * 60 * 60; // １日

// Open-Meteo APIのURL
const char* apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=35.682839&longitude=139.759455&daily=moon_phase&timezone=Asia/Tokyo";


// カスタムキャラクターのデータ
byte newMoon[8]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 新月
byte crescentMoon[8] = {0x00, 0x04, 0x0C, 0x1C, 0x1C, 0x0C, 0x04, 0x00}; // 三日月
byte firstQuarter[8] = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}; // 上弦の月
byte fullMoon[8]     = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 満月
byte lastQuarter[8]  = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}; // 下弦の月
byte waningGibbous[8]= {0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F}; // 欠け始めた満月
byte waxingGibbous[8]= {0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F}; // 満月に近い状態
byte waningCrescent[8] = {0x00, 0x04, 0x06, 0x07, 0x07, 0x06, 0x04, 0x00}; // 欠ける三日月

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  //
  setupLCD();

  // 起動画面の表示
  showStartupScreen();

  // WiFi接続
  connectToWiFi();

  // 月相情報を取得して表示
  fetchMoonPhase();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowDataTask();

}

void setupLCD() {

  lcd.init(); // LCD初期化
  lcd.backlight(); // バックライトをオンに

  // カスタムキャラクターの登録
  lcd.createChar(0, newMoon);
  lcd.createChar(1, crescentMoon);
  lcd.createChar(2, firstQuarter);
  lcd.createChar(3, fullMoon);
  lcd.createChar(4, lastQuarter);
  lcd.createChar(5, waningGibbous);
  lcd.createChar(6, waxingGibbous);
  lcd.createChar(7, waningCrescent);

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

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("WiFi Connected");


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
// 月の取得と表示
//----------------------------------------------------------------------------
void fetchMoonPhase() {

  WiFiClient client;

  if (client.connect("api.open-meteo.com", 443)) {

    // APIリクエストの送信
    client.print(
      String("GET ") + apiUrl +
      " HTTP/1.1\r\n" +
      "Host: api.open-meteo.com\r\n" +
      "Connection: close\r\n\r\n"
    );

    // レスポンスの読み込み
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();
    Serial.println(response);

    // JSON解析
    DynamicJsonDocument docApi(1024);
    deserializeJson(docApi, response);

    // 月相情報の取得
    const char* phase = docApi["daily"]["moon_phase"][0]; // 現在の月相
    int age = docApi["daily"]["moon_phase"][1]; // 月齢（数値での計算を要する）

    /*
    switch (phase) {
      case 0:
        lcd.write(byte(0)); // 新月
        lcd.print(" New Moon");
        break;
      case 1:
        lcd.write(byte(1)); // 三日月
        lcd.print(" Crescent");
        break;
      case 2:
        lcd.write(byte(2)); // 上弦の月
        lcd.print(" First Qtr");
        break;
      case 3:
        lcd.write(byte(3)); // 満月
        lcd.print(" Full Moon");
        break;
      case 4:
        lcd.write(byte(4)); // 下弦の月
        lcd.print(" Last Qtr");
        break;
      case 5:
        lcd.write(byte(5)); // 欠け始めた満月
        lcd.print(" Waning Gib");
        break;
      case 6:
        lcd.write(byte(6)); // 満月に近い状態
        lcd.print(" Waxing Gib");
        break;
      case 7:
        lcd.write(byte(7)); // 欠ける三日月
        lcd.print(" Waning Cres");
        break;
      default:
        lcd.print("Unknown Phase");
        break;
    }
    */

    // 月相のパーセント計算
    int percentage = (age / 29.53) * 100; // 月の満ち欠け周期は約29.53日
    int daysUntilFullMoon = 14 - age; // 満月までの日数計算

    // LCDに表示
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("phase:");
    lcd.print(phase);
    lcd.setCursor(0, 1);
    lcd.print("age:");
    lcd.print(daysUntilFullMoon);
    lcd.print("day");

    // プログレスバーの表示
    lcd.setCursor(0, 1);
    lcd.print("at");
    int barLength = map(percentage, 0, 100, 0, 16); // 16文字分のバー
    for (int i = 0; i < 16; i++) {
      if (i < barLength) {
        lcd.write(0xFF); // プログレスバーを表示（フルブロック）
      } else {
        lcd.print(" "); // 空白
      }
    }

    // パーセンテージを表示
    lcd.setCursor(0, 1);
    lcd.print(percentage);
    lcd.print("%");

    StaticJsonDocument<256> doc;
    doc["moon_phase"] = phase;
    doc["moon_age"] = age;
    doc["days_until_full_moon"] = daysUntilFullMoon;
    doc["moon_visibility_percentage"] = percentage;

    // JSONデータを文字列にシリアライズ
    String json;
    serializeJson(doc, json);
    Serial.println(json);

  } else {

    Serial.println("Connection failed");

  }

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
    fetchMoonPhase();
  }

}