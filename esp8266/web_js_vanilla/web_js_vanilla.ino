#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 4          // DHTセンサーのピン
#define DHTTYPE DHT11     // 使用するDHTセンサーのタイプ
#define READ_INTERVAL 2000 // センサー読み取り間隔（ミリ秒）

const char* ssid = "SSID";         // WiFi SSID
const char* password = "PASSWORD"; // WiFi パスワード

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

unsigned long lastReadTime = 0; // 最後にセンサーを読み取った時間

// HTMLコンテンツ（Vanilla JavaScriptを使用）
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta charset="UTF-8">
  <title>温湿度リアルタイム表示</title>
  <style>
    .data {
      font-size: 2em;
    }
  </style>
</head>
<body>
  <h1>温湿度センサー</h1>
  <div>温度: <span id="temperature" class="data">--</span> °C</div>
  <div>湿度: <span id="humidity" class="data">--</span> %</div>

  <script>
    document.addEventListener("DOMContentLoaded", function() {
      const temperatureElem = document.getElementById('temperature');
      const humidityElem = document.getElementById('humidity');

      // WebSocket接続を確立
      const socket = new WebSocket(`ws://${location.hostname}/ws`);

      // データを受信したときの処理
      socket.onmessage = function(event) {
        const data = JSON.parse(event.data);
        temperatureElem.textContent = data.temperature;
        humidityElem.textContent = data.humidity;
      };

      socket.onopen = function() {
        console.log('WebSocket 接続成功');
      };

      socket.onerror = function(error) {
        console.log('WebSocket エラー', error);
      };
    });
  </script>
</body>
</html>
)rawliteral";

// WiFi接続
void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
}

// Webサーバーのセットアップ
void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", htmlPage);
  });

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.begin();
}

// WebSocketイベント処理
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket クライアントが接続されました");
  }
}

// センサーのデータを取得し、JSONで出力
void readAndSendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = t;
    jsonDoc["humidity"] = h;

    String jsonData;
    serializeJson(jsonDoc, jsonData);

    ws.textAll(jsonData);      // WebSocketで送信
    Serial.println(jsonData);   // シリアルで出力
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectToWiFi();
  setupWebServer();
  Serial.println("Webサーバーが開始されました");
}

void loop() {
  ws.cleanupClients();

  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;
    readAndSendSensorData();
  }
}
