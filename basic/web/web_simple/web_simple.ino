/*
arduino-cli lib install SPI
arduino-cli lib install Ethernet
arduino-cli lib install SD
*/
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

#define SENSOR_PIN A0  // 任意のセンサー用ピン
const int chipSelect = 4;  // SDカードのチップセレクトピン

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // MACアドレス
EthernetServer server(80);  // HTTPサーバー設定

void setup() {
  Serial.begin(9600);

  // Ethernetの開始
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Ethernet初期化失敗");
    while (true);
  }
  server.begin();
  Serial.println("サーバー起動");

  // SDカードの初期化
  if (!SD.begin(chipSelect)) {
    Serial.println("SDカード初期化失敗");
    return;
  }
  Serial.println("SDカード初期化成功");
}

void loop() {
  // センサー値の読み取り
  int sensorValue = analogRead(SENSOR_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);

  // SDカードにログを記録
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print("センサー値: ");
    dataFile.println(voltage);
    dataFile.close();
    Serial.println("ログに記録しました");
  } else {
    Serial.println("ログファイルを開けません");
  }

  // Ethernetサーバーのリクエスト処理
  EthernetClient client = server.available();
  if (client) {
    if (client.connected()) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<html><body>");
      client.println("<h1>センサー値</h1>");
      client.print("<p>");
      client.print(voltage);
      client.println(" V</p>");
      client.println("</body></html>");
      client.stop();
    }
  }
  delay(1000);
}