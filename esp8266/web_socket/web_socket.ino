//arduino-cli lib install "ESPAsyncWebServer"
//arduino-cli lib install "ESPAsyncTCP"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワード、ホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

// DHT設定
#define DHTPIN D4    // DHTセンサー接続ピン
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WebSocketサーバー設定
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>DHT11 WebSocket</title>
</head>
<body>
  <h1>DHT11 WebSocket Data</h1>
  <p>Temperature: <span id="temp">--</span>°C</p>
  <p>Humidity: <span id="hum">--</span>%</p>

  <script>
    const tempElement = document.getElementById('temp');
    const humElement = document.getElementById('hum');

    const ws = new WebSocket(`ws://${window.location.hostname}/ws`);

    ws.onmessage = function(event) {
      const data = JSON.parse(event.data);
      tempElement.textContent = data.temp;
      humElement.textContent = data.hum;
    };
  </script>
</body>
</html>
)rawliteral";

void notifyClients(float temperature, float humidity) {
  String message = "{\"temp\":";
  message += String(temperature);
  message += ",\"hum\":";
  message += String(humidity);
  message += "}";
  ws.textAll(message);  // 全クライアントにメッセージを送信
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // クライアントからのメッセージ処理（必要に応じて）
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client connected: %u\n", client->id());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client disconnected: %u\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // WiFi接続
  connectToWiFi();

  // DHTセンサー初期化
  dht.begin();

  // WebSocketサーバー初期化
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Webページを返すルートを設定
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", htmlPage);
  });

  server.begin();
}

void loop() {

  // 温度と湿度を取得
  /*
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // データを更新して送信
  if (!isnan(temperature) && !isnan(humidity)) {
    notifyClients(temperature, humidity);
  }
  */

  // 簡易的に乱数を生成して送信
  float randomTemperature = random(15, 30);  // 15〜30の間の乱数
  float randomHumidity = random(40, 60);      // 40〜60の間の乱数
  notifyClients(randomTemperature, randomHumidity);

  ws.cleanupClients();  // クライアントの管理
  delay(500);  // 2秒ごとにデータを更新

}


//----------------------------------------------------------------------------
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // WiFi接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // mDNSサービスの開始
  Serial.println("");
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.println("===============================================");
  Serial.println("              Network Details                  ");
  Serial.println("===============================================");
  Serial.print("WebServer    : http://");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname     : http://");
  Serial.print(hostname);
  Serial.println(".local");
  Serial.print("IP address   : ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask  : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP   : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP       : ");
  Serial.println(WiFi.dnsIP());
  Serial.print("MAC address  : ");
  Serial.println(WiFi.macAddress());
  Serial.println("===============================================");

}