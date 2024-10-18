#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SD_MMC.h>
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>
#include <eloquent_tinyml/zoo/person_detection.h>
#include <eloquent_esp32cam.h>
#include <ArduinoJson.h>  // JSONライブラリ

using eloq::camera;
using eloq::tinyml::zoo::personDetection;

const char* ssid = "your-SSID";        // Wi-FiのSSID
const char* password = "your-PASSWORD"; // Wi-Fiのパスワード

WebServer server(80);  // Webサーバーのポートを80に設定

void handleRoot();
void listFiles();
void handleFileView();
void handleFileDelete();
void handleFileListJson(); // 新しいハンドラ関数を宣言
void setupWiFi();
void setupSDCard();

void setup() {
    delay(3000);
    Serial.begin(115200);
    Serial.println("__PERSON DETECTION__");

    setupWiFi();   // Wi-Fiの設定
    setupSDCard(); // SDカードの初期化

    // カメラの設定
    camera.pinout.freenove_s3();
    camera.brownout.disable();
    camera.resolution.yolo();
    camera.pixformat.gray();

    // カメラ初期化
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    // モデル初期化
    while (!personDetection.begin().isOk())
        Serial.println(personDetection.exception.toString());

    Serial.println("Camera OK");

    // ルートにアクセスした際のハンドラを設定
    server.on("/", handleRoot);

    // ファイルリンクをクリックしたときのハンドラを設定
    server.onNotFound(handleFileView);

    // ファイル削除用のハンドラを設定
    server.on("/delete", HTTP_GET, handleFileDelete);

    // JSON形式でファイルリストを取得するためのハンドラを設定
    server.on("/filelist.json", HTTP_GET, handleFileListJson);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();  // クライアントからのリクエストを処理
}

// ルートページのハンドラ
void handleRoot() {
    server.send(200, "text/html", "<h1>File List</h1><ul>");
    listFiles();  // SDカード内のファイルをリスト表示
    server.sendContent("</ul>");
}

// ファイルリストを取得して表示
void listFiles() {
    File root = SD_MMC.open("/");
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        String fileName = String(file.name());
        String fileLink = "<li><a href=\"/" + fileName + "\">" + fileName + "</a></li>";
        server.sendContent(fileLink);
        file = root.openNextFile();
    }
}

// ファイルリンクをクリックした際の処理
void handleFileView() {
    String path = server.uri();  // リクエストされたファイルのパスを取得
    if (SD_MMC.exists(path)) {
        File file = SD_MMC.open(path);
        if (file) {
            String deleteButton = "<form action=\"/delete\" method=\"get\">";
            deleteButton += "<input type=\"hidden\" name=\"file\" value=\"" + path + "\">";
            deleteButton += "<input type=\"submit\" value=\"Delete File\">";
            deleteButton += "</form>";

            if (path.endsWith(".jpg")) {
                // 画像ファイルを表示する
                String htmlContent = "<html><body>";
                htmlContent += "<h1>Image: " + path + "</h1>";
                htmlContent += "<img src=\"" + path + "\" alt=\"image\">";
                htmlContent += deleteButton; // 削除ボタンを追加
                htmlContent += "</body></html>";
                server.send(200, "text/html", htmlContent);
            } else if (path.endsWith(".json")) {
                // JSONファイルをHTMLページに表示
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, file);
                String jsonString;
                serializeJsonPretty(doc, jsonString);

                String htmlContent = "<html><body>";
                htmlContent += "<h1>JSON File: " + path + "</h1>";
                htmlContent += "<pre>" + jsonString + "</pre>";
                htmlContent += deleteButton; // 削除ボタンを追加
                htmlContent += "</body></html>";

                server.send(200, "text/html", htmlContent);
            } else {
                // 他のファイルの場合はダウンロード用に送信
                server.streamFile(file, "application/octet-stream");
            }
            file.close();
        }
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

// ファイル削除用のハンドラ
void handleFileDelete() {
    String fileName = server.arg("file");  // 削除対象のファイル名を取得
    if (SD_MMC.exists(fileName)) {
        SD_MMC.remove(fileName);  // ファイルを削除
        Serial.println("Deleted file: " + fileName);
        server.send(200, "text/html", "<h1>File Deleted</h1><a href=\"/\">Back to File List</a>");
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

// JSON形式のファイルリストを提供するハンドラ
void handleFileListJson() {
    File root = SD_MMC.open("/");
    if (!root) {
        server.send(500, "application/json", "{\"error\":\"Failed to open directory\"}");
        return;
    }
    if (!root.isDirectory()) {
        server.send(500, "application/json", "{\"error\":\"Not a directory\"}");
        return;
    }

    DynamicJsonDocument doc(1024);
    JsonArray filesArray = doc.createNestedArray("files");

    File file = root.openNextFile();
    while (file) {
        String fileName = String(file.name());
        if (fileName.endsWith(".jpg") || fileName.endsWith(".json")) {
            JsonObject fileObject = filesArray.createNestedObject();
            fileObject["name"] = fileName;
            fileObject["size"] = file.size();
        }
        file = root.openNextFile();
    }

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

// Wi-Fi接続設定
void setupWiFi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// SDカードの初期化
void setupSDCard() {
    if (!SD_MMC.begin()) {
        Serial.println("SD Card Mount Failed");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        return;
    }
    Serial.println("SD Card initialized.");
}
