#include <LiquidCrystal_I2C.h>

// set lcd address
LiquidCrystal_I2C lcd(0x27,16,2);  // 0x27 or 0x3F

// set analog input
const int AQ0 = A0;   //MQ-3センサモジュールA0の出力先

void setup() {
  Serial.begin(115200);

  // init lcd
  lcd.init();
  lcd.clear();
  lcd.backlight();      // Make sure backlight is on
  
  // show title
  lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
  lcd.print("MQ-3 Alcohol");

  // CSVのヘッダーを出力
  Serial.println("Alcohol Sensor Value (V)");
}

void loop() {
  float A0_out;

  // read alcohol sensor analog 0 pin
  A0_out = (float)analogRead(AQ0) / 1024.0f * 5.0f;

  // CSV形式でシリアルに出力
  Serial.println(A0_out, 4);

  // show out lcd
  lcd.setCursor(2,1);
  lcd.print(A0_out);

  delay(1000);
}
