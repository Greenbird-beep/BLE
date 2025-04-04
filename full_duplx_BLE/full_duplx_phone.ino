#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.print("Received from phone: ");
      Serial.println(rxValue);
    }
  }
};



class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Phone connected!");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Phone disconnected!");
        BLEDevice::startAdvertising(); // Re-advertise
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init("ESP32_BLE_FullDuplex");

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Ready");
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Android compatibility
    pAdvertising->setMinPreferred(0x12);  // Android compatibility
    BLEDevice::startAdvertising();

    Serial.println("BLE Full Duplex Server is up. Waiting for phone...");
}

void loop() {
    if (deviceConnected && Serial.available()) {
        String data = Serial.readStringUntil('\n');
        pCharacteristic->setValue(data.c_str());
        pCharacteristic->notify();
        Serial.print("Sent to phone: ");
        Serial.println(data);
    }

    delay(100);
}
