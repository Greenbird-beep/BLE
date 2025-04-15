
/**
 *  NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE server library.
 *
 *  Created: on March 22 2020
 *      Author: H2zero
 */
#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE server and characteristic pointers
static NimBLEServer* pServer;
static NimBLECharacteristic* pChr = nullptr; // Global pointer for access in loop()

// GPIO pin connected to a digital sensor
const int sensorPin = 15;

// Variables to track state and count of sensor triggers
int counta = 0, countb = 0, a = 0, b = 0, diff=0, i=0;

// Custom BLE server callback class
class ServerCallbacks : public NimBLEServerCallbacks {
    // Called when a client connects
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
        // Update connection parameters for better performance
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
    }

    // Called when a client disconnects
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.println("Client disconnected. Restarting advertising...");
        NimBLEDevice::startAdvertising(); // Restart advertising for new connections
    }
} serverCallbacks;

// Custom characteristic callback class
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    // Called when a client writes to the characteristic
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("Write [%s]: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    // Called when a client reads the characteristic
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("Read [%s]\n", pCharacteristic->getUUID().toString().c_str());
    }
} chrCallbacks;

void setup() {
    Serial.begin(115200); // Start serial communication
    delay(1000);
    Serial.println("Starting BLE Server");

    pinMode(sensorPin, INPUT); // Configure sensor pin as input

    // Initialize BLE device with a name
    NimBLEDevice::init("NimBLE");

    // Optional: Set BLE security requirements
    NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);

    // Create BLE server and assign callbacks
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);

    // Create a BLE service with UUID "BAAD"
    NimBLEService* pService = pServer->createService("BAAD");

    // Create a BLE characteristic with UUID "F00D"
    pChr = pService->createCharacteristic(
        "F00D",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    pChr->setValue("Initial"); // Set initial value
    pChr->setCallbacks(&chrCallbacks); // Set characteristic callbacks

    pService->start(); // Start the service

    // Start BLE advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("BLE_Server_2"); // Advertised device name
    pAdvertising->addServiceUUID(pService->getUUID()); // Include service UUID in advertisement
    pAdvertising->start();

    Serial.println("Advertising started");
}

void loop() {
    // Read the current state of the sensor pin
    b = digitalRead(sensorPin);

    // Detect a rising edge (from LOW to HIGH)
    if (b == 1 && a == 0) {
        countb++;
    }

    if (countb/2 != counta/2) {
      Serial.printf("Sensor triggered. Count: %d\n", countb / 2); // Log half-count (pairs of changes)
      counta=countb;

      // If BLE client is connected, notify it with the new count
      if (pChr && pServer->getConnectedCount()) {
          pChr->setValue(""); // Clear previous value
          pChr->setValue("count: " + std::to_string(countb / 2)); // Set new value
          pChr->notify(); // Send notification to client
      }
    }

    a = b;       // Save previous sensor state for edge detection
    delay(30);   // Basic debounce delay
}
