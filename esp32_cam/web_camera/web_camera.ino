#include <WiFi.h>
#include "esp_camera.h"
#include "camera_pins.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM

//----------------------------------------------------------------------------
// 定数と変数の定義
//----------------------------------------------------------------------------

// WiFi SSIDとパスワード、ホスト名を指定
const char* ssid     = "WIFISSID";  // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える
const char* hostname = "HOSTNAME";   // ESP32 CAMのホスト名

void startCameraServer();

void setupLedFlash(int pin);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  camera_config_t config;  // カメラの設定構造体を宣言

  // LEDCチャンネルの設定
  config.ledc_channel = LEDC_CHANNEL_0;         // LED制御のチャンネル番号
  config.ledc_timer   = LEDC_TIMER_0;           // LED制御のタイマー番号

  // カメラの各ピン設定
  config.pin_d0       = Y2_GPIO_NUM;            // D0ピン
  config.pin_d1       = Y3_GPIO_NUM;            // D1ピン
  config.pin_d2       = Y4_GPIO_NUM;            // D2ピン
  config.pin_d3       = Y5_GPIO_NUM;            // D3ピン
  config.pin_d4       = Y6_GPIO_NUM;            // D4ピン
  config.pin_d5       = Y7_GPIO_NUM;            // D5ピン
  config.pin_d6       = Y8_GPIO_NUM;            // D6ピン
  config.pin_d7       = Y9_GPIO_NUM;            // D7ピン
  config.pin_xclk     = XCLK_GPIO_NUM;          // XCLKピン（クロック信号）
  config.pin_pclk     = PCLK_GPIO_NUM;          // PCLKピン（ピクセルクロック信号）
  config.pin_vsync    = VSYNC_GPIO_NUM;         // VSYNCピン（垂直同期信号）
  config.pin_href     = HREF_GPIO_NUM;          // HREFピン（水平参照信号）
  config.pin_sccb_sda = SIOD_GPIO_NUM;          // SCCBデータピン
  config.pin_sccb_scl = SIOC_GPIO_NUM;          // SCCBクロックピン
  config.pin_pwdn     = PWDN_GPIO_NUM;          // 電源ダウンピン
  config.pin_reset    = RESET_GPIO_NUM;         // リセットピン

  config.xclk_freq_hz = 20000000;               // XCLK信号の周波数（20MHz）
  config.frame_size   = FRAMESIZE_UXGA;         // フレームサイズをUXGAに設定
  config.pixel_format = PIXFORMAT_JPEG;         // ピクセル形式をJPEGに設定（ストリーミング用）
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY; // フレームバッファが空のときに画像を取得
  config.fb_location  = CAMERA_FB_IN_PSRAM;     // フレームバッファの位置をPSRAMに設定
  config.jpeg_quality = 12;                     // JPEG圧縮の品質を設定（数値が小さいほど高品質）
  config.fb_count     = 1;                      // フレームバッファの数を設定

  // PSRAMの有無に応じた設定
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
  }

  // カメラ初期化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // センサー設定
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // 垂直反転
    s->set_brightness(s, 1);   // 明るさを少し上げる
    s->set_saturation(s, -2);  // 彩度を下げる
  }
  // フレームサイズを下げる
  s->set_framesize(s, FRAMESIZE_QVGA);

  // LEDフラッシュ設定
  setupLedFlash(LED_GPIO_NUM);

  // WiFi接続
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // 何もしない。すべては別のタスクでウェブサーバーによって処理されます
  delay(10000);
}
