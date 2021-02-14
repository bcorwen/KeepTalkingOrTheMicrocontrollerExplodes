//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 31/12/20
//======================================================================
//
//  Module: Timer (MASTER)
//
//  version 0.5.0
//
//  Goal for this version: Complete game logic and common comms
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
//#include <LiquidCrystal.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
//#define PIN_PIEZO     GPIO_NUM_2
#define PIN_LED       GPIO_NUM_12
#define PIN_STRIKE_1  GPIO_NUM_18
#define PIN_STRIKE_2  GPIO_NUM_19

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool manual_check = false;
bool game_running = false;
bool game_win = false;

int module_array[12];
bool module_detected = false;
bool module_inited = false;
byte module_count = 0;
byte module_count_solvable = 0;
bool module_solved_array[12];

// Timer
int32_t timeleft;
int32_t gamelength = 300000; //seconds
int32_t thismillis;
//long delta_t;
String timestr = "----";
String timestr_prev = "----";
bool hardcore_mode = false;
byte strike_number = 0;
byte strike_limit = 3;
bool strike_flag = false;
int strike_culprit; // NEW VARIABLE - track the last module which caused a strike, for debrief
char sec_tick_over; // DELETE?
int32_t buzz_timer;
byte time_scale; // Quadruple the time scale: 4 = 1x speed (normal), 5 = 1.25x speed (1 strike), etc...
int32_t blinktime;
int32_t blinkperiod = 250; // Milliseconds
int32_t timerblinktime;
int32_t timerblinkperiod = 500;
byte timerblinkcount = 0;
bool timerblinkbool = true;
bool blinkbool;
int32_t buzzerinterrupt;
bool explosion_fx = false;

Adafruit_7segment timerdisp = Adafruit_7segment(); // Default pins: I2C SCL = GPIO22, I2C SDA = GPIO21

// Widgets
char serial_number[7];
bool serial_vowel;
bool serial_odd;
bool serial_even;
byte battery_number;
bool port_parallel;
bool port_serial;
bool ind_frk;
bool ind_car;

//char ind_names[11][4] = {
//  {"SND"},
//  {"CLR"},
//  {"CAR"},
//  {"IND"},
//  {"FRQ"},
//  {"SIG"},
//  {"NSA"},
//  {"MSA"},
//  {"TRN"},
//  {"BOB"},
//  {"FRK"}
//};

// CAN
#define CAN_ID            CAN_MASTER
#define CAN_MASK          CAN_MASTER

// BLE
bool deviceConnected = false;
//bool deviceConnected_prev = false;
String BLE_value;
String BLE_state = "";
#define SERVICE_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_TX  "beb5483f-36e1-4688-b7f5-ea07361b26a8"

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
      //      gamemode = 0;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
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
          float temp_float;
          temp_holder = BLE_value.substring(2);
          temp_float = temp_holder.toFloat();
          //          gamelength = long(temp_float * 60000);
          gamelength = long(temp_float * 60000000);
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
        } else if (BLE_value == "Z") {
          gamemode = 4;
          holding = false;


        }
        //        pCharacteristic->setValue(BLE_state.c_str()); // Return status
        //        Serial.print("BLE_state = ");
        //        Serial.println(BLE_state);
      }
    }
};

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Timer ==");

  // Start CAN bus
  ktomeCAN.id(CAN_ID, CAN_MASK);
  ktomeCAN.start();
  // start the CAN bus at 500 kbps
  if (!ktomeCAN.start()) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.print("My ID is:   0b");
  ktomeCAN.padZeros(ktomeCAN.can_id);
  Serial.println(ktomeCAN.can_id, BIN);
  Serial.print("My mask is: 0b");
  ktomeCAN.padZeros(ktomeCAN.can_mask);
  Serial.println(ktomeCAN.can_mask, BIN);

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

  // Randomiser
  esp_random();

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_STRIKE_1, OUTPUT);
  digitalWrite(PIN_STRIKE_1, LOW);
  pinMode(PIN_STRIKE_2, OUTPUT);
  digitalWrite(PIN_STRIKE_2, LOW);

  timerInit();

}

