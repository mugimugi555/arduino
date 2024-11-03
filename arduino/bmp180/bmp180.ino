#include <Wire.h>
#include <Adafruit_BMP085.h>

#define seaLevelPressure_hPa 1013.25

Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(115200);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }

  // CSVのヘッダーを出力
  Serial.println("Temperature (C),Pressure (Pa),Altitude (m),Pressure at Sealevel (Pa),Real Altitude (m)");
}

void loop() {
  // センサーデータをCSV形式で出力
  Serial.print(bmp.readTemperature());
  Serial.print(",");
  Serial.print(bmp.readPressure());
  Serial.print(",");
  Serial.print(bmp.readAltitude());
  Serial.print(",");
  Serial.print(bmp.readSealevelPressure());
  Serial.print(",");
  Serial.println(bmp.readAltitude(seaLevelPressure_hPa * 100));

  delay(500);
}
