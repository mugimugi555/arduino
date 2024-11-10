
//
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <HX711.h>
#include <Wire.h>

// HX711ピン設定
#define LOAD_CELL_DOUT_PIN  7  // D5
#define LOAD_CELL_SCK_PIN   6  // D4
HX711 scale;

// 定形外郵便の重量（規格内・規格外）と料金
int regularPostWeight[6]   = {50,100,150,250,500,1000};  // 規格内の重量
int regularPostCost[6]     = {140,180,270,320,510,750};                                // 規格内の料金
int irregularPostWeight[8] = {50,100,150,250,500,1000,2000,4000};  // 規格外の重量
int irregularPostCost[8]   = {260,290,390,450,660,920,1350,1750};

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 1;

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  //
  scale.begin(LOAD_CELL_DOUT_PIN, LOAD_CELL_SCK_PIN);
  scale.set_scale();  // スケール初期化
  scale.tare(-185267-60615,);       // オフセット調整
  //scale.set_scale(1000); // キャリブレーションファクター（例として1000を使用）
  //
  createJson();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

}

// 重量に応じた料金を取得
int getCostRegular(float weight) {

  if (weight <= 50) return regularPostCost[0];
  else if (weight <=  100) return regularPostCost[1];
  else if (weight <=  150) return regularPostCost[2];
  else if (weight <=  250) return regularPostCost[3];
  else if (weight <=  500) return regularPostCost[4];
  else if (weight <= 1000) return regularPostCost[5];
  else return 0; // 規格外対応は省略

}

// 重量に応じた料金を取得
int getCostIrregular(float weight) {

  if (weight <= 50) return irregularPostCost[0];
  else if (weight <=  100) return irregularPostCost[1];
  else if (weight <=  150) return irregularPostCost[2];
  else if (weight <=  250) return irregularPostCost[3];
  else if (weight <=  500) return irregularPostCost[4];
  else if (weight <= 1000) return irregularPostCost[5];
  else if (weight <= 2000) return irregularPostCost[6];
  else if (weight <= 4000) return irregularPostCost[7];
  else return 0; // 規格外対応は省略

}

// JSON形式で重量と料金を返す
String createJson() {

  // 重量測定
  float weight = scale.get_units(10); // 10回平均を取る
  Serial.print("Weight: ");
  Serial.println(weight);

  // JSONドキュメントのサイズを指定（必要に応じて増減）
  StaticJsonDocument<128> doc;

  // JSONにデータを追加
  doc["weight"] = weight;
  doc["costRegular"] = getCostRegular(weight);
  doc["costIrregular"] = getCostIrregular(weight);

  // JSON文字列にシリアライズ
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  // 結果をシリアルモニタに表示
  Serial.println(jsonResponse);

  return jsonResponse;

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// taskInterval 秒ごとに情報を表示する関数
void fetchAndShowTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;
    Serial.println(createJson());
  }

}