void loop() {

  switch (gamemode) {
    case 0: // Connect to phone
      phoneConnect();
      carPark();
      break;
    case 1: // Module poll
      Serial.println(F("Module search..."));
      initialisation();
      carPark();
      break;
    case 2: // Game (manual) set-up
      Serial.println(F("Game set-up..."));
      phoneSetup();
      gameReset();
      visual_fx(false);
      carPark();
      break;
    case 3: // Game running
      Serial.println(F("Game starting!"));
      gameRunning();
      break;
    case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone
      // Show outcome
      stopMessages();
      if (game_win) {
        visual_fx(true);
      } else {
        visual_fx(false);
      }
      carPark();
      break;
  }
}

void carPark() {
  holding = true;
  Serial.println("Script parked - waiting for direction...");
  while (holding) {
    delay (1);
  }
  if (timerblinktime <= millis() && gamemode == 4) {
    bool draw_colon;
    if (game_win) {
      if (timerblinkbool && timerblinkcount < 2) {
        timerdisp.writeDigitNum(0, (int)(timestr.charAt(0) - 48));
        timerdisp.writeDigitNum(1, (int)(timestr.charAt(1) - 48));
        timerdisp.writeDigitNum(3, (int)(timestr.charAt(2) - 48));
        timerdisp.writeDigitNum(4, (int)(timestr.charAt(3) - 48));
        draw_colon = true;
        timerblinkcount ++;
      } else {
        timerdisp.writeDigitNum(0, ' ');
        timerdisp.writeDigitNum(1, ' ');
        timerdisp.writeDigitNum(3, ' ');
        timerdisp.writeDigitNum(4, ' ');
        timerblinktime = millis() + timerblinkperiod;
        timerblinkbool = false;
        draw_colon = false;
      }
      timerdisp.drawColon(draw_colon);
      timerdisp.writeDisplay();
    } else {
      visual_fx(false);
      game_win = false;
    }
  }
}

//**********************************************************************
// FUNCTIONS: Companion App Setup : gamemode = 0
//**********************************************************************

void phoneConnect() {
  Serial.println("Waiting for phone connection...");
  while (!deviceConnected) {
    delay(500);
    Serial.println("waiting...");
  }
  Serial.println("Phone connected!");
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : gamemode = 1
//**********************************************************************

void initialisation() { // Comm with modules to determine what's connected

  byte max_module_copies = 11;
  char CAN_message[2];
  memset(module_array, 0, sizeof(module_array));
  memset(module_solved_array, 0, sizeof(module_solved_array));

  Serial.println("Starting search for modules...");
  module_count = 0;
  module_count_solvable = 0;

  // Why are we cycling through all modules and sending a message for each? Just send one message to all!
  CAN_message[0] = 'P';
  CAN_message[1] = '\0';
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), CAN_message, 1);
  digitalWrite(PIN_LED, LOW);

  delay(1000);

  bool message_waiting = false;
  do { // There are outstanding messages
    CANInbox();
    if (module_detected) {
      module_array[module_count] = (ktomeCAN.can_msg_id & (CAN_ALL_MOD | CAN_MUID_ALL));
      Serial.print("Module count: ");
      Serial.println(module_count + 1);
      Serial.print("Module id: 0b");
      ktomeCAN.padZeros(module_array[module_count]);
      Serial.println(module_array[module_count], BIN);
      if ((module_array[module_count] & (CAN_STD_MOD | CAN_MUID_ALL)) > 0) {
        module_count_solvable++;
      }
      module_count++;
      module_detected = false;
    }
    message_waiting = ktomeCAN.messageWaiting();
  } while (message_waiting);

  Serial.println("All modules polled. Connected modules: ");
  for (byte ii = 0; ii < module_count; ii ++) {
    Serial.print("0b");
    ktomeCAN.padZeros(module_array[ii]);
    Serial.println(module_array[ii], BIN);
  }
  Serial.print("Total connected modules: ");
  Serial.println(module_count);
  Serial.print("Solvable modules: ");
  Serial.println(module_count_solvable);

  for (byte msg_part = 0; msg_part < 2; msg_part++) {
    String BLE_msg;
    byte spec_mod_count;
    BLE_msg = "i ";
    BLE_msg += (msg_part + 1);
    for (byte msg_mod = (1 + (7 * msg_part)); msg_mod < (8 + (7 * msg_part)); msg_mod++) {
      spec_mod_count = 0;
      for (byte msg_num = 0; msg_num < module_count; msg_num++) {
        if ((module_array[msg_num] & CAN_ALL_MOD) == (CAN_MASTER >> msg_mod)) {
          spec_mod_count++;
        }
      }
      BLE_msg += ' ';
      BLE_msg += char(spec_mod_count + '0'); // Space=separated list of numbers corresponding to the amount of modules detected in the bomb
    }
    BLESend(BLE_msg);
    delay(100);
  }

}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

