
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPSモジュールのTX/RXピンをArduinoに接続
const int RXPin = 4;   // Arduinoのデジタルピン4をGPSのTXに接続
const int TXPin = 3;   // Arduinoのデジタルピン3をGPSのRXに接続
const uint32_t GPSBaud = 38400;  // E108-GN04Dのボーレート

// GPSオブジェクトの作成
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(115200);  // ArduinoとPC間のシリアル通信の初期化
  ss.begin(GPSBaud);     // GPSモジュールとのシリアル通信の初期化
  Serial.println("GPSモジュールからの生データを取得中...");
}

void loop() {
  // GPSモジュールからデータを読み取り、直接シリアルモニタに出力
  while (ss.available() > 0) {
    char c = ss.read();          // 1バイトずつ読み取る
    Serial.write(c);              // 読み取ったバイトをそのままシリアルに出力
    gps.encode(c);                // TinyGPS++でデコード

    // デコードが完了したら位置情報を出力
    if (gps.location.isUpdated()) {
      Serial.print("緯度: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(", 経度: ");
      Serial.print(gps.location.lng(), 6);
      Serial.print(", 高度: ");
      Serial.print(gps.altitude.meters());
      Serial.print(" m, 速度: ");
      Serial.print(gps.speed.kmph());
      Serial.println(" km/h");
    }

    // 時刻情報も更新されたら出力
    if (gps.time.isUpdated()) {
      Serial.print("時刻: ");
      Serial.print(gps.time.hour());
      Serial.print(":");
      Serial.print(gps.time.minute());
      Serial.print(":");
      Serial.println(gps.time.second());
    }
  }
}
