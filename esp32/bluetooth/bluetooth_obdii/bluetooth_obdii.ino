/*
arduino-cli lib install "ELMduino"
*/

#include <LittleFS.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>
#include <TinyGPS++.h>

BluetoothSerial SerialBT;
ELM327 myELM327;
TinyGPSPlus gps;

// CSVデータを保存するファイルパス
const char* csvFilePath = "/obd_data.csv";

unsigned long lastWiFiCheck = 0;  // 最後にWi-Fiをチェックした時刻
const unsigned long wifiCheckInterval = 60000; // Wi-Fiチェックの間隔（1分）

String datetime;
String previousSpeed = "0"; // 前回の速度を保持する変数
String previousRPM = "0"; // 前回の回転数を保持する変数
unsigned long lastSpeedCheckTime = 0; // 最後の速度チェック時間
bool gpsAvailable = false; // GPSセンサーの有無フラグ
bool absActive = false; // ABSの有無フラグ

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
  if (gpsAvailable) {
    while (Serial.available()) {
      gps.encode(Serial.read());
    }
  }

  // GPS情報の取得
  if (gpsAvailable && gps.location.isUpdated()) {
    datetime = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) + " " +
               String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
  }

  // OBD-IIデータを取得
  String rpm = getOBDData("010C");
  String speed = getOBDData("010D");
  String engineLoad = getOBDData("0104");
  String coolantTemp = getOBDData("0105");
  String intakeTemp = getOBDData("010F");
  String throttlePosition = getOBDData("0111");
  String fuelLevel = getOBDData("012F");
  String engineTime = getOBDData("0133");
  String ambientTemp = getOBDData("0146");
  String fuelRate = getOBDData("015E");
  String fuelPressure = getOBDData("010B");

  // ABSとスリップの状態を取得
  absActive = checkABSStatus();

  // 急ブレーキの判定
  bool emergencyBraking = isEmergencyBraking(speed);

  // ABSが発動していて速度が落ちない場合、スリップしている可能性を警告
  if (absActive && speed.toFloat() >= previousSpeed.toFloat()) {
    Serial.println("ABSが発動中で速度が落ちない可能性があります。雪道でスリップしているかもしれません。");
  }

  // タイヤの回転数が高く、速度がゼロの場合のチェック
  checkHighRPMAndNoMovement(rpm, speed);

  // CSVにデータを追加
  appendCSV(datetime,
             gpsAvailable ? gps.location.lat() : 0.0,
             gpsAvailable ? gps.location.lng() : 0.0,
             gpsAvailable ? gps.altitude.meters() : 0.0,
             rpm, speed, engineLoad, coolantTemp, intakeTemp, throttlePosition,
             fuelLevel, engineTime, ambientTemp, fuelRate, fuelPressure,
             absActive ? "On" : "Off", emergencyBraking ? "Slip" : "No Slip");

  previousSpeed = speed; // 現在の速度を保存
  previousRPM = rpm; // 現在の回転数を保存
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
    file.println("Datetime,Latitude,Longitude,Altitude,RPM,Speed,EngineLoad,CoolantTemp,IntakeTemp,ThrottlePosition,FuelLevel,EngineTime,AmbientTemp,FuelRate,FuelPressure,ABSStatus,SlipStatus");
    file.close();
  }
}

// データをCSVファイルに追加する関数
void appendCSV(String datetime, float latitude, float longitude, float altitude,
                String rpm, String speed, String engineLoad, String coolantTemp, String intakeTemp,
                String throttlePosition, String fuelLevel, String engineTime, String ambientTemp,
                String fuelRate, String fuelPressure, String absStatusStr, String slipStatus) {
  File file = LittleFS.open(csvFilePath, FILE_APPEND);
  if (file) {
    String data = datetime + ","
                  + latitude + ","
                  + longitude + ","
                  + altitude + ","
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
                  + fuelPressure + ","
                  + absStatusStr + ","
                  + slipStatus;
    file.println(data); // データの追加
    file.close();
  }
}

// ABSの状態をチェックする関数（仮定）
bool checkABSStatus() {
  // 実際の車両に応じたPIDを使用してください
  String absData = getOBDData("0101"); // ABSのPIDを仮定
  return absData.indexOf("ABS") != -1; // ABSが発動中かを判断（実際のレスポンスに応じて修正）
}

// 急ブレーキ判定の関数
bool isEmergencyBraking(String currentSpeed) {
  unsigned long currentTime = millis();

  // 一定時間間隔で速度をチェック
  if (currentTime - lastSpeedCheckTime >= 500) { // 500msごと
    if (previousSpeed != "0" && currentSpeed != "0") {
      int previousSpeedInt = previousSpeed.toInt();
      int currentSpeedInt = currentSpeed.toInt();
      int speedDifference = previousSpeedInt - currentSpeedInt;

      // しきい値を設定（例えば、速度が10km/h以上減少）
      if (speedDifference > 10) {
        lastSpeedCheckTime = currentTime; // チェック時刻を更新
        previousSpeed = currentSpeed; // 現在の速度を保存
        return true; // 急ブレーキと判定
      }
    }
    previousSpeed = currentSpeed; // 現在の速度を保存
  }
  return false; // 急ブレーキでない
}

// タイヤの回転数が高く、速度がゼロの場合のチェック
void checkHighRPMAndNoMovement(String rpm, String speed) {
  // RPMがしきい値を超え、速度がゼロの場合
  if (rpm.toInt() > 3000 && speed == "0") { // 3000 RPMをしきい値として仮定
    Serial.println("タイヤの回転数が異常に高く、前に進まない状態です。雪道発進時にタイヤが滑っている可能性があります。");
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
