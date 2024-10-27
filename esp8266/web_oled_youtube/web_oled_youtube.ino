/*
arduino-cli lib install "ESP8266WiFi"
arduino-cli lib install "ESP8266HTTPClient"
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "Adafruit GFX"
arduino-cli lib install "Adafruit SSD1306"
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h> // SPIライブラリのインクルード
#include <Wire.h> // I2C通信のためのライブラリ

/*
OLED Pin	ESP8266 Pin
VCC	3.3V
GND	GND
SCL	D1 (GPIO 5)
SDA	D2 (GPIO 4)
*/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Your_SSID";                 // Wi-FiのSSID
const char* password = "Your_PASSWORD";         // Wi-Fiのパスワード
const char* jsonUrl = "http://yourserver.com/path/to/frames_info.json"; // JSONファイルのURL
const char* imageUrlBase = "http://yourserver.com/path/to/frames/"; // 画像のベースURL

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  loadImageCount();
}

void loop() {
  // ここにループする処理を追加することができます
}

void loadImageCount() {
  HTTPClient http;
  http.begin(jsonUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    int imageCount = doc["image_count"];
    Serial.printf("Number of images: %d\n", imageCount);

    // 画像の処理
    for (int i = 0; i < imageCount; i++) {
      String fileName = doc["image_files"][i].as<String>();
      Serial.println(fileName);
      displayImage(fileName);
      delay(2000); // 画像を表示する時間（2秒）
    }
  } else {
    Serial.printf("Failed to load JSON, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void displayImage(String fileName) {
  // 画像のURLを作成
  String imageUrl = String(imageUrlBase) + fileName;

  // 画像を取得
  HTTPClient http;
  http.begin(imageUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    display.clearDisplay(); // 画面をクリア
    display.drawBitmap(0, 0, stream, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // BMP画像を表示
    display.display(); // 描画を更新
  } else {
    Serial.printf("Failed to load image, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
