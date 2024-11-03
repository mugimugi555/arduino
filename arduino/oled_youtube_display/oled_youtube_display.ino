/*
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit SSD1306"

bash upload_arduino.sh oled_youtube_display/oled_youtube_display.ino
bash oled_youtube_display/download_and_send.sh
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

//
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//
void setup() {

  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  display.ssd1306_command(SSD1306_SETCONTRAST); // Send the contrast command
  display.ssd1306_command(255); // Set the contrast value (0-255)
  display.clearDisplay();

}

//
void loop() {

  if (Serial.available()) {
    drawImageFromSerial();
  }

}

//
void drawImageFromSerial() {

  uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

  int bytesRead = Serial.readBytes(buffer, sizeof(buffer));

  Serial.print("Bytes read: ");
  Serial.println(bytesRead);

  if (bytesRead == sizeof(buffer)) {
    display.clearDisplay();
    display.drawBitmap(0, 0, buffer, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();
  } else {
    Serial.println("Image data size mismatch.");
  }

}
