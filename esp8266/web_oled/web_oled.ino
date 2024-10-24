#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "your-SSID";       // WiFi SSID
const char* password = "your-PASSWORD"; // WiFiパスワード
const char* apiKey = "your-API-key"; // OpenWeatherMapのAPIキー
const char* city = "Tokyo";          // 都市名

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // WiFi接続
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // OLEDディスプレイの初期化
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + apiKey + "&units=metric";

    http.begin(weatherUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);

      // JSONパース
      StaticJsonDocument<512> doc;
      deserializeJson(doc, payload);

      String weather = doc["weather"][0]["main"];
      float temp = doc["main"]["temp"];

      // OLEDに表示
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.println("Weather: " + weather);
      display.println("Temp: " + String(temp) + "C");
      display.display();
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
  delay(60000); // 1分ごとに更新
}