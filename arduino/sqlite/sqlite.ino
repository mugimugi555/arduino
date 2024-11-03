/*
arduino-cli lib install "Arduino_JSON"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
*/

#include <DHT.h>
#include <Arduino_JSON.h>
#include <sqlite3.h>

// DHTセンサー設定
#define DHTPIN 4      // DHT11センサーが接続されているピン
#define DHTTYPE DHT11 // 使用するDHTセンサーの種類
DHT dht(DHTPIN, DHTTYPE);

// SQLite設定
sqlite3 *db;

// SQLクエリのバッファサイズ
const int SQL_QUERY_SIZE = 512;

// シリアル通信で受け取るコマンドのバッファサイズ
char commandBuffer[128];

// データ取得間隔（1秒）
unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // SQLiteデータベースの初期化
  if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
    Serial.println("データベースの初期化に失敗しました！");
    return;
  }

  // テーブルの作成
  const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS dht_data (id INTEGER PRIMARY KEY, timestamp TEXT, temperature REAL, humidity REAL, discomfort_index REAL);";
  char *errMsg;
  if (sqlite3_exec(db, sqlCreateTable, NULL, NULL, &errMsg) != SQLITE_OK) {
    Serial.println("テーブルの作成に失敗しました！");
    sqlite3_free(errMsg);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // DHT11からデータを取得
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHTセンサーのデータ取得に失敗しました！");
      return;
    }

    // 不快指数の計算
    float discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;

    // データベースに挿入
    char sqlInsert[SQL_QUERY_SIZE];
    snprintf(sqlInsert, SQL_QUERY_SIZE, "INSERT INTO dht_data (timestamp, temperature, humidity, discomfort_index) VALUES (datetime('now'), %f, %f, %f);", temperature, humidity, discomfortIndex);
    char *errMsg;
    if (sqlite3_exec(db, sqlInsert, NULL, NULL, &errMsg) != SQLITE_OK) {
      Serial.println("データの挿入に失敗しました！");
      sqlite3_free(errMsg);
    }
  }

  // シリアル通信でCRUDリクエストを受信
  if (Serial.available()) {
    size_t len = Serial.readBytesUntil('\n', commandBuffer, sizeof(commandBuffer) - 1);
    commandBuffer[len] = '\0'; // null終端
    processCommand(commandBuffer);
  }
}

// シリアルコマンドの処理
void processCommand(char *command) {
  char responseBuffer[512];
  String response;

  if (strncmp(command, "/item?", 6) == 0) {
    // SQL条件付き絞り込みの処理
    int tempMin, tempMax, humMin, humMax;
    float diMin, diMax;

    // コマンドから条件を取得（例: /item?tempMin=20&tempMax=30&humMin=40&humMax=70&diMin=60.0&diMax=80.0）
    sscanf(command, "/item?tempMin=%d&tempMax=%d&humMin=%d&humMax=%d&diMin=%f&diMax=%f", &tempMin, &tempMax, &humMin, &humMax, &diMin, &diMax);

    // クエリ作成
    snprintf(responseBuffer, sizeof(responseBuffer), "SELECT * FROM dht_data WHERE temperature >= %d AND temperature <= %d AND humidity >= %d AND humidity <= %d AND discomfort_index >= %f AND discomfort_index <= %f;", tempMin, tempMax, humMin, humMax, diMin, diMax);

    // クエリ実行
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, responseBuffer, -1, &stmt, NULL) == SQLITE_OK) {
      JSONVar jsonResponse = JSON.parse("[]"); // JSON配列を作成

      // クエリ結果の処理
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        JSONVar jsonRow;
        jsonRow["id"] = sqlite3_column_int(stmt, 0);
        jsonRow["timestamp"] = (const char *)sqlite3_column_text(stmt, 1);
        jsonRow["temperature"] = sqlite3_column_double(stmt, 2);
        jsonRow["humidity"] = sqlite3_column_double(stmt, 3);
        jsonRow["discomfort_index"] = sqlite3_column_double(stmt, 4);

        jsonResponse[jsonResponse.length()] = jsonRow;
      }

      response = JSON.stringify(jsonResponse); // JSON文字列に変換
      sqlite3_finalize(stmt);
    }
    else {
      response = "{\"error\":\"クエリの実行に失敗しました。\"}";
    }
  }
  else {
    response = "{\"error\":\"無効なコマンドです。\"}";
  }

  Serial.println(response); // JSON形式で結果をシリアル出力
}
