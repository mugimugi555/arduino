#include <WiFi.h>
#include <esp_camera.h>
#include <SD.h>
#include <FS.h>
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema.h"
#include "tensorflow/lite/version.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// WiFi設定
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// SDカードの設定
#define SD_CS 5  // SDカードのCSピン番号
File file;

// TensorFlow Lite用の変数
const int inputWidth = 224;  // モデルが要求する幅
const int inputHeight = 224; // モデルが要求する高さ
uint8_t inputData[inputWidth * inputHeight * 3]; // RGBの入力データ用バッファ

// エラーレポーターとインタープリターを作成
tflite::MicroErrorReporter errorReporter;
tflite::ops::micro::AllOpsResolver resolver;
tflite::MicroInterpreter* interpreter;
const tflite::Model* model;

void setup() {
  Serial.begin(115200);
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
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;

  esp_camera_init(&config);

  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // SDカード初期化
  if (!SD.begin(SD_CS)) {
    Serial.println("SDカードの初期化に失敗しました");
    return;
  }
  Serial.println("SDカードの初期化に成功しました");

  // TensorFlow Liteモデルの初期化
  model = tflite::GetModel(your_model_data); // your_model_dataをあなたのモデルのバイナリデータに置き換えてください
  interpreter = new tflite::MicroInterpreter(model, resolver, inputData, sizeof(inputData), &errorReporter);
  interpreter->AllocateTensors();
}

void resizeAndNormalizeImage(uint8_t* inputImage, int inputWidth, int inputHeight, uint8_t* outputImage, int outputWidth, int outputHeight) {
  // ここに画像のリサイズと正規化処理を実装
  // この部分はライブラリや手法によって異なるため、適宜実装してください。
}

const char* runInference() {
  // 入力データをモデルに設定
  memcpy(interpreter->input(0)->data.uint8, inputData, inputWidth * inputHeight * 3);

  // 推論を実行
  interpreter->Invoke();

  // 出力結果を取得（クラス数がOUTPUT_CLASSESだと仮定）
  float* output = interpreter->output(0)->data.f;
  int detectedIndex = -1;
  float maxScore = 0.0;

  for (int i = 0; i < OUTPUT_CLASSES; i++) {
    if (output[i] > maxScore) {
      maxScore = output[i];
      detectedIndex = i;
    }
  }

  // 検出されたオブジェクトの名前を取得
  return getObjectName(detectedIndex); // インデックスに基づいてオブジェクト名を返す関数を実装してください
}

void loop() {
  // 画像を取得
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("カメラからの画像取得に失敗しました");
    return;
  }

  // 画像の前処理
  resizeAndNormalizeImage(fb->buf, fb->width, fb->height, inputData, inputWidth, inputHeight);

  // モデルによる物体認識を実行
  const char* detectedObject = runInference();

  // 物体が認識された場合
  if (detectedObject != nullptr) {
    // 画像をSDカードに保存
    String imageFileName = "/image_" + String(millis()) + ".jpg"; // 画像ファイル名
    file = SD.open(imageFileName.c_str(), FILE_WRITE);
    if (file) {
      file.write(fb->buf, fb->len);
      file.close();
      Serial.println("画像を保存しました: " + imageFileName);
    }

    // 認識結果をテキストファイルに保存
    String textFileName = "/result_" + String(millis()) + ".txt"; // テキストファイル名
    file = SD.open(textFileName.c_str(), FILE_WRITE);
    if (file) {
      file.println(detectedObject);
      file.close();
      Serial.println("認識結果を保存しました: " + textFileName);
    }
  }

  // フレームバッファを解放
  esp_camera_fb_return(fb);

  // 次のループまで待機
  delay(10000); // 10秒待機
}

// 他に必要な関数やモデルのロード部分も追加してください
