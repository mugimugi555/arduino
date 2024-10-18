#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// WiFi設定
const char* ssid = "YOUR_SSID";      // WiFiのSSID
const char* password = "YOUR_PASSWORD"; // WiFiのパスワード

WebServer server(80); // HTTPサーバーのポートを80に設定

void setup() {
  Serial.begin(115200);

  // カメラの設定
  camera_config_t config;
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
  config.frame_size = FRAMESIZE_SVGA; // 解像度設定
  config.pixel_format = PIXFORMAT_JPEG; // 画像フォーマット設定

  // カメラ初期化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("カメラ初期化に失敗しました: %s\n", esp_err_to_name(err));
    return;
  }

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // HTTPサーバー開始
  server.on("/", HTTP_GET, handleRoot);
  server.on("/video", HTTP_GET, streamVideo);
  server.begin();
  Serial.println("HTTPサーバーが開始されました");
}

void loop() {
  server.handleClient(); // クライアントからのリクエストを処理
}

// ルートリクエストの処理
void handleRoot() {
  String html = "<!DOCTYPE html>\
  <html>\
  <head>\
    <title>ESP32-CAM Stream</title>\
  </head>\
  <body>\
    <h1>ESP32-CAM Video Stream</h1>\
    <img src=\"/video\" width=\"640\" height=\"480\">\
  </body>\
  </html>";

  server.send(200, "text/html", html); // HTMLを送信
}

// 動画ストリーミングの処理
void streamVideo() {
  server.sendHeader("Access-Control-Allow-Origin", "*"); // CORSヘッダーを追加
  server.send(200, "multipart/x-mixed-replace; boundary=frame", ""); // ストリーミング開始

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("カメラからのフレーム取得に失敗しました");
      return;
    }

    // JPEG形式で画像を送信
    String boundary = "--frame\r\n"
                      "Content-Type: image/jpeg\r\n"
                      "Content-Length: " + String(fb->len) + "\r\n\r\n";
    server.sendContent(boundary);
    server.sendContent((const char *)fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb); // フレームバッファを解放

    delay(100); // 適切なフレームレートを調整
  }
}