void phoneSetup() {

  game_ready = false;

  serialGenerate(); // Generate serial #
  widgetGenerate(); // Generate widgets

  char CAN_message[9];
  CAN_message[0] = 'W';
  if (serial_vowel) {
    CAN_message[1] = '1';
  } else {
    CAN_message[1] = '0';
  }
  if (serial_odd) {
    CAN_message[2] = '1';
  } else {
    CAN_message[2] = '0';
  }
  CAN_message[3] = (battery_number + '0');
  if (ind_car) {
    CAN_message[4] = '1';
  } else {
    CAN_message[4] = '0';
  }
  if (ind_frk) {
    CAN_message[5] = '1';
  } else {
    CAN_message[5] = '0';
  }
  if (port_parallel) {
    CAN_message[6] = '1';
  } else {
    CAN_message[6] = '0';
  }
  if (port_serial) {
    CAN_message[7] = '1';
  } else {
    CAN_message[7] = '0';
  }
  CAN_message[8] = '\0';

  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), CAN_message, 1);
  digitalWrite(PIN_LED, LOW);

  moduleCheck();

}

void moduleCheck() {

  char CAN_message[2];
  bool manual_error = false;

  // Send request to modules to generate games
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), "I", 1);
  digitalWrite(PIN_LED, LOW);
  delay(750);
  // Need to check that all modules have replied that they have completed their setup!

  byte mods_setup = 0;
  do { // There are outstanding modules to generate a game
    CANInbox();
    if (module_inited) {
      Serial.print("Module 0b");
      ktomeCAN.padZeros(ktomeCAN.can_msg_id - CAN_MASTER);
      Serial.print(ktomeCAN.can_msg_id - CAN_MASTER, BIN);
      Serial.println(" has set-up a scenario!");
      mods_setup++;
      Serial.print(module_count - mods_setup);
      Serial.println(" modules are left to report in.");
      module_inited = false;
    }
  } while (mods_setup != module_count);

  String BLE_msg;

  // Receive games back
  for (byte ii = 0; ii < module_count; ii++) {
    Serial.print("Checking manual for module ");
    Serial.println(ii + 1);
    //    Serial.print("0b");
    //    ktomeCAN.padZeros(module_array[ii] & CAN_MANUALSETUP);
    //    Serial.println(module_array[ii] & CAN_MANUALSETUP, BIN);

    BLE_msg = "c ";

    if ((module_array[ii] & CAN_MANUALSETUP) > 0 ) { // This module needs manual setup
      Serial.println("Needs manual setup!");
      digitalWrite(PIN_LED, HIGH);
      ktomeCAN.send(module_array[ii], "C" , 1);
      digitalWrite(PIN_LED, LOW);

      while (!ktomeCAN.messageWaiting()) {
        delay(100);
        Serial.println("Waiting for module setup response...");
      }
      CANInbox();

      if (module_array[ii] & CAN_WIRES > 0 ) { // Module needing setup is WIRES

      } else if ((module_array[ii] & CAN_BUTTON) > 0 ) {  // Module needing setup is BUTTON

      } else if ((module_array[ii] & CAN_KEYPAD) > 0 ) {  // Module needing setup is KEYPAD
        Serial.println("Keypad setup incoming...");
        byte keypad_r1 = ktomeCAN.can_msg[1] - '0';
        byte keypad_r2 = ktomeCAN.can_msg[2] - '0';
        byte keypad_r3 = ktomeCAN.can_msg[3] - '0';
        byte keypad_r4 = ktomeCAN.can_msg[4] - '0';
        Serial.print("Keypad key IDs: ");
        Serial.print(keypad_r1);
        Serial.print(", ");
        Serial.print(keypad_r2);
        Serial.print(", ");
        Serial.print(keypad_r3);
        Serial.print(", ");
        Serial.println(keypad_r4);
        BLE_msg += 'k';
        for (byte jj = 0; jj < 4; jj++) {
          BLE_msg += ' ';
          BLE_msg += ktomeCAN.can_msg[jj + 1] - '0';
        }
        BLESend(BLE_msg);
        manual_check = false;
        carPark(); // Wait for user to complete manual setup
        if (manual_check) { // User states this module is setup
          digitalWrite(PIN_LED, HIGH);
          ktomeCAN.send(module_array[ii], "M" , 1);
          digitalWrite(PIN_LED, LOW);
        } else {
          Serial.println("Expecting module check, but received another message!");
          manual_error = true;
          break;
        }

      } else if ((module_array[ii] & CAN_CWIRES) > 0 ) { // Module needing setup is COMPLICATED WIRES

      } else if ((module_array[ii] & CAN_WIRESQ) > 0 ) { // Module needing setup is WIRE SEQUENCE

      }

    } else {
      Serial.println("Does not need manual setup!");
    }
    if (manual_error) { // Likely another command came in when expecting a manual setup message from app
      Serial.println("Aborting loop to check modules setup.");
      break;
    }
  }
  if (!manual_error) { // If all went well, send final message to check all manual setup is complete
    BLE_msg = "c Y";
    BLESend(BLE_msg);
  }
}

