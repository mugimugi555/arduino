#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
WiFiServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

TinyGPSPlus gps;
SoftwareSerial ss(D1, D2);  // D1=RX, D2=TX

void setup() {
  Serial.begin(115200);
  ss.begin(9600);  // GPSモジュールのボーレートに合わせる

  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Webサーバー、WebSocketサーバーの開始
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);  // Webクライアントを処理
  }

  webSocket.loop();  // WebSocketを処理

  // GPSデータの処理
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      sendGpsData();
    }
  }
}

// Webクライアントの処理
void handleClient(WiFiClient client) {
  String header;
  while (client.connected()) {
    if (client.available()) {
      String request = client.readStringUntil('\r');
      client.flush();

      // ルートアクセス時にJSONデータを表示
      if (request.indexOf("GET / ") >= 0) {
        String json = createJson();
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println();
        client.println(json);
        break;
      }
    }
  }
  client.stop();
}

// WebSocketイベントの処理
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    webSocket.broadcastTXT("WebSocket Connected");
  }
}

// GPSデータをJSON形式で生成
String createJson() {
  StaticJsonDocument<256> doc;
  doc["latitude"] = gps.location.lat();
  doc["longitude"] = gps.location.lng();
  doc["altitude"] = gps.altitude.meters();
  doc["speed"] = gps.speed.kmph();
  doc["satellites"] = gps.satellites.value();
  doc["date"] = String(gps.date.month()) + "/" + String(gps.date.day()) + "/" + String(gps.date.year());
  doc["time"] = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());

  String json;
  serializeJson(doc, json);
  return json;
}

// GPSデータをWebSocketで送信
void sendGpsData() {
  String json = createJson();
  webSocket.broadcastTXT(json);  // WebSocketでJSONを送信
}
