#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>      // ESP8266用のWi-Fiライブラリ
#include <ESP8266WebServer.h> // ESP8266用のWebサーバーライブラリ
#include <ESP8266mDNS.h>      // ESP8266用のmDNSライブラリ

// WiFi SSIDとパスワードをホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

// BME280オブジェクトの作成
Adafruit_BME280 bme;

// Webサーバーのポート番号
ESP8266WebServer server(80);

// ESP01とBME280センサーの接続:
// SCL (クロック信号) -> GPIO0 (D3ピン)
// SDA (データ信号) -> GPIO2 (D4ピン)
// VCC -> 3.3V
// GND -> GND
  
// I2Cのピン設定
#define I2C_SCL 0  // GPIO0 (D3ピン)
#define I2C_SDA 2  // GPIO2 (D4ピン)

void setup() {

  Serial.begin(115200);

  // I2Cピンの設定
  Wire.begin(I2C_SDA, I2C_SCL);

  // WiFi接続の設定
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

  // mDNSサービスの開始
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

}

void loop() {

  server.handleClient(); // クライアントからのリクエストを処理
  MDNS.update();  // mDNSサービスの更新

}

void connectToWiFi() {

  WiFi.hostname(hostname); // ホスト名を設定
  WiFi.begin(ssid, password); // Wi-Fi接続を開始

  // Wi-Fi接続が完了するまで待つ
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // ESP8266のIPアドレスを表示
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // サブネットマスク、ゲートウェイ、DNS、MACアドレスを表示
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

// ルートURLへのリクエストハンドラ
void handleRoot() {

  // 温度、湿度、気圧の値を取得
  float temperature = bme.readTemperature();
  float humidity    = bme.readHumidity();
  float pressure    = bme.readPressure() / 100.0F; // PaをhPaに変換

  // JSON形式のレスポンスを作成
  String message = String("{\"temperature\":") + temperature + 
                   String(",\"humidity\":")    + humidity    + 
                   String(",\"pressure\":")    + pressure    + "}";

  // レスポンスをクライアントに送信
  server.send(200, "application/json", message);

}