void widgetGenerate() {

  battery_number = 0;
  port_parallel = false;
  port_serial = false;
  ind_frk = false;
  ind_car = false;

  byte widget_total = 5;
  byte die_roll;
  byte list_indicators[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // SND, CLR, CAR, IND, FRQ, SIG, NSA, MSA, TRN, BOB, FRK
  byte selection_pick;

  String BLE_msg = "c e";

  for (byte i = 0; i < widget_total; i++) {
    die_roll = random(0, 100);
    selection_pick = 0;
    if (die_roll < 30) { // 30% chance of an indicator
      do {
        selection_pick = list_indicators[random(0, 11)]; // Pick from the indicator list
      } while (selection_pick == 0); // Try again if this has already been selected previously
      list_indicators[selection_pick] = 0;
      if (die_roll < 10) {
        selection_pick += 96;
      } else {
        selection_pick += 64;
      }
      if (selection_pick == 24) { // CAR lit
        ind_car = true;
      } else if (selection_pick == 32) { // FRK lit
        ind_frk = true;
      }
      BLE_msg = BLE_msg + " i" + char(selection_pick);
    } else if (die_roll < 65) { // 35% chance of a port
      //      if (random(0, 2) < 1) { // Serial/parallel ports
      if (random(0, 10) < 6) { //60% chance of parallel
        selection_pick += 2;
        port_parallel = true;
      }
      if (random(0, 10) < 5) { //50% chance of a serial
        selection_pick += 1;
        port_serial = true;
      }
      //      } else { // Random ports
      //        selection_pick = 4;
      //      }
      BLE_msg = BLE_msg + " p" + selection_pick;
    } else { // 35% chance of a battery
      if (random(0, 10) < 5) { //50% chance of a D battery
        selection_pick = 1;
        battery_number += 1;
      } else {
        battery_number += 2;
      }
      BLE_msg = BLE_msg + " b" + selection_pick;
    }
  }

  Serial.println(BLE_msg);
  BLESend(BLE_msg);

  Serial.print("Serial port: ");
  Serial.println(port_serial);
  Serial.print("Parallel port: ");
  Serial.println(port_parallel);
  Serial.print("Lit CAR: ");
  Serial.println(ind_car);
  Serial.print("Lit FRK: ");
  Serial.println(ind_frk);
  Serial.print("Batteries: ");
  Serial.println(battery_number);

  manual_check = false;
  carPark(); // Wait for user to complete manual setup
  if (manual_check) { // User states this module is setup
    Serial.println("Widget setup confirmed!");
  } else {
    Serial.println("Expecting module check, but received another message!");
    //    manual_error = true;
    //    break;
  }
}

void serialGenerate () {

  serial_vowel = 0;
  serial_odd = 0;
  serial_even = 0;

  for (int i = 0; i < 6; i++) {
    if (i == 3 || i == 4) {
      serial_number[i] = char_generator(1);
    } else if (i == 2) {
      serial_number[i] = char_generator(0);
    } else if (i == 5) {
      serial_number[i] = char_generator(0);
      if (serial_number[i] % 2 == 1) {
        serial_even = 1;
      } else {
        serial_odd = 1;
      }
    } else {
      serial_number[i] = char_generator(2);
    }
  }
  serial_number[6] = '\0';
  Serial.print(F("Serial #: "));
  Serial.println(serial_number);

  char can_msg[8];
  can_msg[0] = 'S';
  for (byte i = 0; i < 6; i++) {
    can_msg[i + 1] = serial_number[i];
  }
  can_msg[7] = '\0';
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send(CAN_WIDGETS, can_msg, 7);
  digitalWrite(PIN_LED, LOW);

  Serial.print("Vowels?: ");
  Serial.println(serial_vowel);
  Serial.print("Even?: ");
  Serial.println(serial_odd);
  Serial.print("Odds?: ");
  Serial.println(serial_even);
}

char char_generator(bool alphanum_type) {
  char temp;
  byte dice_roll;

  if (alphanum_type == 0) {
    dice_roll = random(0, 10);
  } else if (alphanum_type == 1) {
    dice_roll = random(10, 35);
  } else {
    dice_roll = random(0, 35);
  }

  if (dice_roll < 10) {
    temp = dice_roll + '0';
  } else {
    temp = dice_roll + 'A' - 10;
    if (temp == 'A' || temp == 'E' || temp == 'I' || temp == 'O' || temp == 'U') {
      serial_vowel = 1;
      if (temp == 'O') {
        temp = 'E';
      }
    } else if (temp == 'Y') {
      temp = 'Z';
    }
  }
  return temp;
}

//**********************************************************************
// FUNCTIONS: Game Managers
//**********************************************************************

void gameReset() {

  game_win = false;
  game_running = false;
  //Set timer
  strike_number = 0;
  if (hardcore_mode) {
    strike_limit = 1;
  } else {
    strike_limit = 3;
  }
  strikeUpdate();
  timerblinkcount = 0;

  memset(module_solved_array, 0, sizeof(module_solved_array));

}

void gameRunning() {

  bool message_waiting = false;

  timeleft = gamelength;
  timerUpdate();
  String BLE_msg = "m ";
  BLE_msg = BLE_msg + module_count_solvable;
  BLESend(BLE_msg);

  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), "A", 1);
  digitalWrite(PIN_LED, LOW);

  delay(random(2000) + 1000);

  game_running = true;
  //  thismillis = millis();
  thismillis = micros();

  while (gamemode == 3) {
    // Loop through all checks to do in a single "frame" when the game is in progress

    // Check CAN inbox
    do {
      CANInbox();
      message_waiting = ktomeCAN.messageWaiting();
    } while (message_waiting);

    // Solve update


    // Strike update
    strikeCalc();

    // Timer update
    timerCalc();

  }
}

