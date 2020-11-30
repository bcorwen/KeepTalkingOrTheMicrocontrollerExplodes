#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

bool deviceConnected = false;

String BLE_value;
String BLE_state = "";
#define SERVICE_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // Service UUID for the device
#define CHARACTERISTIC_UUID_RX  "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Characteristic UUID for incoming messages
#define CHARACTERISTIC_UUID_TX  "beb5483f-36e1-4688-b7f5-ea07361b26a8" // Characteristic UUID for outgoing messages

// Service and interrupt setup - useful for incoming messages

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
uint8_t txValue = 0;

String temp_holder;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      gamemode = 0;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks { // Set up an interrupt function which can parse and react to incoming messages
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        BLE_value = "";
        for (int i = 0; i < rxValue.length(); i++) {
          BLE_value = BLE_value + rxValue[i];
        }
        Serial.print("BLE_value = ");
        Serial.println(BLE_value);

        // If-else or switch-case below to read (part of) the incoming BLE message and react accordingly
        BLE_state = "";
        if (BLE_value.substring(0, 2) == "t=") {
          temp_holder = BLE_value.substring(2);
          gamelength = temp_holder.toInt();
          BLE_state = "ok";
        }
        Serial.print("BLE_state = ");
        Serial.println(BLE_state);
      }
    }
};

void setup() {

  // Start BLE
  BLEDevice::init("KTOME");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  
}

void loop() {

  String BLE_msg;
  BLE_message = "Hellow world!";
  BLESend(BLE_msg);
  delay(1000);
  
}

void BLESend(String msg_data) {
  
  String BLE_output(msg_data);
  Serial.print("Sending \"");
  Serial.print(msg_data);
  Serial.println("\" to phone app...");
  pTxCharacteristic->setValue(BLE_output.c_str());
  pTxCharacteristic->notify();
  
}
