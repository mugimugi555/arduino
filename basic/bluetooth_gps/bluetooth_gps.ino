#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

/*
接続ピン情報
GPSセンサー:
- TX -> Arduino RX (ハードウェアシリアル)
- RX -> Arduino TX (ハードウェアシリアル)

Bluetoothモジュール:
- TX -> Arduino ピン10 (SoftwareSerialのRXピン)
- RX -> Arduino ピン11 (SoftwareSerialのTXピン)

VCC -> Arduinoの5Vまたは3.3V
GND -> ArduinoのGND
*/

TinyGPSPlus gps;
SoftwareSerial btSerial(10, 11); // RX, TX for Bluetooth connection
const int GPS_BAUD = 9600;
const int BT_BAUD = 9600;

const char* bluetoothName = "GPS_Module"; // Bluetoothモジュールの名前

float lastSpeed = 0.0;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(GPS_BAUD);      // GPSセンサーのハードウェアシリアル
  btSerial.begin(BT_BAUD);     // Bluetoothのソフトウェアシリアル

  delay(1000);
  Serial.println("Sending AT commands to set Bluetooth name...");
  btSerial.println("AT");           // ATコマンドモード確認
  delay(1000);
  btSerial.print("AT+NAME="); // 名前を設定するためのコマンド
  btSerial.println(bluetoothName); // 変数からBluetoothモジュールの名前を取得
  delay(1000);

  Serial.println("Initializing GPS and Bluetooth...");
}

void loop() {
  updateGPSData();
  sendJSONData();
  delay(1000); // 1秒間隔で送信
}

// GPSデータの更新とパース
void updateGPSData() {
  while (Serial.available() > 0) {
    gps.encode(Serial.read());
  }
}

// JSONデータの作成とBluetoothへの送信
void sendJSONData() {
  // 現在の速度を取得して加速度を計算
  float currentSpeed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
  unsigned long currentTime = millis();
  float acceleration = 0.0;

  if (lastTime > 0 && currentTime > lastTime) {
    float deltaTime = (currentTime - lastTime) / 1000.0;  // 秒に変換
    acceleration = (currentSpeed - lastSpeed) / deltaTime; // m/s^2
  }

  lastSpeed = currentSpeed;
  lastTime = currentTime;

  // JSONの生成
  StaticJsonDocument<500> jsonDoc;

  // GPSデータの設定
  if (gps.location.isValid()) {
    jsonDoc["latitude"] = gps.location.lat();
    jsonDoc["longitude"] = gps.location.lng();
  } else {
    jsonDoc["latitude"] = "N/A";
    jsonDoc["longitude"] = "N/A";
  }

  if (gps.altitude.isValid()) {
    jsonDoc["altitude"] = gps.altitude.meters();  // 標高
  } else {
    jsonDoc["altitude"] = "N/A";
  }

  jsonDoc["speed"] = currentSpeed;           // 速度 (km/h)
  jsonDoc["acceleration"] = acceleration;    // 加速度 (m/s^2)

  if (gps.course.isValid()) {
    jsonDoc["direction"] = gps.course.deg(); // 進行方向 (度)
  } else {
    jsonDoc["direction"] = "N/A";
  }

  // 現在の日時を取得
  if (gps.date.isValid() && gps.time.isValid()) {
    setTime(gps.time.value());  // GPSからの時間を設定
    jsonDoc["datetime"] = String(hour()) + ":" + String(minute()) + ":" + String(second()) + " " +
                          String(day()) + "/" + String(month()) + "/" + String(year());
  } else {
    jsonDoc["datetime"] = "N/A";
  }

  // 衛星情報
  jsonDoc["satellites"] = gps.satellites.value();  // 使用中の衛星の数
  jsonDoc["hdop"] = gps.hdop.hdop();               // 水平精度

  // JSONデータをBluetooth経由で送信
  String output;
  serializeJson(jsonDoc, output);
  btSerial.println(output);  // スマホやPCに送信

  // シリアルモニターにも出力（デバッグ用）
  Serial.println(output);
}
