#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fiの設定
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// HX711ピン設定
#define LOAD_CELL_DOUT_PIN  5  // D5
#define LOAD_CELL_SCK_PIN   4  // D4
HX711 scale;

// LCD設定
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2Cアドレスは環境に応じて変更

// 料金用の変数
int regularPostCost = 0;
int irregularPostCost = 0;
int specialPostCost = 0;

// 「円」文字の配列
const uint8_t yenSymbol[8] = {
  0b00000,
  0b01110,
  0b10001,
  0b10001,
  0b01110,
  0b00100,
  0b01010,
  0b10001
};

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, yenSymbol); // 「円」文字を作成
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  lcd.clear();
  lcd.print("WiFi Connected");

  scale.begin(LOAD_CELL_DOUT_PIN, LOAD_CELL_SCK_PIN);
  delay(2000);
  lcd.clear();
}

void loop() {
  // 重量測定
  float weight = scale.get_units(10); // 10回平均を取る
  Serial.print("Weight: ");
  Serial.println(weight);

  // LCD表示
  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(weight);
  lcd.print(" g");

  // 郵便料金の取得
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://www.post.japanpost.jp/fee/"); // 郵便料金ページのURL
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      parseHTML(payload); // HTMLを解析して料金を取得

      // 料金出力
      if (weight <= 50) {
        Serial.print("定形郵便物の料金: ");
        Serial.println(regularPostCost);
        lcd.setCursor(0, 1);
        lcd.print("Regular: ");
        lcd.print(regularPostCost);
        lcd.write(0); // 円を表示
      }
      if (weight <= 1000) { // 定形外郵便物の条件
        Serial.print("定形外郵便物の料金: ");
        Serial.println(irregularPostCost);
        lcd.setCursor(0, 1);
        lcd.print("Irregular: ");
        lcd.print(irregularPostCost);
        lcd.write(0); // 円を表示
      }
      if (weight <= 4000) { // 提携規格外郵便物の条件
        Serial.print("提携規格外郵便物の料金: ");
        Serial.println(specialPostCost);
        lcd.setCursor(0, 1);
        lcd.print("Special: ");
        lcd.print(specialPostCost);
        lcd.write(0); // 円を表示
      }
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(10000); // 10秒待機
}

void parseHTML(const String& html) {
  // 定形郵便物の料金取得
  int startIndex = html.indexOf("定形郵便物");
  if (startIndex != -1) {
    startIndex = html.indexOf("<td><strong>", startIndex);
    if (startIndex != -1) {
      int endIndex = html.indexOf("</strong>", startIndex);
      String cost = html.substring(startIndex + 12, endIndex); // 110円の部分を抽出
      regularPostCost = cost.toInt();
      Serial.print("Regular Post Cost: ");
      Serial.println(regularPostCost);
    }
  }

  // 定形外郵便物の料金取得
  startIndex = html.indexOf("定形外郵便物");
  if (startIndex != -1) {
    startIndex = html.indexOf("<td><strong>", startIndex);
    while (startIndex != -1) {
      int endIndex = html.indexOf("</strong>", startIndex);
      String cost = html.substring(startIndex + 12, endIndex); // 各料金を抽出
      irregularPostCost += cost.toInt(); // 合計しても良い
      Serial.print("Irregular Post Cost: ");
      Serial.println(cost);
      startIndex = html.indexOf("<td><strong>", endIndex); // 次の料金を探す
    }
  }

  // 提携規格外郵便物の料金取得
  startIndex = html.indexOf("規格外");
  if (startIndex != -1) {
    startIndex = html.indexOf("<td><strong>", startIndex);
    if (startIndex != -1) {
      int endIndex = html.indexOf("</strong>", startIndex);
      String cost = html.substring(startIndex + 12, endIndex); // 例として最初の規格外の料金を抽出
      specialPostCost = cost.toInt();
      Serial.print("Special Post Cost: ");
      Serial.println(specialPostCost);
    }
  }
}
