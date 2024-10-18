#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLECharacteristic.h>
#include <HIDTypes.h>
#include <BLEMouse.h>
#include <BLEKeyboard.h>

BLEMouse bleMouse;
BLEKeyboard bleKeyboard;

void setup() {
  Serial.begin(115200);

  // Initialize BLE Mouse and Keyboard
  bleMouse.begin();
  bleKeyboard.begin();
}

void loop() {
  // Simulate keyboard typing
  if (bleKeyboard.isConnected()) {
    bleKeyboard.print("Hello, world!");
    delay(1000);
  }

  // Simulate mouse movement
  if (bleMouse.isConnected()) {
    bleMouse.move(10, 0);  // Move mouse right
    delay(100);
  }
}
