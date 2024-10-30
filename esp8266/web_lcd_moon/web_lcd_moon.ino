#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "YOUR_SSID"; // Wi-FiのSSID
const char* password = "YOUR_PASSWORD"; // Wi-Fiのパスワード

// Open-Meteo APIのURL
const char* apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=35.682839&longitude=139.759455&daily=moon_phase&timezone=Asia/Tokyo";

// LCD初期化（I2Cアドレスは通常0x27または0x3F）
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  lcd.init(); // LCD初期化
  lcd.backlight(); // バックライトをオンに
  delay(10);

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // 月相情報を取得して表示
  fetchMoonPhase();
}

void loop() {
  // 1日ごとに月相を取得
  delay(86400000); // 86400000ミリ秒 = 1日
  fetchMoonPhase();
}

void fetchMoonPhase() {
  WiFiClient client;
  if (client.connect("api.open-meteo.com", 443)) {
    // APIリクエストの送信
    client.print(String("GET ") + apiUrl + " HTTP/1.1\r\n" +
                 "Host: api.open-meteo.com\r\n" +
                 "Connection: close\r\n\r\n");

    // レスポンスの読み込み
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readString();
      }
    }
    client.stop();

    // JSON解析
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);

    // 月相情報の取得
    const char* phase = doc["daily"]["moon_phase"][0]; // 現在の月相
    int age = doc["daily"]["moon_phase"][1]; // 月齢（数値での計算を要する）

    // 月相のパーセント計算
    int percentage = (age / 29.53) * 100; // 月の満ち欠け周期は約29.53日
    int daysUntilFullMoon = 14 - age; // 満月までの日数計算

    // LCDに表示
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("月相: ");
    lcd.print(phase);
    lcd.setCursor(0, 1);
    lcd.print("満月まで: ");
    lcd.print(daysUntilFullMoon);
    lcd.print("日 ");

    // プログレスバーの表示
    lcd.setCursor(0, 1);
    lcd.print("進捗: ");
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

    // シリアルモニタにも表示
    Serial.print("月相: ");
    Serial.println(phase);
    Serial.print("月齢: ");
    Serial.println(age);
    Serial.print("満月までの日数: ");
    Serial.println(daysUntilFullMoon);
    Serial.print("月の状態: ");
    Serial.print(percentage);
    Serial.println("%");
  } else {
    Serial.println("Connection failed");
  }
}
