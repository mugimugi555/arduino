/*
arduino-cli lib install "ELMduino"

EPS32 "4MB" not working :-b

*/

#include <LittleFS.h>
#include <ELMduino.h>
#include <BluetoothSerial.h>
#include <TinyGPS++.h>

BluetoothSerial SerialBT;
ELM327 myELM327;
TinyGPSPlus gps;

// CSVデータを保存するファイルパス
const char* csvFilePath = "/obd_data.csv";

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
  // GPSデータの更新
  while (Serial.available()) {
    gps.encode(Serial.read());
  }

  // GPS情報の取得
  String datetime;
  float latitude = 0.0;
  float longitude = 0.0;
  float altitude = 0.0;

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    altitude = gps.altitude.meters();
    datetime = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) + " " +
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
    latitude,
    longitude,
    altitude,
    datetime
  );

  // 特定のコマンドを受信した場合にCSVファイルを送信
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.equals("SEND_FILE")) {
      sendFile(csvFilePath);
    }
  }

  delay(500); // 次のイテレーションまで0.5秒待つ
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
    file.println("Timestamp,RPM,Speed,EngineLoad,CoolantTemp,IntakeTemp,ThrottlePosition,FuelLevel,EngineTime,AmbientTemp,FuelRate,FuelPressure,Latitude,Longitude,Altitude,Datetime"); // ヘッダーの書き込み
    file.close();
  }
}

// データをCSVファイルに追加する関数
void appendCSV(String rpm, String speed, String engineLoad, String coolantTemp, String intakeTemp, String throttlePosition, String fuelLevel, String engineTime, String ambientTemp, String fuelRate, String fuelPressure, float latitude, float longitude, float altitude, String datetime) {
  File file = LittleFS.open(csvFilePath, FILE_APPEND);
  if (file) {
    String data = String(millis()) + ","
                  + rpm + ","
                  + speed + ","
                  + engineLoad + ","
                  + coolantTemp + ","
                  + intakeTemp + ","
                  + throttlePosition + ","
                  + fuelLevel + ","
                  + engineTime + ","
                  + ambientTemp + ","
                  + fuelRate + ","
                  + latitude + ","
                  + longitude + ","
                  + altitude + ","
                  + datetime;
    file.println(data); // データの追加
    file.close();
  }
}

// ファイルをシリアル通信で送信する関数
void sendFile(const char* path) {
  File file = LittleFS.open(path, FILE_READ);
  if (file) {
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("ファイルを開けませんでした");
  }
}