void visual_fx(bool target_state) {
  bool draw_colon;
  if (target_state) { // Triggers on explosion - flash lights on and ready for them to switch off
    timerdisp.print(10000);
    timerdisp.writeDisplay();
    draw_colon = false;
    digitalWrite(PIN_STRIKE_1, target_state);
    digitalWrite(PIN_STRIKE_2, target_state);
  } else { // Triggers on defusal and reset
    if (game_win) {
      timerdisp.writeDigitNum(0, '0');
      timerdisp.writeDigitNum(1, '0');
      timerdisp.writeDigitNum(3, '0');
      timerdisp.writeDigitNum(4, '0');
      timerblinktime = millis() + timerblinkperiod;
      timerblinkbool = false;
      draw_colon = false;
    } else {
      timerdisp.print(10000);
      timerdisp.writeDisplay();
      draw_colon = false;
      digitalWrite(PIN_STRIKE_1, target_state);
      digitalWrite(PIN_STRIKE_2, target_state);
    }
  }
  timerdisp.drawColon(draw_colon);
  timerdisp.writeDisplay();
  
}

//**********************************************************************
// FUNCTIONS: Game functions
//**********************************************************************

void timerCalc() {

  //  int32_t delta_t = millis() - thismillis;
  int32_t delta_t = micros() - thismillis;
  thismillis += delta_t;
  timeleft = timeleft - (delta_t * (1 + strike_number * 0.25)); // Updates the time left with the change in the time since last updating, modified by the time scale due to strikes
  //  Serial.print("Dilation: ");
  //  Serial.print(1 + strike_number * 0.25);
  //  Serial.print( " | Delta t: ");
  //  Serial.println(delta_t * (1 + strike_number * 0.25));
  //  Serial.println(micros());
  
//  Serial.print(timeleft);
//  Serial.print(" -> ");
//  Serial.println(timeleft / 1000);
//  Serial.print(" -> ");
//  Serial.println(timeleft / 1000000);

  if (timeleft < 0) {
    timeleft = 0;
    // Lose Game!
    strike_culprit = CAN_MASTER;
    gamemode = 4;
  }
  timerUpdate();

}

