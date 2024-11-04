#include <DHT.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define DHTPIN 2       // DHT11のデータピン
#define DHTTYPE DHT11  // DHT11センサーの種類

DHT dht(DHTPIN, DHTTYPE);

// HC-05 Bluetoothを接続したピン
SoftwareSerial BTSerial(10, 11); // RX, TX

void sendATCommand(const char* command) {

  Serial.print("Sending AT command: ");
  Serial.println(command);
  BTSerial.println(command);
  delay(500);

  // 応答の受信と表示
  while (BTSerial.available()) {
    Serial.write(BTSerial.read());
  }

}

void setup() {

  Serial.begin(115200);  // Bluetoothのデフォルト通信速度
  dht.begin();
  BTSerial.begin(9600);   // HC-05のデフォルトのATモード通信速度

}

void loop() {

  Serial.println("HC-05 Bluetooth ATコマンド実行");

  // HC-05の情報を取得するコマンド
  sendATCommand("AT");           // 動作確認
  sendATCommand("AT+VERSION?");   // ファームウェアのバージョン
  sendATCommand("AT+NAME?");      // デバイス名
  sendATCommand("AT+ADDR?");      // MACアドレス
  sendATCommand("AT+UART?");      // ボーレート、停止ビット、パリティ
  sendATCommand("AT+ROLE?");      // モジュールの役割（マスター/スレーブ）
  sendATCommand("AT+PSWD?");      // PINコード
  sendATCommand("AT+CMODE?");     // 接続モード（任意接続/固定接続）
  sendATCommand("AT+BIND?");      // 接続先の固定アドレス（マスターのみ）
  sendATCommand("AT+POLAR?");     // LEDの点滅モード

  // 設定可能なコマンド（必要に応じて有効にしてください）
  // sendATCommand("AT+NAME=NewBluetoothName");    // デバイス名を設定
  // sendATCommand("AT+UART=9600,0,0");            // ボーレートを設定（例：9600）
  // sendATCommand("AT+ROLE=1");                   // 役割を設定（1=マスター、0=スレーブ）
  // sendATCommand("AT+PSWD=1234");                // PINコードを設定
  // sendATCommand("AT+CMODE=0");                  // 接続モードを設定（0=固定アドレス、1=任意アドレス）
  // sendATCommand("AT+BIND=XX:XX:XX:XX:XX:XX");   // 接続先のアドレスを設定（マスターのみ）
  // sendATCommand("AT+POLAR=1,0");                // LEDの点滅を設定（例：1,0）
  // BTSerial.println("AT+ORGL"); // 工場出荷時の設定にリセット

  Serial.println("すべてのATコマンドが実行されました");

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  //if (isnan(humidity) || isnan(temperature)) {
  //  Serial.println("DHT11の読み取りエラー");
  //  delay(2000);
  //  return;
  //}

  // 不快指数の計算
  float discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;

  // JSON形式でデータを構築
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["discomfortIndex"] = discomfortIndex;

  // JSONデータをシリアルで送信
  serializeJson(doc, Serial);
  Serial.println();  // データ区切りのための改行

  delay(2000);  // 2秒間隔で送信
}
