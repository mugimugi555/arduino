#include <NimBLEDevice.h>

NimBLEClient* wiiClient = nullptr;
bool connected = false;
NimBLEUUID serviceUUID;
NimBLEUUID accelCharUUID; // 加速度センサーのCharacteristic UUIDを保存する変数

void setup() {
  Serial.begin(115200);
  Serial.println("WiiリモコンのUUIDをスキャン中...");

  NimBLEDevice::init("");
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  pScan->start(30);  // 30秒間スキャン
}

void loop() {
  if (connected && wiiClient) {
    // 加速度センサーデータの取得処理
    NimBLERemoteService* pService = wiiClient->getService(serviceUUID);
    if (pService != nullptr) {
      NimBLERemoteCharacteristic* pAccelChar = pService->getCharacteristic(accelCharUUID);
      if (pAccelChar != nullptr) {
        std::string value = pAccelChar->readValue();

        // 加速度センサーデータの表示
        int16_t x = (value[0] << 8) | value[1];
        int16_t y = (value[2] << 8) | value[3];
        int16_t z = (value[4] << 8) | value[5];

        Serial.print("X軸: "); Serial.println(x);
        Serial.print("Y軸: "); Serial.println(y);
        Serial.print("Z軸: "); Serial.println(z);
      }
    }
    delay(100); // データ取得間隔
  }
}

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("デバイスが見つかりました: ");
        Serial.println(advertisedDevice->toString().c_str());

        // Wiiリモコンを特定する
        if (advertisedDevice->haveName() && advertisedDevice->getName() == "Nintendo RVL-CNT-01") {
            Serial.println("Wiiリモコンが見つかりました。UUIDを取得中...");

            // サービスUUIDを取得
            for (NimBLEUUID uuid : advertisedDevice->getServiceUUIDs()) {
                Serial.print("サービスUUID: ");
                Serial.println(uuid.toString().c_str());
                // サービスUUIDを保存
                serviceUUID = uuid;
            }

            // 接続処理
            NimBLEDevice::getScan()->stop();
            wiiClient = NimBLEDevice::createClient();
            wiiClient->connect(advertisedDevice);
            connected = true;
            Serial.println("Wiiリモコンに接続されました！");

            // 加速度センサーのCharacteristic UUIDをスキャンして取得
            NimBLERemoteService* pService = wiiClient->getService(serviceUUID);
            if (pService != nullptr) {
                for (NimBLERemoteCharacteristic* pChar : pService->getCharacteristics()) {
                    Serial.print("Characteristic UUID: ");
                    Serial.println(pChar->getUUID().toString().c_str());

                    // 加速度センサーのCharacteristic UUIDを見つけたら保存
                    if (isAccelCharacteristic(pChar->getUUID())) {
                        accelCharUUIDs.push_back(pChar->getUUID());
                        Serial.println("加速度センサーのCharacteristic UUIDが保存されました。");
                    }
                }
            } else {
                Serial.println("サービスが見つかりませんでした。");
            }
        }
    }

    // 加速度センサーのCharacteristic UUIDを判定する関数
    bool isAccelCharacteristic(const NimBLEUUID& charUUID) {
        // ここで、UUIDの形式を確認する処理を実装します
        // 仮のUUIDとして特定の条件で判定する例を示します
        return charUUID.toString().startsWith("0000"); // 例: UUIDが"0000"で始まる場合
    }
};
