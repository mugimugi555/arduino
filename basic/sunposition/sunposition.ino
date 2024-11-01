#include <TinyGPS++.h>
#include <Wire.h>
#include <Servo.h>
#include <SunPosition.h>
#include <HMC5883L.h> // 方位磁石センサー用ライブラリ
#include <ArduinoJson.h> // JSON形式で出力するためのライブラリ

/*
接続方法

GPSセンサー (例: NEO-6M)
VCC: Arduinoの5Vピンに接続
GND: ArduinoのGNDピンに接続
TX: Arduinoのデジタルピン（例: 4番ピン）に接続
RX: Arduinoのデジタルピン（例: 3番ピン）に接続（必要に応じて）

方位磁石センサー (例: HMC5883L)
VCC: Arduinoの5Vピンに接続
GND: ArduinoのGNDピンに接続
SDA: ArduinoのA4ピン（I2C SDA）に接続
SCL: ArduinoのA5ピン（I2C SCL）に接続

サーボモーター
VCC: Arduinoの5Vピンに接続
GND: ArduinoのGNDピンに接続
信号ピン: Arduinoのデジタルピン（例: 9番ピン）に接続
*/

TinyGPSPlus gps;
Servo myServo;
HMC5883L compass; // 方位磁石センサーのインスタンス

void setup() {
  Serial.begin(9600);
  myServo.attach(9);
  compass = HMC5883L();
  compass.initialize(); // 方位磁石センサーの初期化
}

void loop() {
  while (Serial.available() > 0) {
    gps.encode(Serial.read());
  }

  if (gps.location.isUpdated() && gps.time.isUpdated()) {
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    unsigned long time = gps.time.value(); // GPSからの時刻取得

    // 太陽の位置を計算
    SunPosition sunPosition;
    sunPosition.setLocation(latitude, longitude);

    // 現在のUTC時刻
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    int hours = time / 10000; // 時
    int minutes = (time / 100) % 100; // 分
    int seconds = time % 100; // 秒

    // 太陽の位置を計算
    float altitude, azimuth;
    sunPosition.calculate(year, month, day, hours, minutes, seconds, &altitude, &azimuth);

    // 方位磁石データの取得
    Vector norm = compass.readNormalize();
    float heading = atan2(norm.YAxis, norm.XAxis);
    heading = heading * 180/M_PI; // ラジアンを度に変換
    if (heading < 0) {
      heading = 360 + heading;
    }

    // JSONオブジェクトの作成
    StaticJsonDocument<200> jsonDoc; // 必要なサイズに応じて調整
    jsonDoc["latitude"] = latitude;
    jsonDoc["longitude"] = longitude;
    jsonDoc["altitude"] = altitude;
    jsonDoc["azimuth"] = azimuth;
    jsonDoc["heading"] = heading;

    // JSON形式で出力
    serializeJson(jsonDoc, Serial);
    Serial.println(); // 改行を追加

    // サーボモーターを太陽の方位に設定
    float servoAngle = azimuth - heading; // 太陽の方位から現在の方向を引く
    if (servoAngle < 0) {
      servoAngle += 360; // 正の角度に変換
    }
    if (servoAngle > 180) {
      servoAngle -= 360; // 180度を超えたら範囲を調整
    }
    myServo.write(servoAngle); // サーボモーターを設定された角度に動かす
  }

  delay(1000); // 1秒待つ
}
