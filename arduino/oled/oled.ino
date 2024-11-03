#include <Wire.h>
#include "OLED.h"

// 0.91インチのOLEDディスプレイ接続:
// SDA - D4ピン
// SCL - D5ピン
// RST - D2ピン

OLED display(SDA, SCL);  // OLEDオブジェクトを作成

void setup() {
  // D2ピンを出力モードに設定 (リセットピンとして使用)
  pinMode(D2, OUTPUT);

  // D2ピンをLOWにしてOLEDをリセット
  digitalWrite(D2, LOW);
  delay(50);  // 50ミリ秒待機

  // D2ピンをHIGHにしてOLEDを動作させる
  digitalWrite(D2, HIGH);

  // シリアル通信を9600bpsで開始
  Serial.begin(9600);
  Serial.println("OLED test!");

  // OLEDディスプレイを初期化
  display.begin();

  // 「Hello」メッセージを表示
  display.print("Hello ");
  delay(3000);  // 3秒待機

  // 長いメッセージを表示
  display.print("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
  delay(3000);  // 3秒待機

  // ディスプレイをクリア
  display.clear();
  delay(3000);  // 3秒待機

  // メッセージの位置指定テスト
  display.print("TOP-LEFT");        // ディスプレイの左上に表示
  display.print("4th row", 4);      // 4行目に表示
  display.print("RIGHT-BOTTOM", 7, 4);  // 右下に表示
  delay(3000);  // 3秒待機

  // ディスプレイをOFFにするテスト
  display.off();
  display.print("3rd row", 3, 8);   // 3行目に表示
  delay(3000);  // 3秒待機

  // ディスプレイをONにするテスト
  display.on();
  delay(3000);  // 3秒待機
}

// 行と列のカウンタを初期化
int r = 0, c = 0;

void loop() {
  // 行を4で割った余りにリセット
  r = r % 4;

  // マイクロ秒に基づいて列を6で割った余りを取得
  c = micros() % 6;

  // 1行目に戻ったらディスプレイをクリア
  if (r == 0) display.clear();

  // "Hello"を表示 (行と列に基づく)
  display.print("Hello ", r++, c++);

  // 500ミリ秒待機
  delay(500);
}
