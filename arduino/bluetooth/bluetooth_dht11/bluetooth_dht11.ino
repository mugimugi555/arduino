#include <DHT.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define DHTPIN 2       // DHT11のデータピン
#define DHTTYPE DHT11  // DHT11センサーの種類

DHT dht(DHTPIN, DHTTYPE);

// HC-05 Bluetoothを接続したピン
SoftwareSerial BTSerial(10, 11); // RX, TX

void setup() {

  Serial.begin(115200);  // Bluetoothのデフォルト通信速度
  dht.begin();
  BTSerial.begin(9600);   // HC-05のデフォルトのATモード通信速度

}

void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 不快指数の計算
  float discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;

  // JSON形式でデータを構築
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["discomfortIndex"] = discomfortIndex;

  //
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  //
  Serial.println(jsonResponse);
  BTSerial.println(jsonResponse);

  delay(2000);  // 2秒間隔で送信

}
