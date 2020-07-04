//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 4/7/20
//======================================================================
//
//  Module: Timer (MASTER)
//
//  version 0.1.0
//
//  Goal for this version: Get this thing going!
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>
#include <CAN.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Entropy.h>
//#include <LiquidCrystal.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
#define PIN_CAN_TX  GPIO_NUM_26
#define PIN_CAN_RX  GPIO_NUM_27
#define PIN_PIEZO   GPIO_NUM_2


Adafruit_7segment timerdisp = Adafruit_7segment();

// Game
byte gamemode = 0;

// Timer
long timeleft;
int gamelength = 300; //seconds
long thismillis;
long delta_t;
char timestr[5] = "----";
byte strikenumber = 0;
byte strikelimit = 3;
char sec_tick_over;
long buzz_timer;
byte time_scale; // Quadruple the time scale: 4 = 1x speed (normal), 5 = 1.25x speed (1 strike), etc...
long blinktime;
long blinkperiod = 250; // Milliseconds
bool blinkbool;
long buzzerinterrupt;

// Widgets
char serial_number[7];
bool serial_vowel;
bool serial_odd;
bool serial_even;

byte battery_number;
byte port_number;
byte ind_number;
byte widget_types[5];

bool parallel_port;
bool ind_frk;
bool ind_car;

// BLE
String BLE_value;
String BLE_state = "";
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
String temp_holder;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        BLE_value = "";
        for (int i = 0; i < value.length(); i++) {
          BLE_value = BLE_value + value[i];
        }
        Serial.print("BLE_value = ");
        Serial.println(BLE_value);

        // Add in switch/case or if/else to set flags based on BLE_value input.

        if (BLE_value.substring(0, 2) == "t=") {
          temp_holder = BLE_value.substring(2);
          gamelength = temp_holder.toInt();
        }
        if (BLE_value == "check") {
          BLE_state = "";
          // Add in statements to change BLE_state, which will be sent back to phone
          BLE_state = "timer = " + char(gamelength);
          pCharacteristic->setValue(BLE_state.c_str()); // Return status
        }
      }
    }
};

// CAN
#define CAN_FILTER      B10000100000  // Filter for the Master
#define CAN_MASK        B11111100000

#define CAN_TO_MASTER   B10000000000
#define CAN_TO_STD_MOD  B01000000000
#define CAN_TO_NEEDY    B00100000000
#define CAN_RE_WIDGET   B00010000000
#define CAN_RE_TIME     B00001000000
#define CAN_RE_STRIKE   B00000100000


//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(9600);
  while (!Serial);
  Serial.println("== KTOME: Timer ==");

  // Start CAN bus
  CAN.setPins(PIN_CAN_RX, PIN_CAN_TX);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  CAN.filter(CAN_FILTER, CAN_MASK);
  CAN.onReceive(onReceive);

  // Start BLE
  BLEDevice::init("KTOME");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Starting...");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // Randomiser
  Entropy.initialize();
  randomSeed(Entropy.random());

}

void loop() {

  switch (gamemode) {
    case 0: // First time set-up
      initialisation();
      break;
    case 1: // Game in set-up

      break;
    case 2: // Game running
      Serial.println(F("Game starting!"));

      break;
    case 3: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone

      break;
  }
}

//**********************************************************************
// FUNCTIONS: Game Initialisation
//**********************************************************************

// Comm with modules to determine what's connected

// first byte:    'I'     : Scan modules for Initialisation
// second byte:   'a'-'n' : 14 vanilla modules
// third byte:    number  : unique number for that copy of the module, starting at 1.

void initialisation() {

  byte max_module_copies = 1;

  for (byte b2 = 0; b2 < 14; b2++) {
    for (byte b3 = 1; b3 < max_module_copies; b3++) {
      CAN_send(CAN_TO_STD_MOD, ('I' + char(b2 + 97) + char(49)), 3);
      delay(50);
    }
  }

}


//**********************************************************************
// FUNCTIONS: Game Managers
//**********************************************************************

// Managing timer

// Listening to modules

// Game win/loss


//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CAN_send(int id, char msg_data[], byte msg_len) {
  Serial.print("Sending packet ... ");

  CAN.beginPacket(id);
  CAN.write(msg_data, msg_len);
  CAN.endPacket();

  Serial.println("done");
}

void onReceive(int packetSize) {

  Serial.print("Received ");

  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(CAN.packetId(), HEX);

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);

    // only print packet data for non-RTR packets
    while (CAN.available()) {
      Serial.print((char)CAN.read());
    }
    Serial.println();
  }

  Serial.println();
}
