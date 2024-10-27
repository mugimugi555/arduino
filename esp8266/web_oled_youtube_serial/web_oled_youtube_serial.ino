#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
}

void loop() {
  if (Serial.available()) {
    drawImageFromSerial();
  }
}

void drawImageFromSerial() {
  uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

  int bytesRead = Serial.readBytes(buffer, sizeof(buffer));

  if (bytesRead == sizeof(buffer)) {
    display.clearDisplay();
    display.drawBitmap(0, 0, buffer, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();
  } else {
    Serial.println("Image data size mismatch.");
  }
}
