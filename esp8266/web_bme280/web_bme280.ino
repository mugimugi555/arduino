#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h> // ESP8266用のWi-Fiライブラリ
#include <ESP8266WebServer.h> // ESP8266用のWebサーバーライブラリ

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ;
const char* password = "WIFIPASSWD";
const char* hostname = "HOSTNAME"  ;

// BME280オブジェクトの作成
Adafruit_BME280 bme;

// Webサーバーのポート番号
ESP8266WebServer server(80);

//
void setup() {
  Serial.begin(115200);
  
  //
  connectToWiFi();

  // BME280センサーの初期化
  if (!bme.begin(0x76)) {
    Serial.println("BME280センサーが見つかりません。接続を確認してください。");
    while (1);
  }

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

//
void loop() {
  server.handleClient(); // クライアントからのリクエストを処理
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

// ルートURLへのリクエストハンドラ
void handleRoot() {
  // 温度、湿度、気圧の値を取得
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F; // PaをhPaに変換

  // JSON形式のレスポンスを作成
  String message = String("{\"temperature\":") + temperature + 
                   String(",\"humidity\":")    + humidity    + 
                   String(",\"pressure\":")    + pressure    + "}";

  // レスポンスをクライアントに送信
  server.send(200, "application/json", message);
}
