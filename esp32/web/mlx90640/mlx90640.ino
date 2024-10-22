// arduino-cli lib install "Adafruit MLX90640"

#include <Wire.h>
#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
  delay(1000);

  uint8_t foundAddress = 0;

  // Scan for I2C devices and store the first found address
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      foundAddress = address;
      Serial.print("I2C device found at address 0x");
      Serial.println(address, HEX);
      break; // Exit after finding the first device
    }
  }

  if (foundAddress == 0) {
    Serial.println("No I2C devices found.");
    while (1);
  }

  Serial.println( mlx.begin(foundAddress, &Wire) );
  Serial.println( mlx.begin( 0x1, &Wire) );
  // Initialize MLX90640 with found address
  if (!mlx.begin(foundAddress, &Wire)) {
    Serial.println("MLX90640 not found");
    while (1);
  }

  Serial.println("MLX90640 Ready!");
}

void loop() {
  float frame[32*24];
  mlx.getFrame(frame);
  for (int i = 0; i < 32*24; i++) {
    Serial.print(frame[i]); Serial.print(", ");
  }
  delay(1000);
}
