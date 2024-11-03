//HD44780ベースのキャラクターLCDを搭載したArduino – Martyn Currey https://www.martyncurrey.com/arduino-with-hd44780-based-lcds/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2Cアドレス0x27、20x4のLCDを初期化

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワード、ホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

void setup() {

  Serial.begin(115200);

  //
  showStartup();

  // WiFi接続
  connectToWiFi();

  lcd.init(); // LCDの初期化
  lcd.backlight(); // 背景ライトをオン
}

void loop() {

  // ハードウェア情報をLCDに表示
  lcd.clear();

  // LCDに情報を表示
  lcd.setCursor(0, 0);
  lcd.print("Board:");
  lcd.print(ARDUINO_BOARD); // ボード名を表示

  lcd.setCursor(0, 1);
  lcd.print("CPUFreq:");
  lcd.print(ESP.getCpuFreqMHz()); // CPUの周波数を表示
  lcd.print("MHz");

  lcd.setCursor(0, 2);
  lcd.print("Flash:");
  lcd.print(ESP.getFlashChipSize() / 1024);
  lcd.print("KB");

  lcd.setCursor(0, 3);
  lcd.print("FreeHeap:");
  lcd.print(ESP.getFreeHeap());
  lcd.print("B");

  delay(6000); // 3秒待機
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SSID:");
  lcd.print(WiFi.SSID());

  lcd.setCursor(0, 1);
  lcd.print("RSSI:");
  lcd.print(WiFi.RSSI());
  lcd.print("dBm");

  lcd.setCursor(0, 2);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());

  lcd.setCursor(0, 3);
  lcd.print("MAC:");
  lcd.print(WiFi.macAddress()); // MACアドレスをシリアルモニタに表示

  delay(6000); // 3秒待機
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("FlashSpeed:");
  lcd.print(ESP.getFlashChipSpeed()); // フラッシュ速度を取得

  lcd.setCursor(0, 1);
  lcd.print("ChipID:");
  lcd.print(ESP.getChipId());

  lcd.setCursor(0, 2);
  lcd.print("SDKVer:");
  lcd.print(ESP.getSdkVersion()); // SDKのバージョンを取得

  delay(6000); // 3秒待機

  // ホスト名の更新
  MDNS.update();

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
