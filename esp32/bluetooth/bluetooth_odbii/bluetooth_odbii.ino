/*
arduino-cli lib install "ELMduino"
*/

/*
PIDコード	データ内容	単位
010C	エンジン回転数	RPM
010D	車速	km/h
0104	エンジン負荷	%
0105	冷却水温	°C
010F	吸気温度	°C
0111	スロットル位置	%
012F	燃料レベル	%
0133	エンジン運転時間	秒
0146	環境温度	°C
015E	燃料の流入率	g/s
*/
#include <ELMduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
ELM327 myELM327;

#define DEBUG_SERIAL Serial

String supportedPIDs[128];  // サポートされているPIDのリスト

void setup() {
  DEBUG_SERIAL.begin(115200);
  SerialBT.begin("ESP32_OBD");

  if (!SerialBT.connect("OBDII")) {  // 実際のELM327のBluetooth名に変更してください
    DEBUG_SERIAL.println("Bluetooth接続に失敗しました！");
    return;
  }
  DEBUG_SERIAL.println("Bluetooth接続に成功しました");

  if (!myELM327.begin(SerialBT, true, 2000)) {
    DEBUG_SERIAL.println("ELM327の初期化に失敗しました");
    SerialBT.disconnect();
  } else {
    DEBUG_SERIAL.println("ELM327の初期化に成功しました");
  }

  // サポートされるPIDの検出
  detectSupportedPIDs();
}

void loop() {
  // サポートされているPIDごとにデータを取得
  for (String pid : supportedPIDs) {
    if (pid.length() > 0) {
      String data = getOBDData(pid);
      if (data.length() > 0) {
        DEBUG_SERIAL.print("PID ");
        DEBUG_SERIAL.print(pid);
        DEBUG_SERIAL.print(": ");
        DEBUG_SERIAL.println(data);
      }
    }
    delay(500);  // データ取得間隔
  }
  delay(10000);  // 10秒ごとにデータを再取得
}

// サポートされているPIDを複数のビットマップから検出
void detectSupportedPIDs() {
  String commands[] = {"0100", "0120", "0140", "0160"};

  int pidIndex = 0;
  for (String command : commands) {
    String response = getOBDData(command);
    if (response.length() < 8) continue;  // 反応が不十分ならスキップ

    unsigned long bitmap = strtoul(response.c_str(), NULL, 16);  // ビットマップとして解釈
    for (int i = 0; i < 32; i++) {
      if (bitmap & (1 << (31 - i))) {
        supportedPIDs[pidIndex] = command.substring(0, 2) + String(i, HEX).toUpperCase();
        DEBUG_SERIAL.println("Supported PID: " + supportedPIDs[pidIndex]);
        pidIndex++;
      }
    }
  }
}

// ELM327モジュールにコマンドを送信
void sendCommand(String command) {
  SerialBT.print(command + "\r");
  delay(100);
}

// OBD-IIデータ取得
String getOBDData(String command) {
  sendCommand(command);

  String response = "";
  while (SerialBT.available()) {
    char c = SerialBT.read();
    response += c;
  }
  response.trim();
  return response;
}
