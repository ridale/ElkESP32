/*
  Send serial to connected device through
  BT descriptors
  sample data:
  0AZC003900C6
  1EAS000000002111111100000000000D
  0AZC003200CD
  1EAS000000001111111100000000000E
  16XK29460642003191010066
  16XK59460642003191010063
  16XK29470642003191010065
  16XK0048064200319101006F
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ElkM1.hpp"

/** Bluetooth classes*/
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define MAX_STR_MESSAGE_LEN 128U
char strmessage[MAX_STR_MESSAGE_LEN] = {0};

#define MAX_BLE_PACKET 20U
uint8_t packet_data[MAX_BLE_PACKET];

#define ELK_SERVICE_UUID        "2a49bcf9-48df-44d9-95fc-8dffb6e8c7de"
#define SERIAL_CHARACTERISTIC_UUID "7ce710ca-6ff1-4f72-8eee-dfd3b34e52ec"
/**

*/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
/**

*/
void setup() {
  Serial.begin(115200);
  // Create the BLE Device
  BLEDevice::init("2A49BCF9");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(ELK_SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      SERIAL_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create a BLE Config Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(ELK_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}
/**

*/
void loop() {
  if (Serial.available()) {
    size_t idx = 0;
    while (Serial.available()) {
      char ch = (char)Serial.read();
      if (ch == '\n') break;
      strmessage[idx] = ch;
      idx++;
      if (idx == MAX_STR_MESSAGE_LEN) break;
    }
    std::string message(strmessage);
    size_t len = MAX_BLE_PACKET;
    memset(packet_data, 0, MAX_BLE_PACKET);
    len = ElkM1::parse(message, packet_data, len);
    memset(strmessage, 0, MAX_STR_MESSAGE_LEN);
    Serial.print("len: ");
    Serial.print(len);
    Serial.print(" ");
    Serial.println(strmessage);
    // notify changed value
    if (deviceConnected) {
      pCharacteristic->setValue(packet_data, len);
      pCharacteristic->notify();
      value++;
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    Serial.println("Client disconnecting...");
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Client connecting...");
  }
}
