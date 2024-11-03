#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include <JPEGDecoder.h> // JPEG用
#include <PNGdec.h>      // PNG用

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "YOUR_SSID";         // WiFiのSSID
const char* password = "YOUR_PASSWORD"; // WiFiのパスワード

AsyncWebServer server(80);

// 画像を表示するURL
String imageUrl = "";

void setup() {
  Serial.begin(115200);

  // OLEDディスプレイの初期化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // WiFiに接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFiに接続しました！");

  // Webサーバーの設定
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form action=\"/set-image\" method=\"GET\">"
                                      "画像URL: <input type=\"text\" name=\"url\">"
                                      "<input type=\"submit\" value=\"表示\">"
                                      "</form>");
  });

  server.on("/set-image", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("url")) {
      imageUrl = request->getParam("url")->value();
      displayImageFromUrl(imageUrl.c_str());
      request->send(200, "text/html", "画像を表示しました！<br><a href=\"/\">戻る</a>");
    } else {
      request->send(400, "text/html", "URLが指定されていません。<br><a href=\"/\">戻る</a>");
    }
  });

  server.begin();
}

void loop() {
  // 何もしない
}

void displayImageFromUrl(const char* url) {
  WiFiClient client;
  if (client.connect("example.com", 80)) { // 画像のホスト名に変更
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: example.com\r\n" + // 画像のホスト名に変更
                 "Connection: close\r\n\r\n");

    // レスポンスを待機
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        // ヘッダーの終わりを確認
        if (line.length() == 0) {
          break;
        }
      }
    }

    // 画像データの読み込み
    // JPEG画像をデコードするための処理
    if (urlEndsWith(url, ".jpg") || urlEndsWith(url, ".jpeg")) {
      // JPEGデコードの処理
      JpegDec.decodeUrl(url);
      display.clearDisplay();
      drawJpgImage(JpegDec.getWidth(), JpegDec.getHeight());
    }
    // PNG画像をデコードするための処理
    else if (urlEndsWith(url, ".png")) {
      // PNGデコードの処理
      PNG png;
      if (png.open(url, client)) {
        display.clearDisplay();
        drawPngImage(png);
        png.close();
      } else {
        Serial.println("PNG画像の読み込みに失敗しました");
      }
    }
  } else {
    Serial.println("接続に失敗しました");
  }
}

void drawJpgImage(int originalWidth, int originalHeight) {
  int targetWidth = SCREEN_WIDTH;
  int targetHeight = SCREEN_HEIGHT;

  for (int y = 0; y < targetHeight; y++) {
    for (int x = 0; x < targetWidth; x++) {
      int pixelIndex = (y * originalHeight / targetHeight) * originalWidth + (x * originalWidth / targetWidth);
      JpegDec.read(pixelIndex);
      uint16_t color = JpegDec.getPixel(pixelIndex);
      uint8_t grayscale = (uint8_t)((red(color) * 0.299 + green(color) * 0.587 + blue(color) * 0.114)); // モノクロ化
      display.drawPixel(x, y, grayscale > 128 ? WHITE : BLACK); // モノクロに変換
    }
  }
  display.display();
}

void drawPngImage(PNG& png) {
  int targetWidth = SCREEN_WIDTH;
  int targetHeight = SCREEN_HEIGHT;

  for (int y = 0; y < targetHeight; y++) {
    for (int x = 0; x < targetWidth; x++) {
      int originalX = x * png.width / targetWidth;
      int originalY = y * png.height / targetHeight;
      uint16_t color = png.getPixel(originalX, originalY);
      uint8_t grayscale = (uint8_t)((red(color) * 0.299 + green(color) * 0.587 + blue(color) * 0.114)); // モノクロ化
      display.drawPixel(x, y, grayscale > 128 ? WHITE : BLACK); // モノクロに変換
    }
  }
  display.display();
}

// URLが指定された拡張子で終わるか確認する関数
bool urlEndsWith(const char* url, const char* suffix) {
  size_t urlLen = strlen(url);
  size_t suffixLen = strlen(suffix);
  if (urlLen < suffixLen) return false;
  return strcmp(url + urlLen - suffixLen, suffix) == 0;
}
