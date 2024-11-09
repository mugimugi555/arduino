#include "HX711.h"

// ピン設定
const int LOADCELL_DOUT_PIN = 2;  // DT
const int LOADCELL_SCK_PIN = 3;   // SCK

HX711 scale;

// 50gの重り
float knownWeight = 50.0;  // 50gの重り（g）
long rawValue = 0;         // 生の読み取り値
float scaleFactor = 0;     // 計算したスケールファクター

// ゼロリセットに必要な安定した読み取り値の閾値
const long zeroThreshold = 10;  // 読み取る値の変動が10以下なら安定と判断

// 重量変化を判断するための閾値
const long weightThreshold = 20;  // 20g程度の変化で重りが載ったとみなす

void setup() {
  Serial.begin(9600);

  // センサーを初期化
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // ゼロリセットの検出
  long previousReading = scale.get_units(10);  // 初期値を取得
  long currentReading;
  int stableCount = 0;  // 安定した回数をカウント
  unsigned long startTime = millis();  // タイマー開始

  // 10秒間、値が安定するまでチェック
  while (millis() - startTime < 10000) {
    currentReading = scale.get_units(10);  // 現在の値を取得
    if (abs(currentReading - previousReading) < zeroThreshold) {
      stableCount++;  // 値が安定していればカウントを増やす
    } else {
      stableCount = 0;  // 変動があればカウントをリセット
    }
    previousReading = currentReading;

    // 5回連続で安定すればゼロリセット
    if (stableCount >= 5) {
      Serial.println("ゼロリセット完了");
      scale.tare();  // ゼロリセット
      break;
    }

    delay(100);  // 0.1秒ごとにチェック
  }

  // 50gの重りを載せるタイミングを検出
  Serial.println("50gの重りを載せてください...");
  stableCount = 0;  // 再カウント
  previousReading = scale.get_units(10);

  // 重りが載ったかの確認
  while (true) {
    currentReading = scale.get_units(10);  // 現在の値を取得
    if (abs(currentReading - previousReading) > weightThreshold) {
      stableCount++;  // 大きな変動があればカウントアップ
    } else {
      stableCount = 0;  // 変動が少ない場合リセット
    }
    previousReading = currentReading;

    // 5回安定すれば50gの重りとして認識
    if (stableCount >= 5) {
      Serial.println("50gの重りを載せたと認識");
      rawValue = scale.get_units(10);  // 重りを載せた状態で値を取得
      scaleFactor = rawValue / knownWeight;  // スケールファクター計算
      scale.set_scale(scaleFactor);  // 計算したスケールファクターを設定
      Serial.print("計算したスケールファクター: ");
      Serial.println(scaleFactor);
      break;
    }

    delay(100);  // 0.1秒ごとにチェック
  }

  Serial.println("実際の測定を開始します...");
}

void loop() {
  // 実際の物体を載せて重量を測定
  long weight = scale.get_units(10);  // 10回平均を取って安定させる
  Serial.print("測定した重量: ");
  Serial.print(weight);
  Serial.println(" g");

  delay(1000);  // 1秒ごとに読み取る
}
