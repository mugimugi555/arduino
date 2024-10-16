#include <WiFi.h>
#include <ArduinoJson.h>

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;

// MQセンサーのアナログピンの定義
const int MQ2_PIN = 34; // MQ-2 センサー LPG、ブタン、プロパン、煙、アルコールなど
const int MQ3_PIN = 35; // MQ-3 センサー アルコール蒸気（主に飲酒運転検知に使用）
const int MQ4_PIN = 32; // MQ-4 センサー メタン、天然ガス
const int MQ5_PIN = 33; // MQ-5 センサー LPG、メタン、酒精
const int MQ6_PIN = 25; // MQ-6 センサー LPG、ブタン、プロパン、アルコール
const int MQ7_PIN = 26; // MQ-7 センサー 一酸化炭素（CO）
const int MQ8_PIN = 27; // MQ-8 センサー 水素（H2）
const int MQ9_PIN = 14; // MQ-9 センサー 一酸化炭素（CO）および可燃性ガス

// Webサーバーの設定
WiFiServer server(80);

void setup() {
  Serial.begin(115200); // シリアル通信の初期化

  // WiFiに接続
  connectToWiFi();

  // サーバーを開始
  server.begin();
}

void loop() {
  // クライアントが接続しているか確認
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    // HTTPリクエストを処理
    if (request.indexOf("GET /") != -1) {
      // MQセンサーの値を読み取る
      int mq2Value = analogRead(MQ2_PIN);
      int mq3Value = analogRead(MQ3_PIN);
      int mq4Value = analogRead(MQ4_PIN);
      int mq5Value = analogRead(MQ5_PIN);
      int mq6Value = analogRead(MQ6_PIN);
      int mq7Value = analogRead(MQ7_PIN);
      int mq8Value = analogRead(MQ8_PIN);
      int mq9Value = analogRead(MQ9_PIN);

      // JSON形式でデータを作成
      StaticJsonDocument<300> doc;
      doc["MQ-2"] = mq2Value;
      doc["MQ-3"] = mq3Value;
      doc["MQ-4"] = mq4Value;
      doc["MQ-5"] = mq5Value;
      doc["MQ-6"] = mq6Value;
      doc["MQ-7"] = mq7Value;
      doc["MQ-8"] = mq8Value;
      doc["MQ-9"] = mq9Value;

      String jsonResponse;
      serializeJson(doc, jsonResponse);

      // シリアル出力をCSV形式で行う
      Serial.print(mq2Value); Serial.print(",");
      Serial.print(mq3Value); Serial.print(",");
      Serial.print(mq4Value); Serial.print(",");
      Serial.print(mq5Value); Serial.print(",");
      Serial.print(mq6Value); Serial.print(",");
      Serial.print(mq7Value); Serial.print(",");
      Serial.print(mq8Value); Serial.print(",");
      Serial.println(mq9Value);

      // HTTPレスポンスを送信
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(jsonResponse);
    }
    // クライアントを切断
    client.stop();
  }
}

//
void connectToWiFi() {

  // Connect to WiFi
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // Display the ESP32's hostname
  Serial.print("Hostname: http://");
  Serial.print(WiFi.getHostname());
  Serial.println(".local");

  // Display the ESP32's IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display subnet mask
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  // Display gateway IP
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());

  // Display DNS server IP
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  // Display the ESP32's MAC address
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Start mDNS responder
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

}
