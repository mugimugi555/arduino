#include <ESP8266WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266WebServer.h>

const char* ssid = "YOUR_SSID";          // WiFi SSID
const char* password = "YOUR_PASSWORD";  // WiFi Password

ESP8266WebServer server(80); // ポート80でWebサーバーを作成

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  if (!SD.begin(D2)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  server.on("/", handleRoot); // ルートURLへのリクエストを処理
  server.on("/download", handleDownload); // ダウンロードリクエストを処理
  server.on("/delete", handleDelete); // 削除リクエストを処理
  server.begin();
  Serial.println("HTTP server started.");
}
void handleRoot() {
  String response = "<html><body><h1>File List</h1><ul>";

  listFiles(SD.open("/"), response, "/"); // SDカードのルートをリストアップ

  response += "</ul></body></html>";

  server.send(200, "text/html", response); // HTMLレスポンスを送信
}
void listFiles(File dir, String &response, String path) {
  while (File file = dir.openNextFile()) {
    response += "<li>";

    if (file.isDirectory()) {
      // サブディレクトリの場合、リンクを作成
      response += "<a href='?dir=" + path + file.name() + "'>" + file.name() + "</a>";
    } else {
      // ファイルの場合、ダウンロードと削除リンクを作成
      response += file.name();
      response += " <a href='/download?file=" + path + file.name() + "'>Download</a>";
      response += " <a href='/delete?file=" + path + file.name() + "'>Delete</a>";
    }

    response += "</li>";
  }
  dir.close();
}
void handleDownload() {
  if (server.hasArg("file")) {
    String filePath = server.arg("file");
    File file = SD.open(filePath);
    if (file) {
      server.streamFile(file, "application/octet-stream");
      file.close();
    } else {
      server.send(404, "text/plain", "File not found");
    }
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}
void loop() {
  server.handleClient(); // クライアントからのリクエストを処理
}
