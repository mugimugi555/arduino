#include <ESP8266WiFi.h>
#include <DHT.h>
#include <KDEConnect.h>

const char* ssid = "your_SSID";            // Wi-FiのSSID
const char* password = "your_PASSWORD";     // Wi-Fiのパスワード

#define DHTPIN 2                              // DHTセンサーのデータピン
#define DHTTYPE DHT11                         // DHT11センサーを使用

DHT dht(DHTPIN, DHTTYPE);
KDEConnect kde;

const float discomfortIndexThreshold = 80.0; // 不快指数の閾値（例: 80以上でコマンド送信）

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wi-Fi接続待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  dht.begin(); // DHTセンサーの初期化
  kde.begin(); // KDE Connectの初期化
}

void loop() {
  kde.loop(); // KDE Connectのループ処理

  // 温度と湿度を読み取る
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // 摂氏温度

  // 不快指数を計算
  float discomfortIndex = calculateDiscomfortIndex(t, h);

  // 不快指数が閾値を超えた場合にコマンドを送信
  if (discomfortIndex > discomfortIndexThreshold) {
    Serial.println("Discomfort index exceeded threshold!");
    kde.sendMessage("Your KDE Connect Command"); // 登録されたコマンドを送信
  }

  delay(10000); // 10秒ごとに測定
}

// 不快指数を計算する関数
float calculateDiscomfortIndex(float temperature, float humidity) {
  return (0.81 * temperature) + (0.01 * humidity * (0.99 * temperature - 14.3)) + 46.3;
}
