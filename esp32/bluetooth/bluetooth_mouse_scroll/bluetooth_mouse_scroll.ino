#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLECharacteristic.h>
#include <HIDTypes.h>
#include <BleMouse.h>

// Bluetoothデバイスの名前を定義
const char* bluetoothName = "BLUETOOTH_NAME";  // BLEの名前

int intervalTime = 1000 * 15; // 15秒間隔

// BLEマウスのオブジェクトを作成
BleMouse bleMouse(bluetoothName);

// LEDピンを定義
const int redLEDPin = 2;   // 赤色LEDを接続するピン
const int blueLEDPin = 4;  // 青色LEDを接続するピン

void setup() {
  Serial.begin(115200);
  Serial.println("セットアップ開始");

  // LEDピンの設定
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);

  // 初期LED状態設定
  digitalWrite(redLEDPin, HIGH);  // 赤色LED点灯
  digitalWrite(blueLEDPin, LOW);  // 青色LED消灯

  // BLEマウスを開始
  Serial.println("BLEマウスの初期化開始");
  bleMouse.begin();
  Serial.println("BLEマウスの初期化完了");
}

void loop() {
  // BLEデバイスが接続されている場合
  if (bleMouse.isConnected()) {
    Serial.println("BLEマウスが接続されました");

    // 赤色LEDを消灯し、青色LEDを点灯
    digitalWrite(redLEDPin, LOW);
    digitalWrite(blueLEDPin, HIGH);

    // マウススクロールのシミュレーション
    Serial.println("マウススクロール実行");
    bleMouse.move(0, 0, -1); // 下方向にスクロール（Y軸 -1）

    delay(intervalTime); // 15秒待つ
  } else {
    Serial.println("BLEマウスが未接続です");

    // BLEが未接続の場合、赤色LEDを点灯し、青色LEDを消灯
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(blueLEDPin, LOW);

    // 1秒ごとに赤色LEDを点滅
    delay(1000);
    digitalWrite(redLEDPin, LOW);
    delay(1000);
  }
}