void strikeCalc() {

  if (strike_flag) { //
    strike_flag = false;

    strike_number++;
    Serial.print("Strike from 0b");
    ktomeCAN.padZeros(strike_culprit);
    Serial.println(strike_culprit, BIN);
    Serial.print("Current number of strikes: ");
    Serial.println(strike_number);

    String BLE_msg = "x ";
    BLE_msg = BLE_msg + strike_number;
    BLESend(BLE_msg);

    if (strike_number == 2) {
      blinktime = millis() + blinkperiod;
      blinkbool = true;
    }

    if (strike_number < strike_limit) {
      // Play buzzer
      char CAN_message[3] = "X ";
      CAN_message[1] = '0' + strike_number;
      digitalWrite(PIN_LED, HIGH);
      ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), CAN_message, 2);
      digitalWrite(PIN_LED, LOW);

    } else {
      // Game over
      gamemode = 4; // Send to wash-up
    }
  }
  strikeUpdate();
}

void solveUpdate(long id) {

  byte solved_count = 0;
  Serial.print("Defused module is 0b");
  ktomeCAN.padZeros(id);
  Serial.println(id, BIN);

  for (byte ii = 0; ii < 12; ii++) {
    Serial.println((id & (CAN_ALL_MOD | CAN_MUID_ALL)), BIN);
    Serial.println(module_array[ii], BIN);
    if ((id & (CAN_ALL_MOD | CAN_MUID_ALL)) == module_array[ii]) {
      module_solved_array[ii] = true;
      Serial.println("FOUND");
    }
    if (module_solved_array[ii]) {
      solved_count++;
    }
  }

  Serial.print("Number of solved modules: ");
  Serial.println(solved_count);

  if (solved_count == module_count_solvable) {
    // Game win condition
    Serial.println("Game won! All solvable modules have been defused.");
    gamemode = 4;
    game_win = true;
    // Do a timer blink and pass debrief message to app

  }

}

