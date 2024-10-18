#include <WiFi.h>
#include "esp_camera.h"
#include <SD.h>
#include <SPI.h>
#include <WebServer.h>

// WiFi SSIDとパスワードを設定
const char* ssid = "WIFISSID";     // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える

// SDカードのCSピン
#define SD_CS 5

// Webサーバーのポート
WebServer server(80);

// カメラ設定
camera_config_t config;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // SDカードの初期化
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card mount failed");
    return;
  }
  Serial.println("SD Card mounted successfully");

  // カメラの設定
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // カメラ初期化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Webサーバーのルートハンドラを設定
  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/images", HTTP_GET, handleImages);
  server.on("/images/", HTTP_GET, handleImage);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Webサーバーのクライアントリクエストを処理
}

void handleRoot() {
  server.send(200, "text/html", "<h1>ESP32 Camera</h1><a href='/capture'>Capture Image</a><br><a href='/images'>View Images</a>");
}

void handleCapture() {
  // 画像を撮影してSDカードに保存
  takePicture();

  // 成功メッセージを返す
  server.send(200, "text/plain", "Image captured. You can view it at: /images");
}

void takePicture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // 画像のファイル名を生成
  String path = "/image" + String(millis()) + ".jpg";  // 現在のミリ秒をファイル名に使用

  // SDカードに画像を保存
  File file = SD.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    esp_camera_fb_return(fb);
    return;
  }

  // 画像データを書き込む
  file.write(fb->buf, fb->len);
  Serial.printf("Saved file to path: %s\n", path.c_str());

  file.close();
  esp_camera_fb_return(fb);
}

void handleImages() {
  String html = "<h1>Captured Images</h1><ul>";

  // SDカード内のファイルをリストする
  File dir = SD.open("/");
  File file = dir.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String filename = file.name();
      // 画像リンクを作成
      html += "<li><a href='/images/" + filename + "'>" + filename + "</a></li>";
    }
    file = dir.openNextFile();
  }
  html += "</ul><a href='/'>Back</a>";

  server.send(200, "text/html", html);
}

void handleImage() {
  String path = server.uri(); // リクエストされた画像のパス
  File file = SD.open(path.c_str());

  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  // 画像ファイルをクライアントに送信
  server.streamFile(file, "image/jpeg");
  file.close();
}
