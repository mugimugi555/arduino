#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// HC-05 Bluetoothを接続したピン
SoftwareSerial BTSerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);   // HC-05のデフォルトのATモード通信速度
}

void loop() {

  Serial.println("HC-05 Bluetooth ATコマンド実行");
  //BTSerial.println("HC-05 Bluetooth ATコマンド実行");

  // HC-05の情報を取得するコマンド
  // https://cxemka.com/upload/art/jdy32/f/jdy32_datasheet_user_manual_en.pdf
  sendATCommand("AT+VERSION");
  sendATCommand("AT+MAC");
  sendATCommand("AT+MACS");
  sendATCommand("AT+BAUD");
  sendATCommand("AT+NAME");
  sendATCommand("AT+NAMES");
  sendATCommand("AT+TYPE");
  sendATCommand("AT+PIN");

  sendATCommand("AT");           // 動作確認
  sendATCommand("AT+ADDR");      // MACアドレス
  sendATCommand("AT+UART");      // ボーレート、停止ビット、パリティ
  sendATCommand("AT+ROLE");      // モジュールの役割（マスター/スレーブ）
  sendATCommand("AT+PSWD");      // PINコード
  sendATCommand("AT+CMODE");     // 接続モード（任意接続/固定接続）
  sendATCommand("AT+BIND");      // 接続先の固定アドレス（マスターのみ）
  sendATCommand("AT+POLAR");     // LEDの点滅モード

  Serial.println("すべてのATコマンドが実行されました");
  delay(1000);

}

//
void sendATCommand(const char* command) {

  Serial.println(command);
  BTSerial.println(command);  // Bluetoothにコマンドを送信
  delay(500);  // コマンド処理のための待機

  // Bluetoothからの応答を取得
  while (BTSerial.available()) {
    Serial.write((char)BTSerial.read());
    //BTSerial.write((char)BTSerial.read());
  }
  Serial.println();
  //BTSerial.println();
}