//
// arduino-cli lib install "LiquidCrystal I2C"
// arduino-cli lib install "ArduinoJson"

//
// [ESP8266] <---> [LCD]
// 3.3V <--------> VCC
// GND <---------> GND
// GPIO 4 (D2) <-> SDA
// GPIO 5 (D1) <-> SCL

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワード、ホスト名を指定
const char* ssid     = "WIFISSID"  ; // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME"  ; // ESP-01のホスト名

//
LiquidCrystal_I2C lcd(0x27, 16, 2);

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

//
const char* tickerNikkei   = "^N225";
const char* tickerExchange = "USDJPY=X";

ESP8266WebServer server(80);

//
unsigned long previousMillis = 0;
const long interval = 1000 * 60 * 60; //１時間ごとに更新

//
float nikkeiAverage = 0;
float exchangeRate  = 0;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);

  lcd.init();
  lcd.begin(16, 2); // 16カラム、2行のLCDディスプレイの場合
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting...");

  connectToWiFi();
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("WiFi Connected");

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  //
  fetchAndDisplayData();

}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void loop() {

  //
  server.handleClient();

  //
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    fetchAndDisplayData();
  }

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

//----------------------------------------------------------------------------
// 株価の取得関数
//----------------------------------------------------------------------------

// URLエンコード関数
String urlEncode(String str) {

  String encodedString = "";

  char c;
  char buf[3];
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);

    // URLで使用できない文字をエンコードする
    if (isalnum(c)) {
      encodedString += c;  // 英数字はそのまま
    } else {
      sprintf(buf, "%%%02X", c);  // %XXの形式でエンコード
      encodedString += buf;
    }

  }

  return encodedString;

}

// api から取得
float fetchPrice(String ticker) {

  float price = 0;

  Serial.print("Start fetch ");
  Serial.println(ticker);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start fetch ");
  lcd.setCursor(0, 1);
  lcd.print(ticker);

  WiFiClientSecure client;
  client.setInsecure();  // セキュリティ証明書の検証を無効化

  // サーバーに接続
  if (!client.connect("query1.finance.yahoo.com", 443)) {
    Serial.println("Connection failed");
    return 0;
  }

  // リクエスト送信
  //https://query1.finance.yahoo.com/v8/finance/chart/^N225?interval=1d
  client.print("GET /v8/finance/chart/");
  client.print( urlEncode( ticker ) );
  client.println("?interval=1d HTTP/1.1");

  client.println("Host: query1.finance.yahoo.com");
  //client.println("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8");
  client.println("Accept: application/json");
  client.println("Accept-Language: ja;q=0.7");
  client.println("Cache-Control: max-age=0");
  client.println("Priority: u=0, i");
  client.println("Sec-CH-UA: \"Chromium\";v=\"130\", \"Brave\";v=\"130\", \"Not?A_Brand\";v=\"99\"");
  client.println("Sec-CH-UA-Mobile: ?0");
  client.println("Sec-CH-UA-Platform: \"Linux\"");
  client.println("Sec-Fetch-Dest: document");
  client.println("Sec-Fetch-Mode: navigate");
  client.println("Sec-Fetch-Site: none");
  client.println("Sec-Fetch-User: ?1");
  client.println("Sec-GPC: 1");
  client.println("Upgrade-Insecure-Requests: 1");
  client.println("User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36");
  client.println();

  // サーバーからのレスポンスを読み込む
  String searchStr = "regularMarketPrice\":";
  while (client.connected() || client.available()) {

    // データが利用可能な場合は読み取る
    if (client.available()) {

      String line = client.readStringUntil('\n');  // 一行ずつ読み込む

      // 探したい文字列の開始位置を取得
      int startIndex = line.indexOf(searchStr);
      if (startIndex != -1) {

        // 開始位置に探した文字列の長さを加算して、値の開始位置を取得
        startIndex += searchStr.length();

        // カンマの位置を探して、値の終了位置を取得
        int endIndex = line.indexOf(",", startIndex);

        // 値部分を切り出して浮動小数点数に変換
        String valueStr = line.substring(startIndex, endIndex);
        price = valueStr.toFloat();
        break;

      }

    }

  }

  client.stop();

  Serial.println(price);

  return price;

}

//----------------------------------------------------------------------------
// LCDにデータと変動率を表示する関数
//----------------------------------------------------------------------------

//
void fetchAndDisplayData() {

  if (WiFi.status() == WL_CONNECTED) {

    //
    nikkeiAverage = fetchPrice(tickerNikkei);
    exchangeRate  = fetchPrice(tickerExchange);
    displayData(nikkeiAverage, exchangeRate);

  }

}

//
void displayData(float nikkei, float nikkeiPrev) {

  //
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nikkei:");
  lcd.print(nikkei);

  //
  lcd.setCursor(0, 1);
  lcd.print("USDJPY:");
  lcd.print(exchangeRate);

}

//----------------------------------------------------------------------------
// ルートパス処理関数 (WebアクセスでJSONを返す)
//----------------------------------------------------------------------------
void handleRoot() {

  String jsonResponse = getMarketDataJson();

  //
  server.send(200, "application/json", jsonResponse);
  Serial.println(jsonResponse);

}

//----------------------------------------------------------------------------
// 日経平均株価と為替レートのデータをJSON形式で取得する関数
//----------------------------------------------------------------------------
String getMarketDataJson() {

  //
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["nikkei"]    = nikkeiAverage;
  jsonDoc["usd_jpy"]   = exchangeRate;
  jsonDoc["hostname"]  = WiFi.hostname();
  jsonDoc["ipaddress"] = WiFi.localIP().toString();

  //
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  return jsonResponse;

}
