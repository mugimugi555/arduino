// arduino-cli lib install "NTPClient"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名

// NTPクライアント設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000);  // JST (UTC+9)

//
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

}

//
void loop() {

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  // 1秒ごとに時間を表示
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;

    // 現在のエポック時間を取得し、フォーマットして表示
    unsigned long epochTime = timeClient.getEpochTime();
    Serial.println(formatDateTime(epochTime));
  }

  // 1時間に1回NTPで同期
  if (currentMillis % 3600000 == 0) {
    timeClient.update();
    Serial.println("NTP time synced");
  }

}

// 日付と時間をフォーマットする関数
String formatDateTime(unsigned long epochTime) {

  int year = 1970;
  int month, day, hour, minute, second;

  // 日付情報の計算
  unsigned long days = epochTime / 86400L;
  unsigned long remainingSeconds = epochTime % 86400L;

  // 日付を計算（1970年基準）
  while ((days >= (isLeapYear(year) ? 366 : 365))) {
    days -= (isLeapYear(year) ? 366 : 365);
    year++;
  }

  month = 1;
  while (days >= daysInMonth(month, year)) {
    days -= daysInMonth(month, year);
    month++;
  }
  day = days + 1;

  // 時刻情報の計算
  hour = remainingSeconds / 3600;
  remainingSeconds %= 3600;
  minute = remainingSeconds / 60;
  second = remainingSeconds % 60;

  // フォーマットして文字列を返す
  char dateTime[20];
  sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  return String(dateTime);

}

// 閏年かどうかを確認する関数
bool isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 月の日数を取得する関数
int daysInMonth(int month, int year) {
  if (month == 2) return isLeapYear(year) ? 29 : 28;
  if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
  return 31;
}