void stopMessages() {

  char CAN_message[] = "Z0";
  String BLE_msg = "";
  game_running = false;
  if (game_win) { // Game is won
    BLE_msg = "z d";
  } else if (strike_number == strike_limit) { // Game lost due to strike-out
    BLE_msg = "z ";
    for (byte ii = 0; ii < 15; ii++) {
      if ((((strike_culprit & CAN_STD_MOD) << ii) & CAN_STD_MOD ) == 0) {
        BLE_msg = BLE_msg + ii;
        ii = 15;
      }
    }
    CAN_message[1] = '1';
    timerblinktime = thismillis + 100;
  } else if (timeleft == 0) { //  Game lost due to timer
    BLE_msg = "z 0";
    CAN_message[1] = '1';
    timerblinktime = thismillis + 100;
  } else { // Game aborted
    BLE_msg = "z Z";
  }
  BLESend(BLE_msg);
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send((CAN_ALL_MOD | CAN_MUID_ALL), CAN_message, 2);
  digitalWrite(PIN_LED, LOW);
}

//**********************************************************************
// FUNCTIONS: Hardware
//**********************************************************************

void timerInit() {

  timerdisp.begin(0x70); // Timer uses I2C pins @ 0x70 (Data = 21, Clock = 22)
  timerdisp.print(10000);
  timerdisp.writeDisplay();

}

void timerUpdate() {

  bool draw_colon = true;
  char holding_digit;
  bool time_string = false;

  //  if (timeleft >= 60000) { // Over 1 minute on the clock...
  //    holding_digit = (int)(timeleft / 600000) + 48;
  //    timestr.setCharAt(0, holding_digit);
  //    holding_digit = (int)((timeleft % 600000) / 60000) + 48;
  //    timestr.setCharAt(1, holding_digit);
  //    holding_digit = (int)((timeleft % 60000) / 10000) + 48;
  //    timestr.setCharAt(2, holding_digit);
  //    holding_digit = (int)((timeleft % 10000) / 1000) + 48;
  //    timestr.setCharAt(3, holding_digit);
  if (timeleft >= 60000000) { // Over 1 minute on the clock...
    holding_digit = (int32_t)(timeleft / 600000000) + 48;
    timestr.setCharAt(0, holding_digit);
    holding_digit = (int32_t)((timeleft % 600000000) / 60000000) + 48;
    timestr.setCharAt(1, holding_digit);
    holding_digit = (int32_t)((timeleft % 60000000) / 10000000) + 48;
    timestr.setCharAt(2, holding_digit);
    holding_digit = (int32_t)((timeleft % 10000000) / 1000000) + 48;
    timestr.setCharAt(3, holding_digit);
  }
  else if (timeleft == 0) { // Timer ran out
    //    timeleft = 0;
    timestr = "0000";
    draw_colon = false;
    time_string = true;
  }
  //  else { // Under 1 minute left...
  //    holding_digit = (int)(timeleft / 10000) + 48;
  //    timestr.setCharAt(0, holding_digit);
  //    holding_digit = (int)((timeleft % 10000) / 1000) + 48;
  //    timestr.setCharAt(1, holding_digit);
  //    holding_digit = (int)((timeleft % 1000) / 100) + 48;
  //    timestr.setCharAt(2, holding_digit);
  //    holding_digit = (int)((timeleft % 100) / 10) + 48;
  //    timestr.setCharAt(3, holding_digit);
  else { // Under 1 minute left...
    holding_digit = (int32_t)(timeleft / 10000000) + 48;
    timestr.setCharAt(0, holding_digit);
    holding_digit = (int32_t)((timeleft % 10000000) / 1000000) + 48;
    timestr.setCharAt(1, holding_digit);
    holding_digit = (int32_t)((timeleft % 1000000) / 100000) + 48;
    timestr.setCharAt(2, holding_digit);
    holding_digit = (int32_t)((timeleft % 100000) / 10000) + 48;
    timestr.setCharAt(3, holding_digit);
  }
  if (time_string) {
    timerdisp.print(10000);
  } else {
    timerdisp.writeDigitNum(0, (int)(timestr.charAt(0) - 48));
    timerdisp.writeDigitNum(1, (int)(timestr.charAt(1) - 48));
    timerdisp.writeDigitNum(3, (int)(timestr.charAt(2) - 48));
    timerdisp.writeDigitNum(4, (int)(timestr.charAt(3) - 48));
  }
  timerdisp.drawColon(draw_colon);
  timerdisp.writeDisplay();

  String BLE_msg;
  BLE_msg = "t ";
  if (timeleft >= 60000000) { // Over a minute left - look at whole timer string
    if (timestr != timestr_prev) { // second has ticked on
      BLE_msg = BLE_msg + timestr.charAt(0) + timestr.charAt(1) + " " + timestr.charAt(2) + timestr.charAt(3);
      BLESend(BLE_msg);
      if (game_running) {
        digitalWrite(PIN_LED, HIGH);
        ktomeCAN.send((CAN_MASTER), "H", 1);
        digitalWrite(PIN_LED, LOW);
      }
    }
  } else { // Under a minute, just look at first two chars
    if (timestr.charAt(0) != timestr_prev.charAt(0) || timestr.charAt(1) != timestr_prev.charAt(1)) { // second has ticked on
      BLE_msg = BLE_msg + "00 " + timestr.charAt(0) + timestr.charAt(1);
      BLESend(BLE_msg);
      if (game_running) {
        digitalWrite(PIN_LED, HIGH);
        ktomeCAN.send((CAN_MASTER), "H", 1);
        digitalWrite(PIN_LED, LOW);
      }
    }
  }
  timestr_prev = timestr;
}

