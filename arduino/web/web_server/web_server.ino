#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoWebsockets.h>
#include <SD.h>

using namespace websockets;
WebsocketsServer server;

// EthernetおよびSDカードの設定
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MACアドレス
EthernetServer server(80);
WebsocketsServer webSocket; // WebSocketサーバー
File logFile;

// センサーの値を保存する変数
float pressureValue = 0.0;

// センサーの値を記録する間隔（ミリ秒）
const unsigned long logInterval = 60000; // 1分
unsigned long lastLogTime = 0;

void setup() {
  // シリアルモニタの開始
  Serial.begin(9600);

  // Ethernetの開始
  Ethernet.begin(mac);
  server.begin();
  Serial.println("Server is ready");

  // WebSocketの開始
  webSocket.listen(81); // WebSocketはポート81でリッスン
  Serial.println("WebSocket server is ready");

  // SDカードの初期化
  if (!SD.begin(4)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  // ログファイルを開く
  logFile = SD.open("log.txt", FILE_WRITE);
  if (!logFile) {
    Serial.println("Failed to open log file");
    return;
  }
}

void loop() {
  // センサーの値を取得
  getSensorValue(); // ここにセンサーから値を取得するコードを書く

  // 現在の時刻でセンサーの値を記録
  unsigned long currentMillis = millis();
  if (currentMillis - lastLogTime >= logInterval) {
    logSensorValue();
    lastLogTime = currentMillis;
  }


  // 接続されているクライアントのメッセージを受信
  WebsocketsClient client = server.available();

  if (client.connected()) {
    String msg = client.read();

    if (msg.length() > 0) {
      Serial.println("Message received: " + msg);
      client.send("Message received"); // クライアントに返答
    }
  }

  // WebSocketでクライアントに値を送信
  if (server.connectedClients()) {
    String json = createJson();
    server.broadcast(json); // JSON形式で全クライアントに送信
  }

  // サーバーをループ
  server.loop();

  // クライアントの接続をチェック
  EthernetClient client = server.available();
  if (client) {
    handleClient(client);
  }
}

void getSensorValue() {
  // センサーの値をここで取得
  // 例: pressureValue = readPressureSensor();
}

void logSensorValue() {
  if (logFile) {
    logFile.print("Pressure: ");
    logFile.println(pressureValue);
    logFile.flush(); // 書き込みを即座に反映
  }
}

String createJson() {
  // JSON形式の文字列を作成
  String json = "{";
  json += "\"pressure\": " + String(pressureValue);
  json += "}";
  return json;
}

void handleClient(EthernetClient client) {
  // HTTPリクエストの読み込み
  String currentLine = "";
  while (client.connected() && !client.available()) {
    delay(1);
  }

  // リクエストを読み込む
  while (client.available()) {
    char c = client.read();
    if (c == '\n') {
      // リクエストが終わった場合
      if (currentLine.length() == 0) {
        // リクエストが / であればHTMLファイルを返す
        if (client.readStringUntil(' ').equals("GET / ")) {
          sendHtmlResponse(client);
        } else if (client.readStringUntil(' ').equals("GET /json")) {
          sendJsonResponse(client);
        }
        break;
      } else {
        currentLine = "";
      }
    } else {
      currentLine += c;
    }
  }

  // クライアントを切断
  client.stop();
}

void sendHtmlResponse(EthernetClient client) {
  // HTMLレスポンスを返す
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h1>Pressure Sensor Data</h1>");
  client.println("<p>Current Pressure: ");
  client.print(pressureValue);
  client.println("</p>");
  client.println("<script>");
  client.println("var connection = new WebSocket('ws://' + window.location.hostname + ':81');");
  client.println("connection.onmessage = function (event) {");
  client.println("  document.getElementById('pressure').innerHTML = event.data;");
  client.println("};");
  client.println("</script>");
  client.println("<div id='pressure'></div>");
  client.println("</html>");
}

void sendJsonResponse(EthernetClient client) {
  // JSONレスポンスを返す
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println();
  client.print(createJson());
}
