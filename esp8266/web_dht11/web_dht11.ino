/*****************************************************************************

# ESP8266ボードのインストール
arduino-cli config add-board manager.url http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266

# このプログラムで必要なライブラリのインストール
arduino-cli lib install "ESP8266WiFi"        # ESP8266ボード用のWiFi機能を提供するライブラリ
arduino-cli lib install "ESP8266mDNS"        # mDNS（マルチキャストDNS）を使用して、ESP8266デバイスをネットワークで簡単に見つけられるようにするライブラリ
arduino-cli lib install "ESP8266WebServer"   # ESP8266上でWebサーバー機能を実装するためのライブラリ
arduino-cli lib install "ArduinoJson"        # JSON形式のデータを簡単に作成、解析するためのライブラリ
arduino-cli lib install "DHT sensor library" # DHT11やDHT22温湿度センサー用のライブラリ

# コンパイルとアップロード例
bash upload_esp8266_web.sh web_ntp/web_ntp.ino wifissid wifipasswd hostname

*****************************************************************************/

//
#include <ESP8266WiFi.h>       // ESP8266用のWiFi機能を提供するライブラリ。WiFi接続やアクセスポイントの作成に使用します。
#include <ESP8266mDNS.h>       // mDNS（マルチキャストDNS）を使用するためのライブラリ。デバイスをネットワークで簡単に発見できるようにします。
#include <ESP8266WebServer.h>  // ESP8266デバイスでWebサーバーを構築するためのライブラリ。HTTPリクエストの処理やWebページの提供が可能です。
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

// Webサーバー設定
ESP8266WebServer server(80);

//----------------------------------------------------------------------------
// 初期実行
//----------------------------------------------------------------------------
void setup() {

  //
  Serial.begin(115200);

  //
  showSplash();

  // WiFi接続
  connectToWiFi();

  // ルートURLへのハンドラを設定
  server.on("/", handleRoot);

  // Webサーバーの開始
  server.begin();

}

//----------------------------------------------------------------------------
// ループ処理
//----------------------------------------------------------------------------
void loop() {

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  // 1秒ごとに情報を表示
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    Serial.println(createJson());
  }

  // クライアントリクエストを処理
  server.handleClient();

  // ホスト名の更新
  MDNS.update();

}

//----------------------------------------------------------------------------
// スプラッシュ画面の表示
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
  Serial.println("");

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
  Serial.println("");

}

//----------------------------------------------------------------------------
// Webサーバー系
//----------------------------------------------------------------------------

// ルートURLにアクセスした際の処理
void handleRoot() {

  // HTTPレスポンスを送信
  String jsonResponse = createJson();
  server.send(200, "application/json", jsonResponse);

}

// 取得されるデータをJSON形式で生成
String createJson() {

  // JSONオブジェクトを作成
  StaticJsonDocument<200> doc;
  doc["temperature"] = dht.readTemperature();     // 温度
  doc["humidity"]    = dht.readHumidity();        // 湿度
  doc["hostname"]    = hostname;                  // ホスト名
  doc["ipaddress"]   = WiFi.localIP().toString(); // IPアドレス

  // JSONデータを文字列にシリアライズ
  String json;
  serializeJson(doc, json);

  return json;

}
