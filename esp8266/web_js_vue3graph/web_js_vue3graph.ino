/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"        # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncTCP"        # ESP8266用の非同期TCP通信を提供するライブラリ。非同期的に複数のクライアントと接続するために使用します。
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "DHT sensor library" # DHT11やDHT22温湿度センサー用のライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESPAsyncWebServer.h> // ESP8266用の非同期Webサーバーライブラリ。HTTPリクエストの処理を非同期で行い、複数のクライアントからのリクエストに同時に対応できるようにします。
#include <ArduinoJson.h>       // JSON形式のデータを作成・解析するためのライブラリ。API通信やデータの保存に役立ちます。
#include <DHT.h>               // 温度・湿度センサーDHTシリーズを制御するためのライブラリ。DHT11やDHT22などに対応します。

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP8266のホスト名 http://HOSTNAME.local/ でアクセスできるようになります。

// センサーとの接続方法
// DHT11センサー       ESP8266
// VCC  <---------->  3.3V または 5V
// GND  <---------->  GND
// DATA <---------->  D4 (GPIO 2)

#define DHTPIN 2           // DHTセンサーのピン
#define DHTTYPE DHT11      // 使用するDHTセンサーのタイプ
#define READ_INTERVAL 2000 // センサー読み取り間隔（ミリ秒）

DHT dht(DHTPIN, DHTTYPE);

// 非同期Webサーバーの初期化
AsyncWebServer server(80);

// WebSocketの初期化
AsyncWebSocket ws("/ws");

//----------------------------------------------------------------------------
// HTMLコンテンツ（Vanilla(素の) JavaScriptを使用）
//----------------------------------------------------------------------------
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 温湿度モニター</title>
  <link rel="stylesheet" href='https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css'>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <script src='https://cdn.jsdelivr.net/npm/vue@3'></script>
</head>
<body>
  <div id="app" class="container mt-5">
    <h1 class="text-center">温湿度センサー VUE3 グラフ</h1>

    <h2>グラフ</h2>
    <div style="height: 400px;margin-bottom:14px;">
      <canvas id="myChart"></canvas>
    </div>

    <h2>センサーデータ</h2>
    <table class="table table-bordered table-striped">
      <thead class="thead-dark">
        <tr>
          <th>項目</th>
          <th>値</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <th>温度</th>
          <td>{{ sensorData.temperature }} °C</td>
        </tr>
        <tr>
          <th>湿度</th>
          <td>{{ sensorData.humidity }} %</td>
        </tr>
        <tr>
          <th>不快指数</th>
          <td>{{ sensorData.heatIndex }}</td>
        </tr>
        <tr>
          <th>ステータス</th>
          <td>{{ sensorData.status }}</td>
        </tr>
        <tr>
          <th>メッセージ</th>
          <td>{{ sensorData.message }}</td>
        </tr>
        <tr>
          <th>ホスト名</th>
          <td>{{ sensorData.hostname }}</td>
        </tr>
        <tr>
          <th>IPアドレス</th>
          <td>{{ sensorData.ipaddress }}</td>
        </tr>
      </tbody>
    </table>

    <h2>ログ</h2>
    <table class="table table-bordered table-striped">
      <thead class="thead-dark">
        <tr>
          <th>ログデータ</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="(log, index) in logs" :key="index">
          <td>{{ log }}</td>
        </tr>
      </tbody>
    </table>
  </div>
  <script>
    const { createApp } = Vue;
    createApp({
      data() {
        return {
          sensorData: {
            temperature: null,
            humidity:    null,
            heatIndex:   null,
            status:      null,
            message:     null,
            hostname:    null,
            ipaddress:   null
          },
          logs: []
        };
      },
      mounted() {

        //
        const ctx = document.getElementById('myChart').getContext('2d');
        this.chart = new Chart(ctx, {
          type: 'bar',
          data: {
            labels: ['温度', '湿度', '不快指数'],
            datasets: [
              {
                label: '温湿度センサー',
                backgroundColor: ['#42A5F5', '#66BB6A', '#FF7043'],
                data: [0, 0, 0], // 初期データ
              },
            ],
          },
          options: {
            responsive: true,
            maintainAspectRatio: false
          }
        });

        //
        const ws = new WebSocket('ws://' + location.hostname + ':80/ws');
        ws.onmessage = (event) => {
          const data = JSON.parse(event.data);
          Object.keys(this.sensorData).forEach(key => {
            this.sensorData[key] = data[key]; // 各要素のプロパティを更新
          });

          // 受信したデータをログに追加
          this.logs.push(event.data);

          // チャートのデータを更新
          this.chart.data.datasets[0].data = [
            this.sensorData.temperature,
            this.sensorData.humidity,
            this.sensorData.heatIndex
          ];

        };

      }

    }).mount("#app");
  </script>
