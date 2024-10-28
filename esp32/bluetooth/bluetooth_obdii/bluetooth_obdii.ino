/*
arduino-cli lib install "ELMduino"

EPS32 "4MB" not working :-b

*/

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ELMduino.h>
#include <BluetoothSerial.h>
#include <TinyGPS++.h>

BluetoothSerial SerialBT;
ELM327 myELM327;
TinyGPSPlus gps;

// Wi-Fiの設定
const char* ssid = "Your_SSID";  // Wi-FiのSSID
const char* password = "Your_PASSWORD";  // Wi-Fiのパスワード

// CSVデータを保存するファイルパス
const char* csvFilePath = "/obd_data.csv";

unsigned long lastWiFiCheck = 0;  // 最後にWi-Fiをチェックした時刻
const unsigned long wifiCheckInterval = 60000; // Wi-Fiチェックの間隔（1分）

WebServer server(80);

String datetime;

void setup() {
  Serial.begin(115200);

  // LittleFSの初期化
  if (!LittleFS.begin()) {
    Serial.println("LittleFSの初期化に失敗しました！");
    return;
  }

  // BluetoothとELM327の初期化
  SerialBT.begin("ESP32_OBD");
  if (SerialBT.connect("OBDII")) {
    Serial.println("ELM327に接続しました。");
    if (myELM327.begin(SerialBT, true, 2000)) {
      Serial.println("ELM327の初期化に成功しました。");
    }
  } else {
    Serial.println("ELM327への接続に失敗しました。");
  }

  // CSVファイルの初期化とヘッダーの書き込み
  initializeCSV();
}

void loop() {

  // 時間をチェックしてWi-Fi接続を確認
  unsigned long currentMillis = millis();
  if (currentMillis - lastWiFiCheck >= wifiCheckInterval) {
    connectToWiFi();
    lastWiFiCheck = currentMillis;  // 最後のチェック時刻を更新
  }

  // GPSデータの更新
  while (Serial.available()) {
    gps.encode(Serial.read());
  }

  // GPS情報の取得
  if (gps.location.isUpdated()) {
    datetime  = String(gps.date.year())        + "-" + String(gps.date.month())  + "-" + String(gps.date.day()) + " " +
                String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
  }

  // OBD-IIデータを取得しCSVに追加
  appendCSV(
    getOBDData("010C"), // RPM
    getOBDData("010D"), // Speed
    getOBDData("0104"), // Engine Load
    getOBDData("0105"), // Coolant Temp
    getOBDData("010F"), // Intake Temp
    getOBDData("0111"), // Throttle Position
    getOBDData("012F"), // Fuel Level
    getOBDData("0133"), // Engine Time
    getOBDData("0146"), // Ambient Temp
    getOBDData("015E"), // Fuel Rate
    getOBDData("010B"),  // Fuel Pressure
    gps.location.lat(),
    gps.location.lng(),
    gps.altitude.meters(),
    datetime
  );

  // Webサーバーのリクエストを処理
  server.handleClient();
  delay(500); // 次のイテレーションまで0.5秒待つ

}

// Wi-Fiに接続する関数
void connectToWiFi() {
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fiに接続しました。");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/download", HTTP_GET, handleDownload);
    server.begin();
  } else {
    Serial.println("Wi-Fiへの接続に失敗しました。");
  }
}

// OBD-IIデータを特定のPIDから取得する関数
String getOBDData(String pid) {
  myELM327.sendCommand(pid.c_str());
  String response = "";
  while (SerialBT.available()) {
    response += char(SerialBT.read());
  }
  return response;
}

// CSVファイルを初期化し、ヘッダーを書き込む関数
void initializeCSV() {
  File file = LittleFS.open(csvFilePath, FILE_WRITE);
  if (file) {
    //file.println("Timestamp,RPM,Speed,EngineLoad,CoolantTemp,IntakeTemp,ThrottlePosition,FuelLevel,EngineTime,AmbientTemp,FuelRate,FuelPressure,Latitude,Longitude,Altitude,Datetime");
    file.close();
  }
}

// データをCSVファイルに追加する関数
void appendCSV(String rpm, String speed, String engineLoad, String coolantTemp, String intakeTemp, String throttlePosition, String fuelLevel, String engineTime, String ambientTemp, String fuelRate, String fuelPressure, float latitude, float longitude, float altitude, String datetime) {
  File file = LittleFS.open(csvFilePath, FILE_APPEND);
  if (file) {
    String data = String(millis())   + ","
                  + rpm              + ","
                  + speed            + ","
                  + engineLoad       + ","
                  + coolantTemp      + ","
                  + intakeTemp       + ","
                  + throttlePosition + ","
                  + fuelLevel        + ","
                  + engineTime       + ","
                  + ambientTemp      + ","
                  + fuelRate         + ","
                  + latitude         + ","
                  + longitude        + ","
                  + altitude         + ","
                  + datetime;
    file.println(data); // データの追加
    file.close();
  }
}

// ダウンロードエンドポイントを処理し、CSVファイルを提供する関数
void handleDownload() {
  File file = LittleFS.open(csvFilePath, FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "ファイルを開けませんでした");
    return;
  }
  server.streamFile(file, "text/csv"); // CSVファイルをストリーム配信
  file.close();
}
