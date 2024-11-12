#include <EEPROM.h>   // EEPROMライブラリ

// 起動時にハードウェア情報をシリアルモニタに出力する関数
void showStartup() {

  Serial.begin(9600);
  delay(1000); // シリアル通信の準備が整うのを待つ

  Serial.println("===============================================");
  Serial.println("  Arduino Hardware Information");
  Serial.println("===============================================");

  // ボード名を表示
  Serial.print("Board         : ");
  Serial.println("Arduino Uno");

  // CPUの周波数を表示
  Serial.print("CPU Frequency : ");
  Serial.print(F_CPU / 1000000);
  Serial.println(" MHz");

  // EEPROMサイズを表示
  Serial.print("EEPROM Size   : ");
  Serial.print(EEPROM.length());
  Serial.println(" bytes");

  // SRAMの空き容量を表示
  Serial.print("Free SRAM     : ");
  Serial.print(freeMemory());
  Serial.println(" bytes");

  // 内蔵温度センサーの値を表示（相対温度）
  Serial.print("Temperature (relative) : ");
  Serial.println(readTemperature());

  Serial.println("===============================================");
  Serial.println("");
}

// 内蔵温度センサーの値を読み取る関数（相対温度）
int readTemperature() {
  // 温度センサ用のADCチャネルにアクセス
  ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3);
  delay(10);
  // アナログ読み取り
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  int tempReading = ADC;
  // 温度計算は補正が必要（キャリブレーション次第）
  return tempReading;
}

// 使用可能なSRAMの容量を確認する関数
extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
  showStartup(); // ハードウェア情報を表示
}

void loop() {
  // メインループ（何も実行しません）
}
