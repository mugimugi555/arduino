/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
arduino-cli lib install "ESP8266mDNS"        # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ。
arduino-cli lib install "ESPAsyncWebServer"
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

//
#define DHTPIN 2      // DHTセンサーを接続するピン（GPIO2: D4ピン）
#define DHTTYPE DHT11 // DHTセンサーの種類（DHT11 または DHT22）
DHT dht(DHTPIN, DHTTYPE);

// タスクを繰り返し実行する間隔（秒）
const long taskInterval = 1;

// ポート80で非同期Webサーバーを初期化
AsyncWebServer server(80);

// WebSocketサーバー設定
AsyncWebSocket ws("/ws");

// TCPソケット設定
WiFiServer tcpServer(12345);  // TCPサーバーをポート12345で初期化
WiFiClient tcpClient;

//----------------------------------------------------------------------------
// HTMLコンテンツ（Vanilla(素の) JavaScriptを使用）
//----------------------------------------------------------------------------
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta charset="UTF-8">
  <title>温湿度リアルタイム表示</title>
  <style>
    table {
      width: 100%;
      border-collapse: collapse; /* セルの境界を重ねて表示 */
      margin: 20px 0; /* 上下のマージン */
      font-family: Arial, sans-serif; /* フォントファミリーの設定 */
    }
    th, td {
      padding: 12px; /* セルの内側の余白 */
      text-align: left; /* 左寄せ */
      border: 1px solid #ddd; /* セルの境界線 */
    }
    tr:nth-child(even) {
      background-color: #f2f2f2; /* 偶数行の背景色 */
    }
    tr:hover {
      background-color: #ddd; /* ホバー時の背景色 */
    }
    th {
      width:90px;
      text-align: center;
      white-space: nowrap; /* テキストが折り返されないようにする */
      background-color: #696969; /* ヘッダーの背景色 */
      color: white; /* ヘッダーの文字色 */
    }
    .data {
      color: #333; /* データの文字色 */
    }
  </style>
</head>
<body>
  <h1>温湿度センサー(WebSocket)</h1>
  <table>
    <tr>
      <th>温度</th>
      <td><span id="temperature" class="data">--</span> ℃</td>
    </tr>
    <tr>
      <th>湿度</th>
      <td><span id="humidity" class="data">--</span> %</td>
    </tr>
    <tr>
      <th>不快指数</th>
      <td><span id="discomfortIndex" class="data">--</span></td>
    </tr>
    <tr>
      <th>ステータス</th>
      <td><span id="status" class="data">--</span></td>
    </tr>
    <tr>
      <th>メッセージ</th>
      <td><span id="message" class="data">--</span></td>
    </tr>
    <tr>
      <th>ホスト名</th>
      <td><span id="hostname" class="data">--</span></td>
    </tr>
    <tr>
      <th>IPアドレス</th>
      <td><span id="ipaddress" class="data">--</span></td>
    </tr>
    <tr>
      <th>受信データ</th>
      <td><span id="log" class="data">--</span></td>
    </tr>
  </table>
  <script>
    document.addEventListener("DOMContentLoaded", function() {

      // HTML要素のIDを配列に格納
      const elements = [
        { id: 'temperature',     key: 'temperature'     },
        { id: 'humidity',        key: 'humidity'        },
        { id: 'discomfortIndex', key: 'discomfortIndex' },
        { id: 'status',          key: 'status'          },
        { id: 'message',         key: 'message'         },
        { id: 'hostname',        key: 'hostname'        },
        { id: 'ipaddress',       key: 'ipaddress'       }
      ];

      // WebSocket接続を確立
      const socket = new WebSocket(`ws://${location.hostname}/ws`);

      // データを受信したときの処理
      socket.onmessage = function(event) {
        const data = JSON.parse(event.data);

        // log
        console.log(data);
        const logElem = document.getElementById("log");
        if (logElem) {
          log.textContent = event.data;
        }

        // 配列をループして各要素にデータを設定
        elements.forEach(element => {
          const elem = document.getElementById(element.id);
          if (elem) {
            elem.textContent = data[element.key]; // keyに基づいてdataから値を取得
          }
        });
      };

      //
      socket.onopen = function() {
        console.log('WebSocket 接続成功');
      };

      //
      socket.onerror = function(error) {
        console.log('WebSocket エラー', error);
      };

    });
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

  // DHTセンサーの初期化
  dht.begin();

  // 起動画面の表示
  showStartup();

  // WiFi接続
  connectToWiFi();

  // Webサーバーの開始
  setupWebServer();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  // タスク処理
  fetchAndShowTask();

  // ホスト名の更新
  updateMdnsTask();

  ws.cleanupClients();  // クライアントの管理

  //
  handleTCPClient();

}

//----------------------------------------------------------------------------
// 起動画面の表示
//----------------------------------------------------------------------------
void showStartup() {

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

  // WebSocketサーバー初期化
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // ルートへのアクセス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", htmlPage);
  });

  // Webサーバーを開始
  server.begin();

  // TCPソケットサーバー開始
  tcpServer.begin();

}

//
void sendMessage() {

  //
  String jsonResponse = createJson();

  // WebSocketで配信
  ws.textAll(jsonResponse);

  // Socketで配信
  tcpClient.println(jsonResponse);

  //
  Serial.println(jsonResponse);

}

// WebSocketのイベント群
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {

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

// クライアントからのメッセージ処理
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {

  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // クライアントからのメッセージ処理（必要に応じて）
  }

}

// TCPクライアントの接続確認とデータ送信
void handleTCPClient() {

  // 新しいTCPクライアントの接続を確認
  if (tcpServer.hasClient()) {
    if (!tcpClient || !tcpClient.connected()) {
      tcpClient = tcpServer.available();
      Serial.println("New TCP client connected");
    } else {
      tcpServer.available().stop();
      Serial.println("Rejected new client");
    }
  }

}

//----------------------------------------------------------------------------
// 取得されるデータをJSON形式で生成
//----------------------------------------------------------------------------
String createJson() {

  StaticJsonDocument<256> doc;

  // センサーからのデータを取得
  float temperature     = dht.readTemperature();                // 温度
  float humidity        = dht.readHumidity();                   // 湿度
  float discomfortIndex = temperature + 0.36 * humidity + 41.2; // 不快指数の計算

  // JSONオブジェクトの作成
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

//----------------------------------------------------------------------------
// タスク処理
//----------------------------------------------------------------------------

// taskInterval 秒ごとに情報を表示する関数
void fetchAndShowTask() {

  static unsigned long lastTaskMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastTaskMillis >= taskInterval * 1000) {
    lastTaskMillis = currentMillis;
    sendMessage();
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
