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
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理を非同期で行い、複数のクライアントからのリクエストに同時に対応できるようにします。
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <LiquidCrystal_I2C.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// 緯度、経度、APIキーを変数として定義
const char* latitude  = "35.6895";  // 東京
const char* longitude = "139.6917"; //
const char* apiKey    = "API_KEY";  //

// LCD1602のアドレスとサイズを指定（例：0x27, 20x4）
LiquidCrystal_I2C lcd(0x27, 20, 4);

// 🌑🌒🌓🌔🌕🌖🌗🌘

// New Moon 🌑
byte newMoon[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00000
};

// Waxing Crescent 🌒
byte waxingCrescent[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b11101,
    0b11100,
    0b01100,
    0b00000
};

// First Quarter 🌓
byte firstQuarter[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b11100,
    0b11100,
    0b01100,
    0b00000
};

// Waxing Gibbous 🌔
byte waxingGibbous[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b11110,
    0b11111,
    0b01110,
    0b00000
};

// Full Moon 🌕
byte fullMoon[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00000
};

// Waning Gibbous 🌖
byte waningGibbous[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b01111,
    0b00111,
    0b00111,
    0b01110,
    0b00000
};

// Last Quarter 🌗
byte lastQuarter[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b01111,
    0b00111,
    0b00111,
    0b01110,
    0b00000
};

// Waning Crescent 🌘
byte waningCrescent[8] = {
    0b00000,
    0b01110,
    0b11111,
    0b01111,
    0b00111,
    0b00011,
    0b01110,
    0b00000
};

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 12 * 60 * 60; // 12時間

// ポート80で非同期Webサーバーを初期化
AsyncWebServer server(80);

//
String jsonResponse;

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  // LCD初期化
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();

  // 起動画面の表示
  showStartup();

  // WiFi接続
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  connectToWiFi();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  // Webサーバーの開始
  setupWebServer();

  // APIリクエストを送信
  sendApiRequest();

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

  // Webサーバーのハンドラを設定
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", jsonResponse); // JSONレスポンスを返す
  });

  // サーバーを開始
  server.begin();

}

// APIリクエストを行い、結果をjsonResponseに格納する関数
String sendApiRequest() {

  if (WiFi.status() == WL_CONNECTED) {

    //
    WiFiClientSecure client;
    client.setInsecure();  // セキュリティ証明書の検証を無効化

    HTTPClient http;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fetch-API:Start...");

    String url = "https://moon-phase.p.rapidapi.com/advanced?lat=" + String(latitude) + "&lon=" + String(longitude);
    http.begin(client, url); // WiFiClientを引数に渡す
    http.addHeader("x-rapidapi-host", "moon-phase.p.rapidapi.com");
    http.addHeader("x-rapidapi-key", apiKey);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {

      jsonResponse = http.getString(); // レスポンスを格納
      Serial.println(jsonResponse);

      // ArduinoJsonを使ってシリアルにJSONを出力
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, jsonResponse);
      //serializeJsonPretty(doc, Serial); // フォーマットされたJSONを出力

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fetch-API:Done OK");

      //
      printLCD();

    } else {

      jsonResponse = "{\"error\":\"Error on HTTP request: " + String(httpResponseCode) + "\"}";
      Serial.println(jsonResponse); // エラーメッセージを出力

    }

    http.end(); // リソースを解放

  } else {

    jsonResponse = "{\"error\":\"WiFi not connected\"}";
    Serial.println(jsonResponse); // WiFi未接続のメッセージを出力

  }

  return jsonResponse;

}

