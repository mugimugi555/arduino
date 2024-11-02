#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <DHT.h>
#include <ArduinoJson.h>

/*
DHT11/DHT22の接続図
DHT11/DHT22ピン	ESP8266ピン
VCC	3V3
GND	GND
DATA	D4（GPIO2）
*/

#define DHTPIN D4      // DHTセンサーのピン
#define DHTTYPE DHT11  // DHT11 または DHT22
DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");  // WebSocketのパス

unsigned long previousMillis = 0;  // 前回の時間を記録
const long interval = 1000;         // データ取得の間隔（ミリ秒）

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD"); // WiFi接続
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  dht.begin();

  // HTMLファイルの配信
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  // WebSocket接続のコールバック設定
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("Client disconnected");
    }
  });

  server.addHandler(&ws);
  server.begin();
}

void loop() {
  // すべてのAsyncWebSocketイベントを処理
  ws.cleanupClients();

  // 現在の時間を取得
  unsigned long currentMillis = millis();

  // 一定間隔でデータを取得
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // 前回の時間を更新
    sendSensorData();                 // センサーデータを取得して送信
  }
}

// センサーのデータを取得してJSON形式で送信する関数
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hi = calculateHeatIndex(t, h);  // 不快指数を計算

  // JSONオブジェクトの作成
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = t;
  jsonDoc["humidity"] = h;
  jsonDoc["heatIndex"] = hi;  // 不快指数を追加

  // JSONデータを文字列に変換
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // SerialにJSON形式で出力
  Serial.println(jsonString);

  // WebSocketでJSONデータを送信
  ws.textAll(jsonString);
}

// 不快指数を計算する関数
float calculateHeatIndex(float temperature, float humidity) {
  float c1 = 16.99;
  float c2 = 0.567 * temperature;
  float c3 = -0.393 * humidity;
  float c4 = 0.00193 * temperature * humidity;
  float c5 = 0.00357 * temperature * temperature;
  float c6 = -0.00234 * humidity * humidity;
  float c7 = 0.00073 * temperature * temperature * humidity;
  float c8 = -0.00057 * temperature * humidity * humidity;
  float c9 = 0.000005 * temperature * temperature * humidity * humidity;

  return c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9;
}

// HTMLの定義
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 温湿度モニター</title>
  <script src="https://cdn.jsdelivr.net/npm/vue@3"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> <!-- Chart.jsを追加 -->
</head>
<body>
  <div id="app">
    <h1>温湿度センサー</h1>
    <p>温度: {{ temperature }} °C</p>
    <p>湿度: {{ humidity }} %</p>
    <p>不快指数: {{ heatIndex }} </p> <!-- 不快指数を表示 -->

    <canvas id="temperatureChart" width="400" height="200"></canvas> <!-- 温度グラフ用のキャンバス -->
    <canvas id="humidityChart" width="400" height="200"></canvas> <!-- 湿度グラフ用のキャンバス -->
  </div>

  <script>
    const { createApp } = Vue;

    createApp({
      data() {
        return {
          temperature: null,
          humidity: null,
          heatIndex: null,  // 不快指数を追加
          temperatureData: [],  // 温度データ
          humidityData: []  // 湿度データ
        };
      },
      mounted() {
        const ws = new WebSocket("ws://" + location.hostname + ":80/ws");
        ws.onmessage = (event) => {
          const data = JSON.parse(event.data);
          this.temperature = data.temperature;
          this.humidity = data.humidity;
          this.heatIndex = data.heatIndex;  // 不快指数を更新

          // データを配列に追加
          this.temperatureData.push(this.temperature);
          this.humidityData.push(this.humidity);

          // グラフを更新
          this.updateCharts();
        };

        // グラフの初期化
        this.initCharts();
      },
      methods: {
        initCharts() {
          const ctxTemp = document.getElementById('temperatureChart').getContext('2d');
          const ctxHum = document.getElementById('humidityChart').getContext('2d');

          this.temperatureChart = new Chart(ctxTemp, {
            type: 'line',
            data: {
              labels: [], // ラベルは空で初期化
              datasets: [{
                label: '温度 (°C)',
                data: this.temperatureData,
                borderColor: 'rgba(255, 99, 132, 1)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                borderWidth: 1,
                fill: true
              }]
            },
            options: {
              scales: {
                y: {
                  beginAtZero: true
                }
              }
            }
          });

          this.humidityChart = new Chart(ctxHum, {
            type: 'line',
            data: {
              labels: [], // ラベルは空で初期化
              datasets: [{
                label: '湿度 (%)',
                data: this.humidityData,
                borderColor: 'rgba(54, 162, 235, 1)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                borderWidth: 1,
                fill: true
              }]
            },
            options: {
              scales: {
                y: {
                  beginAtZero: true
                }
              }
            }
          });
        },
        updateCharts() {
          const currentTime = new Date().toLocaleTimeString(); // 現在の時間を取得
          this.temperatureChart.data.labels.push(currentTime);
          this.humidityChart.data.labels.push(currentTime);

          // 新しいデータをグラフに追加
          this.temperatureChart.data.datasets[0].data = this.temperatureData;
          this.humidityChart.data.datasets[0].data = this.humidityData;

          // グラフを更新
          this.temperatureChart.update();
          this.humidityChart.update();
        }
      }
    }).mount("#app");
  </script>
</body>
</html>
)rawliteral";