#include <ELMduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
ELM327 myELM327;

#define DEBUG_SERIAL Serial

void setup() {
  DEBUG_SERIAL.begin(115200);  // デバッグ出力
  SerialBT.begin("ESP32_OBD"); // Bluetoothの初期化

  if (!SerialBT.connect("OBDII")) { // ELM327の名前が"OBDII"の場合
    DEBUG_SERIAL.println("Bluetooth接続に失敗しました！");
    return;
  }
  DEBUG_SERIAL.println("Bluetooth接続に成功しました！");

  // ELM327の初期化
  if (!myELM327.begin(SerialBT, true, 2000)) {
    DEBUG_SERIAL.println("ELM327の初期化に失敗しました");
    SerialBT.disconnect();
  } else {
    DEBUG_SERIAL.println("ELM327の初期化に成功しました");
  }
}

void loop() {
  float rpm = myELM327.rpm(); // エンジンの回転数取得
  if (myELM327.status == ELM_SUCCESS) {
    DEBUG_SERIAL.print("RPM: ");
    DEBUG_SERIAL.println(rpm);
  } else {
    DEBUG_SERIAL.println("RPMの取得に失敗しました");
  }

  float speed = myELM327.kph(); // 車速取得
  if (myELM327.status == ELM_SUCCESS) {
    DEBUG_SERIAL.print("Speed: ");
    DEBUG_SERIAL.println(speed);
  } else {
    DEBUG_SERIAL.println("Speedの取得に失敗しました");
  }

  delay(1000); // 1秒ごとに取得
}
