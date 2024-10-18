#include <WiFi.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema.h"
#include "tensorflow/lite/version.h"

// WiFi SSIDとパスワードを設定
const char* ssid = "WIFISSID";     // 自分のWi-Fi SSIDに置き換える
const char* password = "WIFIPASSWD"; // 自分のWi-Fiパスワードに置き換える

// TensorFlow Liteの設定
const tflite::Model* model = tflite::GetModel(model_data);
tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;
constexpr int kTensorArenaSize = 2 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);

void startCameraServer();
void setupLedFlash(int pin);
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // カメラの設定と初期化
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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // TensorFlow Liteの初期化
  interpreter.AllocateTensors();
}

void loop() {
  // カメラから画像を取得
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // 画像データをTensorFlow Liteモデルに渡す
  // （ここではフレームの前処理を行う必要があります）

  // 推論を実行
  interpreter.Invoke();

  // 結果を取得し、JSON形式でシリアル出力
  // （ここでは具体的なモデルの出力を確認し、適切な形式に変換する必要があります）

  // 結果を出力（仮のデータを使用）
  const char* object_detected = "Object Name"; // 実際の検出結果に置き換える
  int confidence = 85; // 実際の信頼度に置き換える

  // JSON形式で出力
  String jsonOutput = String("{\"object\":\"") + object_detected + "\", \"confidence\":" + confidence + "}";
  Serial.println(jsonOutput);

  esp_camera_fb_return(fb);
  delay(1000); // 1秒ごとに検出
}
