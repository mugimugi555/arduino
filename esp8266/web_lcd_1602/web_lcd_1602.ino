#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <LiquidCrystal_I2C.h> // I2C接続のLCDディスプレイを制御するためのライブラリ。センサーデータやステータス情報を表示する際に使用します。

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

  //
  setupLCD();

}

void loop() {

}

//
void setupLCD() {

  // LCD初期化
  lcd.init();

   // バックライトをオンに
  lcd.backlight();

  //
  addCustomCharLCD();

 // 0〜255の文字コードを16個ずつ表示
  for (int start = 0; start < 256; start += 16) {

    lcd.clear();

    // LCDの2行に16個の文字を表示
    for (int i = 0; i < 8; i++) {
      lcd.setCursor(i, 0);
      lcd.write(start + i);      // 1行目の前半8文字を表示
      lcd.setCursor(i + 8, 0);
      lcd.write(start + i + 8);  // 1行目の後半8文字を表示
    }

    for (int i = 0; i < 8; i++) {
      lcd.setCursor(i, 1);
      lcd.write(start + i + 8);      // 2行目の前半8文字を表示
      lcd.setCursor(i + 8, 1);
      lcd.write(start + i + 8 + 8);  // 2行目の後半8文字を表示
    }

    delay(2000);  // 3秒間待機してから次の16個の文字に切り替え

  }

}

//
void addCustomCharLCD() {

  byte yen[8] = {
    0b00000,
    0b01010,
    0b00100,
    0b11111,
    0b00100,
    0b11111,
    0b00100,
    0b01010
  };

  byte degree[8] = {
    0b00110,
    0b01001,
    0b00110,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
  };

  byte waterDrop[8] = {
    0b00100,
    0b00100,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00000
  };

  byte sun[8] = {
    0b01010,
    0b11111,
    0b01110,
    0b11111,
    0b01110,
    0b11111,
    0b01010,
    0b00000
  };

  byte cloud[8] = {
    0b00000,
    0b00110,
    0b11111,
    0b11111,
    0b11110,
    0b01100,
    0b00000,
    0b00000
  };

  byte umbrella[8] = {
    0b00100,
    0b11111,
    0b11111,
    0b00100,
    0b00100,
    0b01010,
    0b10001,
    0b00000
  };

  byte moon[8] = {
    0b00111,
    0b01110,
    0b11110,
    0b11110,
    0b11110,
    0b01110,
    0b00111,
    0b00000
  };

  byte warning[8] = {
    0b00100,
    0b00100,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b00100,
    0b00000
  };

  lcd.createChar(0, yen);
  lcd.createChar(1, degree);
  lcd.createChar(2, waterDrop);
  lcd.createChar(3, sun);
  lcd.createChar(4, cloud);
  lcd.createChar(5, umbrella);
  lcd.createChar(6, moon);
  lcd.createChar(7, warning);

}