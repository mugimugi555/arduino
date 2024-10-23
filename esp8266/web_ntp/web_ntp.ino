#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFi接続情報
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

// NTPクライアント設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // タイムゾーン設定（UTC）

unsigned long lastMillis;

void setup() {
  Serial.begin(115200);

  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // NTPクライアント開始
  timeClient.begin();
  timeClient.update();

  lastMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1秒ごとに内部クロックで時間を進める
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    Serial.println(timeClient.getFormattedTime());
  }

  // 1時間に1回NTPで同期
  if (currentMillis % 3600000 == 0) {
    timeClient.update();
    Serial.println("NTP time synced");
  }
}
