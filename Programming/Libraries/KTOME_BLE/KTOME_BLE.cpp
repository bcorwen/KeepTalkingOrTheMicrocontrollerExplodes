#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <KTOME_BLE.h>

bool KTOME_BLE::start() {
	BLEDevice::init("KTOME");
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new KTOME_BLE_ServerCallbacks());
	BLEService *pService = pServer->createService(SERVICE_UUID);

	pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
	pTxCharacteristic->addDescriptor(new BLE2902());

	BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

	pRxCharacteristic->setCallbacks(new KTOME_BLE_Callbacks());

	pService->start();
	pServer->getAdvertising()->start();
}

void KTOME_BLE::send(String BLE_msg) {
	String BLE_output(BLE_msg);
	Serial.print("Sending \"");
	Serial.print(BLE_msg);
	Serial.println("\" to phone app...");
	pTxCharacteristic->setValue(BLE_output.c_str());
	pTxCharacteristic->notify();
}

KTOME_BLE ktomeBLE = KTOME_BLE();