void strikeUpdate() {
  switch (strike_number) {
    case 0:
      digitalWrite(PIN_STRIKE_1, LOW);
      digitalWrite(PIN_STRIKE_2, LOW);
      break;
    case 1:
      digitalWrite(PIN_STRIKE_1, HIGH);
      digitalWrite(PIN_STRIKE_2, LOW);
      break;
    case 2:
      if (blinktime <= millis()) {
        blinkbool = !blinkbool;
        digitalWrite(PIN_STRIKE_1, blinkbool);
        digitalWrite(PIN_STRIKE_2, blinkbool);
        blinktime = millis() + blinkperiod;
      }
      break;
    case 3:
      digitalWrite(PIN_STRIKE_1, HIGH);
      digitalWrite(PIN_STRIKE_2, HIGH);
      break;
  }
}

//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox() {
  if (ktomeCAN.messageWaiting()) { // Outstanding messages to handle
    ktomeCAN.receive();
    if (ktomeCAN.can_msg[0] == 'p' && gamemode == 1) {
      module_detected = true;
      Serial.println("Module detected!");
    }
    else if (ktomeCAN.can_msg[0] == 'i' && gamemode == 2) {
      module_inited = true;
      Serial.println("Module has declared it is setup!");
    }
    else if (ktomeCAN.can_msg[0] == 'c' && gamemode == 2) {
      Serial.println("Module is transmitting it's manual setup needs!");
    }
    else if (ktomeCAN.can_msg[0] == 'x' && gamemode == 3) {
      Serial.println("Module announces a strike!");
      strike_flag = true;
      strike_culprit = ktomeCAN.can_msg_id;
    }
    else if (ktomeCAN.can_msg[0] == 'd' && gamemode == 3) {
      Serial.println("Module announces its defusal!");
      solveUpdate(ktomeCAN.can_msg_id);
    }

  }
}

void BLESend(String msg_data) {
  String BLE_output(msg_data);
  Serial.print("Sending \"");
  Serial.print(msg_data);
  Serial.println("\" to phone app...");
  pTxCharacteristic->setValue(BLE_output.c_str());
  pTxCharacteristic->notify();
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