</body>
</html>
)rawliteral";

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  // シリアル通信を115200ボーで開始(picocom -b 115200 /dev/ttyUSB0)
  Serial.begin(115200);

  // 起動画面の表示
  showSplash();

  //
  dht.begin();

  // WiFi接続
  connectToWiFi();

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // すべてのAsyncWebSocketイベントを処理
  ws.cleanupClients();

  // タスク処理
  displayInfoTask();

  // ホスト名の更新
  updateMdnsTask();

}

//----------------------------------------------------------------------------
// 起動画面の表示
//----------------------------------------------------------------------------
void showSplash(){

  // figlet ESP8266
  Serial.println("");
  Serial.println("");
  Serial.println("===============================================");
  Serial.println("  _____ ____  ____  ___ ____   __    __");
  Serial.println("  | ____/ ___||  _ \\( _ )___ \\ / /_  / /_  ");
  Serial.println("  |  _| \\___ \\| |_) / _ \\ __) | '_ \\| '_ \\ ");
  Serial.println("  | |___ ___) |  __/ (_) / __/| (_) | (_) |");
  Serial.println("  |_____|____/|_|   \\___/_____|\\___/ \\___/ ");
  Serial.println("");
  Serial.println("===============================================");

  // ボード名を表示
  Serial.print("Board         : ");
  Serial.println(ARDUINO_BOARD);

  // CPUの周波数を表示
  Serial.print("CPU Frequency : ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  // フラッシュサイズを表示
  Serial.print("Flash Size    : ");
  Serial.print(ESP.getFlashChipSize() / 1024);
  Serial.println(" KB");

  // 空きヒープメモリを表示
  Serial.print("Free Heap     : ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" B");

  // フラッシュ速度を取得
  Serial.print("Flash Speed   : ");
  Serial.print(ESP.getFlashChipSpeed() / 1000000);
  Serial.println(" MHz");

  // チップIDを取得
  Serial.print("Chip ID       : ");
  Serial.println(ESP.getChipId());

  // SDKバージョンを取得
  Serial.print("SDK Version   : ");
  Serial.println(ESP.getSdkVersion());

  Serial.println("===============================================");
  Serial.println("");

}

//----------------------------------------------------------------------------
// WiFi接続関数
//----------------------------------------------------------------------------
void connectToWiFi() {

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  Serial.print("Connected to ");
  Serial.println(ssid);

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
  Serial.println("");

}

//----------------------------------------------------------------------------
// Webサーバーの設定
//----------------------------------------------------------------------------
void setupWebServer() {

  //
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", htmlPage);
  });

  // WebSocketの設定

  // WebSocket接続のコールバック設定
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("Client disconnected");
    }
  });
  server.addHandler(&ws);

  // Webサーバーの開始
  server.begin();

}

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// センサーのデータを取得し、JSONで出力
void displayInfoTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= 1000) {
    lastTaskMillis = currentMillis;

    String jsonData = createJson();

    // WebSocketで送信
    ws.textAll(jsonData);

    // シリアルで出力
    Serial.println(jsonData);

  }

}

// 0.5秒ごとにホスト名を更新する関数
void updateMdnsTask() {

  static unsigned long lastMdnsMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMdnsMillis >= 500) {
    lastMdnsMillis = currentMillis;
    MDNS.update();
  }

}

//----------------------------------------------------------------------------
// 取得されるデータをJSON形式で生成
//----------------------------------------------------------------------------
String createJson() {

  StaticJsonDocument<256> doc;

  //
  float temperature     = dht.readTemperature();                // 温度
  float humidity        = dht.readHumidity();                   // 湿度
  float discomfortIndex = temperature + 0.36 * humidity + 41.2; // 不快指数の計算
  //if (!isnan(h) && !isnan(t)) {

  //
  doc["temperature"]     = temperature;               // 温度 (摂氏)
  doc["humidity"]        = humidity;                  // 湿度 (%)
  doc["discomfortIndex"] = discomfortIndex;           // 不快指数 (相対的な快適さを示す指標)
  doc["status"]          = 1;                         // ステータス (正常の場合は1)
  doc["message"]         = "正常に取得できました。";      // メッセージ (データ取得が成功したことを示す)
  doc["hostname"]        = hostname;                  // ホスト名 (デバイスの名前)
  doc["ipaddress"]       = WiFi.localIP().toString(); // IPアドレス (デバイスのネットワークアドレス)

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}
