#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLECharacteristic.h>
#include <HIDTypes.h>
#include <BleKeyboard.h>

// Bluetoothデバイスの名前を定義
const char* bluetoothName = "BLUETOOTH_NAME";  // BLEの名前

int intervalTime = 1000 * 15; // 15秒間隔

// BLEキーボードのオブジェクトを作成
BleKeyboard bleKeyboard(bluetoothName);

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

  // BLEキーボードを開始
  Serial.println("BLEキーボードの初期化開始");
  bleKeyboard.begin();
  Serial.println("BLEキーボードの初期化完了");
}

void loop() {
  // BLEデバイスが接続されている場合
  if (bleKeyboard.isConnected()) {
    Serial.println("BLEキーボードが接続されました");

    // 赤色LEDを消灯し、青色LEDを点灯
    digitalWrite(redLEDPin, LOW);
    digitalWrite(blueLEDPin, HIGH);

    // キーボードの「下」キーをエミュレート
    Serial.println("カーソルキーの下を押下");
    bleKeyboard.press(KEY_DOWN_ARROW);  // カーソルキーの「下」を押す
    delay(100);                         // 少し待機
    bleKeyboard.releaseAll();           // キーを離す
    Serial.println("カーソルキーの下を離しました");

    delay(intervalTime); // 15秒待つ

  } else {
    Serial.println("BLEキーボードが未接続です");

    // BLEが未接続の場合、赤色LEDを点灯し、青色LEDを消灯
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(blueLEDPin, LOW);

    // 1秒ごとに赤色LEDを点滅
    delay(1000);
    digitalWrite(redLEDPin, LOW);
    delay(1000);
  }
}
