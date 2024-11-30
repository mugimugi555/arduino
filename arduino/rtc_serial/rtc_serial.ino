#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);  // シリアル通信開始
  Wire.begin();        // RTC通信開始

  if (!rtc.begin()) {
    Serial.println("RTC接続エラー");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTCの電力が切れていました。時刻を再設定してください。");
  }

  Serial.println("PCから時刻を送信してください (形式: yyyy-MM-dd HH:mm:ss)");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.length() > 0) {
      setRTC(input);
    }
  }
}

void setRTC(String datetime) {
  int year = datetime.substring(0, 4).toInt();
  int month = datetime.substring(5, 7).toInt();
  int day = datetime.substring(8, 10).toInt();
  int hour = datetime.substring(11, 13).toInt();
  int minute = datetime.substring(14, 16).toInt();
  int second = datetime.substring(17, 19).toInt();

  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("時刻を設定しました:");
  Serial.println(datetime);
}
