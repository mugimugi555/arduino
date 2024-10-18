#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// WiFi SSIDとパスワードを指定
const char* ssid     = "WIFISSID";  // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME";   // ESP8266のホスト名

// LCDのI2Cアドレス（0x27または0x3Fが一般的）
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 日経平均株価のAPI URL
const char* nikkeiUrl = "https://query1.finance.yahoo.com/v8/finance/chart/^N225?interval=1d";
// ドル円の為替レートを取得するAPI URL
const char* exchangeUrl = "https://query1.finance.yahoo.com/v8/finance/chart/USDJPY=X?interval=1d";

void setup() {
    // シリアルモニタの初期化
    Serial.begin(115200);

    // LCDの初期化
    lcd.begin();
    lcd.backlight();
    lcd.print("Connecting...");

    // Wi-Fiに接続
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected!");
    lcd.clear();
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // 日経平均株価を取得
        http.begin(nikkeiUrl);
        int nikkeiCode = http.GET();
        float nikkeiAverage = 0;

        if (nikkeiCode > 0) {
            String nikkeiPayload = http.getString();
            Serial.println(nikkeiPayload);  // レスポンスをシリアルモニタに表示
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, nikkeiPayload);
            nikkeiAverage = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"][0];  // 日経平均株価を取得
        } else {
            Serial.println("Error on Nikkei HTTP request");
        }
        http.end();

        // ドル円の為替レートを取得
        http.begin(exchangeUrl);
        int exchangeCode = http.GET();
        float exchangeRate = 0;

        if (exchangeCode > 0) {
            String exchangePayload = http.getString();
            Serial.println(exchangePayload);  // レスポンスをシリアルモニタに表示
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, exchangePayload);
            exchangeRate = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"][0];  // ドル円の為替レートを取得
        } else {
            Serial.println("Error on Exchange HTTP request");
        }
        http.end();

        // LCDに表示
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Nikkei:");
        lcd.setCursor(0, 1);
        lcd.print(nikkeiAverage);
        lcd.setCursor(0, 2);
        lcd.print("1 USD:");
        lcd.setCursor(0, 3);
        lcd.print("¥");
        lcd.print(exchangeRate);
    }
    delay(60000); // 60秒ごとに価格を更新
}
