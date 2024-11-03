#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2Cアドレス0x27、16x2のLCDを初期化

// カスタムキャラクター用のバイト配列
byte customChar[8]; // 受け取ったカスタムキャラクターのビットマップを保存
int bitIndex = 0;   // 何行目かを示すインデックス

void setup() {
  Serial.begin(9600);   // シリアル通信速度をBashと合わせる
  lcd.init();           // LCDの初期化
  lcd.backlight();      // 背景ライトをオン
}

void loop() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();

    // 受信データをそのままシリアル表示
    Serial.print(incomingByte);

    // 数字文字の場合にのみ処理
    if (incomingByte == '0' || incomingByte == '1') {
      // 文字をバイトデータにシフトして追加
      customChar[bitIndex / 5] <<= 1;            // 1ビット左シフト
      customChar[bitIndex / 5] |= (incomingByte - '0'); // 0または1をORで追加
      bitIndex++;

      // 次の行に進む条件
      if (bitIndex % 5 == 0 && bitIndex > 0) {
        bitIndex += 3; // 行終わりに残り3ビット分のパディング
      }

      // カスタムキャラクターが完成した場合
      if (bitIndex >= 40) {
        lcd.createChar(0, customChar);  // LCDに登録
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(0)); // カスタムキャラクターを描画

        // 作成したカスタムキャラクターをシリアルモニターに表示
        Serial.println("\n\nCustom character created:");
        for (int i = 0; i < 8; i++) {
          Serial.print("Row ");
          Serial.print(i);
          Serial.print(": ");
          Serial.println(customChar[i], BIN); // バイナリ形式で表示
        }

        bitIndex = 0;  // 初期化して次のキャラクターを受信できるようにする
      }
    }
  }
}
