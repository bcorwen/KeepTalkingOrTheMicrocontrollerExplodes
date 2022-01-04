#ifndef KTOME_BLE_H
#define KTOME_BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_TX  "beb5483f-36e1-4688-b7f5-ea07361b26a8"

class KTOME_BLE {
	private:
		
		
	public:
		bool start();
		void send(String BLE_msg);
	
		bool deviceConnected = false;
		//bool deviceConnected_prev = false;
		String BLE_value = "";
		String BLE_state = "";	
		BLEServer* pServer = NULL;
		BLECharacteristic* pTxCharacteristic = NULL;
		uint8_t txValue = 0;
		String temp_holder;		
		
};

class KTOME_BLE_ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      gamemode = 0;
    }
};

class KTOME_BLE_Callbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        BLE_value = "";
        for (int i = 0; i < rxValue.length(); i++) {
          BLE_value = BLE_value + rxValue[i];
        }
        Serial.print("BLE_value = ");
        Serial.println(BLE_value);

        // Add in switch/case or if/else to set flags based on BLE_value input.
        BLE_state = "";
        if (BLE_value.substring(0, 2) == "t=") {
          temp_holder = BLE_value.substring(2);
          gamelength = temp_holder.toInt();
          BLE_state = "ok";
        } else if (BLE_value.substring(0, 2) == "h=") { // Hardcore selector
          if (BLE_value.substring(2) == "0") {
            hardcore_mode = false;
          } else {
            hardcore_mode = true;
          }
          BLE_state = "ok";
        } else if (BLE_value == "I") { // App moving from BLE connection screen to Game Manager or manual re-poll of connected modules
          gamemode = 1; // Now (re-)start polling connected modules
          holding = false;
          //
        } else if (BLE_value == "C") { // App starting module manual setup
          gamemode = 2;
          holding = false;
        } else if (BLE_value == ">") {
          manual_check = true;
          holding = false;
        } else if (BLE_value == "A") {
          gamemode = 3;
          holding = false;

          //        } else if (BLE_value == "ri") { // Request master to re-run initialisation of connect modules
          //          BLE_state = "ok";
          //          gamemode = 0;
          //        } else if (BLE_value == "mms") { // Start manual module setup
          //          BLE_state = "b 2 3"; // Space=separated list of manual setup instructions: first char is letter coded to module type, then numbers corresponding to app array
          //        } else if (BLE_value == "?") { // Check/confirm manual setup
          //          BLE_state = "ok"; // "!" if incorrect on check, otherwise send next manual module setup as above; "ok" when completed setup for all modules
          //        } else if (BLE_value == "gs") { // Start game
          //          game_ready = true;
          //          BLE_state = "ok";
          //        } else if (BLE_value == "check") { // LEGACY - unused
          //          BLE_state = "";
          //          // Add in statements to change BLE_state, which will be sent back to phone
          //          BLE_state = "timer = " + char(gamelength);
        }
        //        pCharacteristic->setValue(BLE_state.c_str()); // Return status
        Serial.print("BLE_state = ");
        Serial.println(BLE_state);
      }
    }
};

extern KTOME_BLE ktomeBLE;

#endif