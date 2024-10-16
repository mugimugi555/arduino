#include <BleMouse.h>

//
BleMouse bleMouse("auto_scroller");

// LEDの接続ピンを定義
const int redLEDPin  = 2; // 赤色LEDを接続するピン
const int blueLEDPin = 4; // 青色LEDを接続するピン

//
void setup() {

  //
  Serial.begin(115200);
    
  // LEDピンの設定
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
    
  // LEDの初期状態を設定
  digitalWrite(redLEDPin, HIGH);  // 赤色LEDを点灯
  digitalWrite(blueLEDPin, LOW);  // 青色LEDを消灯
    
  bleMouse.begin(); // BLEマウスの初期化

}

//
void loop() {

  // マウスが接続されている場合
  if (bleMouse.isConnected()) {

    // 赤色LEDを消灯し、青色LEDを点灯
    digitalWrite(redLEDPin, LOW);   // 赤色LEDを消灯
    digitalWrite(blueLEDPin, HIGH); // 青色LEDを点灯
        
    // スクロールのシミュレーション

    // ここでは、1秒ごとにスクロールダウン（y軸の値を負にする）
    bleMouse.move(0, 0, -1); // y方向に-1スクロール（下方向）
    delay(1000 * 15); // 15秒待つ

    //bleMouse.scroll(0, 1); // y方向に1スクロール（上方向）
    //delay(1000); // 1秒待つ

  } else {

    // 赤色LEDを点灯し、青色LEDを消灯
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(blueLEDPin, LOW);

    // 1秒ごとに赤色LEDを点滅
    delay(1000);
    digitalWrite(redLEDPin, LOW);
    delay(1000);

  }

}