//
void printLCD() {

  lcd.clear();

  // JSONデータを解析
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, jsonResponse);

  // 月相名に基づいてカスタムキャラクターを選択
  String phase_name = doc["moon"]["phase_name"];
  if (phase_name == "New Moon") {
    lcd.createChar(0, newMoon);
  } else if (phase_name == "Waxing Crescent") {
    lcd.createChar(0, waxingCrescent);
  } else if (phase_name == "First Quarter") {
    lcd.createChar(0, firstQuarter);
  } else if (phase_name == "Waxing Gibbous") {
    lcd.createChar(0, waxingGibbous);
  } else if (phase_name == "Full Moon") {
    lcd.createChar(0, fullMoon);
  } else if (phase_name == "Waning Gibbous") {
    lcd.createChar(0, waningGibbous);
  } else if (phase_name == "Last Quarter") {
    lcd.createChar(0, lastQuarter);
  } else if (phase_name == "Waning Crescent") {
    lcd.createChar(0, waningCrescent);
  }

  // カスタムキャラクターをLCDに作成して描画
  lcd.setCursor(0, 0);  // カスタムキャラクターの位置を明示的に設定
  lcd.write(byte(0)); // カスタムキャラクターを表示

  //
  lcd.setCursor(1, 0);
  lcd.print(phase_name);
  Serial.print("MoonParseName:");
  Serial.println(phase_name);

  //
  int age = doc["moon"]["age_days"];
  lcd.setCursor(0, 1);
  lcd.print("Age:");
  lcd.print(age);
  lcd.print("Day");
  Serial.print("MoonAge:");
  Serial.println(age);

  //
  float phase = doc["moon"]["phase"];
  int phasePercentage = static_cast<int>(phase * 100); // 整数値に変換
  lcd.print("(");
  lcd.print(phasePercentage);
  lcd.print("%)");
  Serial.print("MoonParse:");
  Serial.print(phasePercentage);
  Serial.println("%");

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
    Serial.println(sendApiRequest());
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

//----------------------------------------------------------------------------
// アニメーション（テスト）
//----------------------------------------------------------------------------
/*
// 満月のビットパターン
byte fullMoon[8] = {
    0b00000,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b00000
};

// 満月のビットパターンをシフト

for (int i = 0; i < 5; i++) {
  for (int j = 0; j < 8; j++) {
    fullMoon[j] = (fullMoon[j] << 1) | (fullMoon[j] >> 7); // 左シフト
  }
  lcd.createChar(0, fullMoon );
  lcd.setCursor(0, 0);
  lcd.write(byte(0)); // カスタムキャラクターを表示
  delay(1000);                   // 表示を見やすくするための遅延
}

// フェードインエフェクト
void fadeIn() {
  for (int i = 0; i <= 8; i++) {
    lcd.createChar(0, getFadeChar(i));
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    delay(300);
  }
}

// スライドインエフェクト
void slideIn() {
  for (int pos = 16; pos >= 0; pos--) {
    lcd.setCursor(pos, 0);
    lcd.write(byte(0));
    delay(100);
  }
}

// 点滅エフェクト
void blink() {
  for (int i = 0; i < 5; i++) {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    delay(500);
    lcd.setCursor(0, 0);
    lcd.write(' ');  // クリア
    delay(500);
  }
}

// 輪郭描画エフェクト
void drawOutline() {
  byte outline[8] = {
    0b00000000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
  };

  for (int i = 0; i < 5; i++) {
    lcd.createChar(0, outline);
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    delay(500);
  }
}

// パルスエフェクト
void pulse() {
  for (int i = 0; i < 8; i++) {
    lcd.createChar(0, getPulseChar(i));
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    delay(300);
  }
}

// フェードイン用のカスタムキャラクターを取得
byte getFadeChar(int step) {
  byte fadeChar[8] = {0};
  for (int i = 0; i < 8; i++) {
    fadeChar[i] = fullMoon[i] >> step; // シフト演算
  }
  return fadeChar;
}

// パルス用のカスタムキャラクターを取得
byte getPulseChar(int step) {
  byte pulseChar[8] = {0};
  for (int i = 0; i < 8; i++) {
    pulseChar[i] = fullMoon[i] ^ (1 << step); // パルス効果
  }
  return pulseChar;
}

void drawFullMoon() {
    for (int i = 0; i < 4; i++) {
        // 満月の内側からビットを描画
        for (int j = 2; j < 6; j++) {
            fullMoon[j] = fullMoon[j] | (1 << (i + 2)); // 中央から外側にビットを追加
        }

        lcd.createChar(0, fullMoon);
        lcd.setCursor(0, 0);
        lcd.write(byte(0)); // カスタムキャラクターを表示
        delay(500);
    }

    // リセット
    for (int j = 0; j < 8; j++) {
        fullMoon[j] = 0b00000000; // カスタムキャラクターをクリア
    }
}
*/