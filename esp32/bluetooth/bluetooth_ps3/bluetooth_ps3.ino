#include <NimBLEDevice.h>
#include <NimBLEArduino.h>

NimBLEClient* ps3Client = nullptr;
bool connected = false;

// UUIDを保存する変数
NimBLEUUID buttonServiceUUID;
NimBLEUUID buttonCharacteristicUUID;
NimBLEUUID analogServiceUUID;
NimBLEUUID analogCharacteristicUUID;

void setup() {
  Serial.begin(115200);
  Serial.println("PS3コントローラのUUIDをスキャン中...");

  NimBLEDevice::init("ESP32_PS3");
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new PS3AdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  pScan->start(30);  // 30秒間スキャン
}

void loop() {
  if (connected && ps3Client) {
    // ボタンの状態を取得
    NimBLERemoteService* buttonService = ps3Client->getService(buttonServiceUUID);
    if (buttonService) {
      NimBLERemoteCharacteristic* buttonCharacteristic = buttonService->getCharacteristic(buttonCharacteristicUUID);
      if (buttonCharacteristic) {
        std::string buttonData = buttonCharacteristic->readValue();
        Serial.print("ボタンデータ: ");
        Serial.println((uint8_t)buttonData[0], HEX);
      }
    }

    // アナログスティックの値を取得
    NimBLERemoteService* analogService = ps3Client->getService(analogServiceUUID);
    if (analogService) {
      NimBLERemoteCharacteristic* analogCharacteristic = analogService->getCharacteristic(analogCharacteristicUUID);
      if (analogCharacteristic) {
        std::string analogData = analogCharacteristic->readValue();
        Serial.print("左スティックX: ");
        Serial.println((int16_t)((analogData[0] << 8) | analogData[1]));
        Serial.print("左スティックY: ");
        Serial.println((int16_t)((analogData[2] << 8) | analogData[3]));
        Serial.print("右スティックX: ");
        Serial.println((int16_t)((analogData[4] << 8) | analogData[5]));
        Serial.print("右スティックY: ");
        Serial.println((int16_t)((analogData[6] << 8) | analogData[7]));
      }
    }

    delay(100); // データ取得間隔
  } else {
    Serial.println("PS3コントローラが未接続です");
    delay(1000);
  }
}

class PS3AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    Serial.print("デバイスが見つかりました: ");
    Serial.println(advertisedDevice->toString().c_str());

    if (advertisedDevice->haveName() && advertisedDevice->getName() == "PLAYSTATION(R)3") {
      Serial.println("PS3コントローラが見つかりました。接続中...");

      NimBLEDevice::getScan()->stop();
      ps3Client = NimBLEDevice::createClient();
      ps3Client->connect(advertisedDevice);
      connected = true;
      Serial.println("PS3コントローラに接続されました！");

      // サービスUUIDとCharacteristic UUIDをスキャンして取得
      NimBLERemoteService* pService = ps3Client->getService(NimBLEUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")); // 仮のUUID
      if (pService != nullptr) {
        for (NimBLERemoteCharacteristic* pChar : pService->getCharacteristics()) {
          Serial.print("Characteristic UUID: ");
          Serial.println(pChar->getUUID().toString().c_str());

          // ボタンCharacteristic UUIDの保存
          if (pChar->getUUID().toString() == "your_button_characteristic_uuid") { // 仮のUUID
            buttonCharacteristicUUID = pChar->getUUID();
            Serial.println("ボタンCharacteristic UUIDが保存されました。");
          }

          // アナログCharacteristic UUIDの保存
          if (pChar->getUUID().toString() == "your_analog_characteristic_uuid") { // 仮のUUID
            analogCharacteristicUUID = pChar->getUUID();
            Serial.println("アナログCharacteristic UUIDが保存されました。");
          }
        }
      }
    }
  }
};
