#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fiの設定
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Webサーバーポート
ESP8266WebServer server(80);

// HX711ピン設定
#define LOAD_CELL_DOUT_PIN  5  // D5
#define LOAD_CELL_SCK_PIN   4  // D4
HX711 scale;

// LCD設定
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2Cアドレスは環境に応じて変更

// 重量ごとの料金を格納する配列
int regularPostCost[6] = {0}; // 定形外規格内料金（50g, 100g, 150g, 250g, 500g, 1kg）
int irregularPostCost[7] = {0}; // 定形外規格外料金（50g, 100g, 150g, 250g, 500g, 1kg, 2kg）

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

  // 郵便料金の取得・パース
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://www.post.japanpost.jp/fee/"); // 郵便料金ページのURL
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      parseHTML(payload); // HTMLを解析して料金を取得
      Serial.println("郵便料金のパースが完了しました。");
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  // Webサーバーの設定
  server.on("/data", handleDataRequest);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

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
  int cost = getCost(weight);
  lcd.setCursor(0, 1);
  lcd.print("Cost: ");
  lcd.print(cost);
  lcd.write(0); // 円を表示

  delay(10000); // 10秒待機
}

// 重量に応じた料金を取得
int getCost(float weight) {
  if (weight <= 50) return regularPostCost[0];
  else if (weight <= 100) return regularPostCost[1];
  else if (weight <= 150) return regularPostCost[2];
  else if (weight <= 250) return regularPostCost[3];
  else if (weight <= 500) return regularPostCost[4];
  else if (weight <= 1000) return regularPostCost[5];
  else return 0; // 規格外対応は省略
}

void parseHTML(const String& html) {
  // 定形外規格内の料金取得
  int startIndex = html.indexOf("定形外郵便物（規格内）");
  if (startIndex != -1) {
    for (int i = 0; i < 6; i++) {
      startIndex = html.indexOf("<td><strong>", startIndex);
      if (startIndex != -1) {
        int endIndex = html.indexOf("</strong>", startIndex);
        String cost = html.substring(startIndex + 12, endIndex);
        regularPostCost[i] = cost.toInt();
        startIndex = endIndex;
        Serial.print("Regular Cost [");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(regularPostCost[i]);
      }
    }
  }

  // 定形外規格外の料金取得
  startIndex = html.indexOf("定形外郵便物（規格外）");
  if (startIndex != -1) {
    for (int i = 0; i < 7; i++) {
      startIndex = html.indexOf("<td><strong>", startIndex);
      if (startIndex != -1) {
        int endIndex = html.indexOf("</strong>", startIndex);
        String cost = html.substring(startIndex + 12, endIndex);
        irregularPostCost[i] = cost.toInt();
        startIndex = endIndex;
        Serial.print("Irregular Cost [");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(irregularPostCost[i]);
      }
    }
  }
}

// JSON形式で重量と料金を返す
void handleDataRequest() {
  float weight = scale.get_units(10);
  int cost = getCost(weight);

  String jsonResponse = "{";
  jsonResponse += "\"weight\": " + String(weight) + ",";
  jsonResponse += "\"cost\": " + String(cost);
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}
