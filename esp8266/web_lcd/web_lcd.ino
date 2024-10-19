//
// arduino-cli lib install "LiquidCrystal I2C"
// arduino-cli lib install "ArduinoJson"
//

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
const char* nikkeiUrl   = "https://query1.finance.yahoo.com/v8/finance/chart/^N225?interval=1d";
const char* exchangeUrl = "https://query1.finance.yahoo.com/v8/finance/chart/USDJPY=X?interval=1d";

ESP8266WebServer server(80);

//
unsigned long previousMillis = 0;
const long interval = 1800000;

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

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

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
  Serial.println("===============================================");

}

//----------------------------------------------------------------------------
// 日経平均株価の取得関数
//----------------------------------------------------------------------------
float fetchPrice(String url) {

  Serial.println("start fetch");
  Serial.println(url);

  //
  HTTPClient http;

  //
  WiFiClientSecure client;
  client.setInsecure();

  //
  http.setTimeout(1000 * 10);
  //http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36");
  http.addHeader("User-Agent", "ESP8266");
  http.begin(client, url);  // WiFiClientとURLを渡す
  http.setTimeout(1000 * 10);

  //
  //int httpCode = http.GET();
  int httpCode;
  int maxRetries = 3; // 最大リトライ回数
  int retries = 0;

  while (retries < maxRetries) {
      httpCode = http.GET();
      if (httpCode > 0) {
          break;  // 正常に通信できたらループを抜ける
      }
      retries++;
      delay(2000);  // 2秒待機して再試行
  }

  float price = 0;

  Serial.println(httpCode);
  Serial.println(http.getString());

  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    price = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"][0];
    Serial.println(price);
  } else {

    /// HTTP client errors
    /*
    HTTPC_ERROR_CONNECTION_REFUSED  (-1)
    HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
    HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
    HTTPC_ERROR_NOT_CONNECTED       (-4)
    HTTPC_ERROR_CONNECTION_LOST     (-5)
    HTTPC_ERROR_NO_STREAM           (-6)
    HTTPC_ERROR_NO_HTTP_SERVER      (-7)
    HTTPC_ERROR_TOO_LESS_RAM        (-8)
    HTTPC_ERROR_ENCODING            (-9)
    HTTPC_ERROR_STREAM_WRITE        (-10)
    HTTPC_ERROR_READ_TIMEOUT        (-11)
    */

    if (httpCode == -1) {
      Serial.println("Error: Connection refused (-1)");
    } else if (httpCode == -2) {
      Serial.println("Error: Send header failed (-2)");
    } else if (httpCode == -3) {
      Serial.println("Error: Send payload failed (-3)");
    } else if (httpCode == -4) {
      Serial.println("Error: Not connected (-4)");
    } else if (httpCode == -5) {
      Serial.println("Error: Connection lost (-5)");
    } else if (httpCode == -6) {
      Serial.println("Error: No stream (-6)");
    } else if (httpCode == -7) {
      Serial.println("Error: No HTTP server (-7)");
    } else if (httpCode == -8) {
      Serial.println("Error: Too less RAM (-8)");
    } else if (httpCode == -9) {
      Serial.println("Error: Encoding error (-9)");
    } else if (httpCode == -10) {
      Serial.println("Error: Stream write error (-10)");
    } else if (httpCode == -11) {
      Serial.println("Error: Read timeout (-11)");
    } else {
      Serial.printf("Error: Unknown error (%d)\n", httpCode);
    }

    lcd.clear();
    lcd.print("Error: ");
    lcd.print(httpCode); // エラーコードをLCDに表示

  }

  http.end();
  return price;

}

//----------------------------------------------------------------------------
// LCDにデータと変動率を表示する関数
//----------------------------------------------------------------------------

//
void fetchAndDisplayData() {

  if (WiFi.status() == WL_CONNECTED) {

    //
    nikkeiAverage = fetchPrice(nikkeiUrl);

    Serial.println("wait 10s");
    delay(1000 * 10 );

    //
    exchangeRate  = fetchPrice(exchangeUrl);

    displayData(nikkeiAverage, exchangeRate);

  }

}

//
void displayData(float nikkei, float nikkeiPrev) {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nikkei:");
  lcd.print(nikkei);

  lcd.setCursor(0, 1);
  lcd.print("USDJPY:");
  lcd.print(exchangeRate);

}

//----------------------------------------------------------------------------
// ルートパス処理関数 (WebアクセスでJSONを返す)
//----------------------------------------------------------------------------
void handleRoot() {

  String jsonResponse = getMarketDataJson();

  server.send(200, "application/json", jsonResponse);
  Serial.println(jsonResponse);

}

//----------------------------------------------------------------------------
// 日経平均株価と為替レートのデータをJSON形式で取得する関数
//----------------------------------------------------------------------------
String getMarketDataJson() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["nikkei"]    = nikkeiAverage;
  jsonDoc["usd_jpy"]   = exchangeRate;
  jsonDoc["hostname"]  = WiFi.hostname();
  jsonDoc["ipaddress"] = WiFi.localIP().toString();

  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  return jsonResponse;

}
