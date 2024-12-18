#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Wi-Fi設定
const char* ssid = "your_SSID"; // 自分のWi-Fi SSIDに変更
const char* password = "your_PASSWORD"; // 自分のWi-Fiパスワードに変更

// API URL
const char* apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=35.682839&longitude=139.759455&daily=moon_phase&timezone=Asia/Tokyo";

// タイマー関連
unsigned long previousMillis = 0;
const long interval = 86400000; // 1日（ミリ秒）

// 満月のビットマップデータ（64x64ピクセル）
// https://javl.github.io/image2cpp/
// 64x64のビットマップデータをここに追加します
// 例: 0xFF, 0x00, ... という形式で、ビットマップを作成します。
// 'moon', 64x64px
const unsigned char moonBitmap[] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x05, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xff, 0xff,
	0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7f, 0xff,
	0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfe, 0x04, 0x00, 0x00, 0x00, 0xcf, 0xff,
	0xff, 0xfd, 0xff, 0xe8, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x01, 0xff,
	0xff, 0xff, 0xc7, 0x8f, 0x5f, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00, 0x03, 0xff, 0x80, 0x00, 0xff,
	0xff, 0xfc, 0x07, 0x60, 0x46, 0x80, 0x00, 0x7f, 0xff, 0xfc, 0xff, 0xe0, 0x80, 0x00, 0x00, 0x3f,
	0xff, 0xfe, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x1f,
	0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfc, 0x06, 0x80, 0x00, 0x0f,
	0xff, 0xff, 0xff, 0xfc, 0x03, 0xb0, 0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x31, 0xf8, 0x00, 0x07,
	0xff, 0xff, 0xff, 0xf6, 0x7f, 0xb0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfe, 0xff, 0x80, 0x00, 0x03,
	0xff, 0xff, 0xff, 0xf7, 0xff, 0x80, 0x00, 0x03, 0xfe, 0xff, 0xff, 0xf7, 0xff, 0xe0, 0x00, 0x01,
	0xff, 0xff, 0xff, 0xf9, 0xff, 0xe0, 0x03, 0xc5, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xe0, 0x07, 0xc7,
	0xff, 0xff, 0xff, 0xc3, 0xff, 0xe0, 0x0f, 0xe7, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xe2, 0x0f, 0xe7,
	0xff, 0xff, 0xff, 0xc7, 0xff, 0xef, 0x07, 0xe3, 0xff, 0xfb, 0xff, 0x8f, 0xff, 0xff, 0x07, 0xc3,
	0xff, 0xfe, 0x7f, 0xdf, 0xef, 0xff, 0x07, 0xc1, 0xff, 0xf8, 0x0f, 0xff, 0x7f, 0xff, 0x83, 0x99,
	0xff, 0x78, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x71, 0xff, 0xbc, 0x1f, 0xff, 0xef, 0xff, 0xf0, 0x33,
	0x7f, 0xbd, 0x7f, 0xff, 0x9f, 0xff, 0xfc, 0x3f, 0xff, 0xfd, 0xff, 0x87, 0x8f, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xbf, 0xfe, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xef,
	0xbf, 0xff, 0xbf, 0xf0, 0x07, 0xfb, 0xff, 0xdf, 0xff, 0xff, 0xfe, 0xf0, 0x07, 0xf0, 0x7f, 0xff,
	0xdf, 0xff, 0xfc, 0x20, 0x00, 0x70, 0x7f, 0x93, 0xdf, 0xff, 0xd8, 0x00, 0x00, 0x60, 0x7f, 0x0f,
	0xef, 0xff, 0xd8, 0x00, 0x00, 0xf0, 0x7f, 0x1f, 0xef, 0xff, 0xdc, 0x00, 0x00, 0x60, 0x3f, 0xbb,
	0xe7, 0xff, 0xfe, 0x00, 0x00, 0x20, 0x3f, 0xf7, 0xe7, 0x7f, 0xee, 0x00, 0x00, 0x06, 0x07, 0xff,
	0xf0, 0xff, 0xff, 0x00, 0x00, 0x0e, 0x0e, 0xff, 0xf0, 0xff, 0xff, 0x00, 0x00, 0x1e, 0x5c, 0xff,
	0xf8, 0xf7, 0xfe, 0x00, 0x00, 0x0e, 0x38, 0xff, 0xfc, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f,
	0xfc, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff,
	0xff, 0x1b, 0xbf, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x83, 0x84, 0x00, 0x00, 0x00, 0x03, 0xff,
	0xff, 0xf0, 0x80, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff,
	0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff,
	0xff, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xff,
	0xff, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 528)
const int epd_bitmap_allArray_LEN = 1;
const unsigned char* epd_bitmap_allArray[1] = {
	epd_bitmap_moon
};


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1日経過したらAPIにアクセス
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // タイマーをリセット
    getMoonPhase();
  }
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

void getMoonPhase() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      displayMoonPhase(payload); // 月の状態を表示
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void displayMoonPhase(String json) {
  // JSONデータを解析
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonObject obj = doc["daily"];

  // 月の状態を取得
  float moonPhase = obj["moon_phase"][0]; // 最新の月相データを取得（0.0〜1.0の範囲）

  // OLEDに描画
  display.clearDisplay();

  // 月のビットマップを描画
  display.drawBitmap(32, 0, moonBitmap, 64, 64, WHITE);

  // マスク処理を適用する
  int maskWidth = 64 * (1.0 - moonPhase); // 満ち欠けの割合に基づいて幅を計算

  // 欠けた部分を黒で塗りつぶす
  display.fillRect(32 + maskWidth, 0, 64 - maskWidth, 64, BLACK);

  // 月相の表示
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Moon Phase: "); display.print(moonPhase * 100); display.println("%");

  display.display();
}
