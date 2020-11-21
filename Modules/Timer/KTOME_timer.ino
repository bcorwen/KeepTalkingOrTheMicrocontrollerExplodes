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
#include <BLE2902.h>
#include <Wire.h>
#include <CAN.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_LEDBackpack.h>
//#include <Entropy.h>
//#include <LiquidCrystal.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
#define PIN_CAN_TX  GPIO_NUM_26
#define PIN_CAN_RX  GPIO_NUM_27
//#define PIN_PIEZO   GPIO_NUM_2
#define PIN_LED     GPIO_NUM_12
#define PIN_BUTTON  GPIO_NUM_36

//Adafruit_7segment timer_display = Adafruit_7segment(); // Default pins: I2C SCL = GPIO22, I2C SDA = CPIO21

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool manual_check = false;

int module_array[12];
bool module_detected = false;
bool module_inited = false;
byte module_count = 0;
byte spec_mod_count = 0;

// Timer
long timeleft;
int gamelength = 300; //seconds
long thismillis;
//long delta_t;
char timestr[5] = "----";
bool hardcore_mode = false;
byte strike_number = 0;
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
//
//byte battery_number;
//byte port_number;
//byte ind_number;
//byte widget_types[5];
//
//bool parallel_port;
//bool ind_frk;
//bool ind_car;

// CAN                      [ Module type ][ID][ unused ]
#define CAN_ID            0b10000000000000000000000000000 // ID for Master (timer)
#define CAN_MASK          0b10000000000000000000000000000 // Filter for the Master (timer)

#define CAN_TO_MASTER     0b10000000000000000000000000000
#define CAN_TO_WIRES      0b01000000000000000000000000000
#define CAN_TO_BUTTON     0b00100000000000000000000000000
#define CAN_TO_KEYPAD     0b00010000000000000000000000000
#define CAN_TO_SIMON      0b00001000000000000000000000000
#define CAN_TO_WHOS       0b00000100000000000000000000000
#define CAN_TO_MEMORY     0b00000010000000000000000000000
#define CAN_TO_MORSE      0b00000001000000000000000000000
#define CAN_TO_CWIRES     0b00000000100000000000000000000
#define CAN_TO_WIRESQ     0b00000000010000000000000000000
#define CAN_TO_MAZE       0b00000000001000000000000000000
#define CAN_TO_PASSWORD   0b00000000000100000000000000000
#define CAN_TO_VENT       0b00000000000010000000000000000
#define CAN_TO_CAPACITOR  0b00000000000001000000000000000
#define CAN_TO_KNOB       0b00000000000000100000000000000
#define CAN_TO_STD_MOD    0b01111111111100000000000000000
#define CAN_TO_NEEDY_MOD  0b00000000000011100000000000000
#define CAN_TO_ALL_MOD    0b01111111111111100000000000000
#define CAN_MUID_1        0b00000000000000010000000000000
#define CAN_MUID_2        0b00000000000000001000000000000
#define CAN_MUID_3        0b00000000000000000100000000000
#define CAN_MUID_4        0b00000000000000000010000000000
#define CAN_MUID_ALL      0b00000000000000011110000000000

#define CAN_MANUALSETUP   0b01110000110000000000000000000

char buffer_msg[9][16];
int buffer_can_id[16];
byte buffer_pointer_r;
byte buffer_pointer_w;
char msg_received[9];
int msg_rec_id;

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
      gamemode = 0;
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

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Timer ==");

  // Start CAN bus
  CAN.setPins(PIN_CAN_RX, PIN_CAN_TX);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  //  CAN.filterExtended(CAN_ID, CAN_MASK);
  CAN.onReceive(onReceive);

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

  pinMode(PIN_BUTTON, INPUT_PULLUP);

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
      carPark();
      break;
    case 3: // Game running
      Serial.println(F("Game starting!"));
      gameRunning();
      break;
    case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone

      break;
  }
}

void carPark() {
  holding = true;
  Serial.println("Script parked - waiting for direction...");
  while (holding) {
    delay (1);
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

  //  delay(500);

  byte max_module_copies = 4;
  char CAN_message[2];
  memset(module_array, 0, sizeof(module_array));

  Serial.println("Starting search for modules...");
  module_count = 0;

  // Why are we cycling through all modules and sending a message for each? Just send one message to all!
  CAN_message[0] = 'P';
  CAN_message[1] = '\0';
  CANSend((CAN_TO_ALL_MOD | CAN_MUID_ALL), CAN_message, 1);

  delay(1000);

  do { // There are outstanding messages
    CANReceive();
    if (module_detected) {
      module_array[module_count] = (msg_rec_id & (CAN_TO_ALL_MOD | CAN_MUID_ALL));
      Serial.print("Module count: ");
      Serial.println(module_count + 1);
      Serial.print("Module id: 0b");
      padZerosCAN(module_array[module_count]);
      Serial.println(module_array[module_count], BIN);
      module_count++;
    }
  } while (buffer_pointer_r != buffer_pointer_w);

  Serial.println("All modules polled. Connected modules: ");
  for (byte ii = 0; ii < module_count; ii ++) {
    Serial.print("0b");
    padZerosCAN(module_array[ii]);
    Serial.println(module_array[ii], BIN);
  }

  for (byte msg_part = 0; msg_part < 2; msg_part++) {
    String BLE_msg;
    BLE_msg = "i ";
    BLE_msg += (msg_part + 1);
    for (byte msg_mod = (1 + (7 * msg_part)); msg_mod < (8 + (7 * msg_part)); msg_mod++) {
      spec_mod_count = 0;
      for (byte msg_num = 0; msg_num < module_count; msg_num++) {
        if ((module_array[msg_num] & CAN_TO_ALL_MOD) == (CAN_TO_MASTER >> msg_mod)) {
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

  bool game_ready = false;

  widgetGenerate(); // Generate widgets
  serialGenerate(); // Generate serial #

  moduleCheck();

  long timer_int;

  //  do {
  //
  //    thismillis = millis();
  //    bool led_state;
  //
  //    if (thismillis > timer_int) {
  //
  //      if (led_state) {
  //        digitalWrite(PIN_LED, LOW);
  //        led_state = false;
  //        timer_int = thismillis + 800;
  //      } else {
  //        digitalWrite(PIN_LED, HIGH);
  //        led_state = true;
  //        timer_int = thismillis + 200;
  //      }
  //    }
  //
  //  } while (!game_ready && (gamemode == 2));

}

void moduleCheck() {

  char CAN_message[2];
  bool manual_error = false;

  // Send request to modules to generate games
  CANSend((CAN_TO_ALL_MOD | CAN_MUID_ALL), "I", 1);
  delay(750);
  // Need to check that all modules have replied that they have completed their setup!

  byte mods_setup = 0;
  do { // There are outstanding modules to generate a game
    CANReceive();
    if (module_inited) {
      Serial.print("Module 0b");
      padZerosCAN(msg_rec_id - CAN_TO_MASTER);
      Serial.print(msg_rec_id - CAN_TO_MASTER, BIN);
      Serial.println(" has set-up a scenario!");
      mods_setup++;
      Serial.print(module_count - mods_setup);
      Serial.println(" modules are left to report in.");
    }
  } while (mods_setup != module_count);

  String BLE_msg;

  // Receive games back
  for (byte ii = 0; ii < module_count; ii++) {
    Serial.print("Checking manual for module ");
    Serial.println(ii + 1);
    //    Serial.print("0b");
    //    padZerosCAN(module_array[ii] & CAN_MANUALSETUP);
    //    Serial.println(module_array[ii] & CAN_MANUALSETUP, BIN);

    BLE_msg = "c ";

    if ((module_array[ii] & CAN_MANUALSETUP) > 0 ) { // This module needs manual setup
      Serial.println("Needs manual setup!");
      CAN_message[0] = 'C';
      CAN_message[1] = '\0';
      CANSend(module_array[ii], CAN_message, 1);
      while (buffer_pointer_r == buffer_pointer_w) {
        delay(500);
        Serial.println("Waiting for module setup response...");
      }
      CANReceive();
      if (module_array[ii] & CAN_TO_WIRES > 0 ) { // Module needing setup is WIRES

      } else if ((module_array[ii] & CAN_TO_BUTTON) > 0 ) {  // Module needing setup is BUTTON

      } else if ((module_array[ii] & CAN_TO_KEYPAD) > 0 ) {  // Module needing setup is KEYPAD
        Serial.println("Keypad setup incoming...");
        byte keypad_r1 = msg_received[1] - '0';
        byte keypad_r2 = msg_received[2] - '0';
        byte keypad_r3 = msg_received[3] - '0';
        byte keypad_r4 = msg_received[4] - '0';
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
          BLE_msg += msg_received[jj + 1] - '0';
        }
        BLESend(BLE_msg);
        manual_check = false;
        carPark(); // Wait for user to complete manual setup
        if (manual_check) { // User states this module is setup

        } else {
          Serial.println("Expecting module check, but received another message!");
          manual_error = true;
          break;
        }

      } else if ((module_array[ii] & CAN_TO_CWIRES) > 0 ) { // Module needing setup is COMPLICATED WIRES

      } else if ((module_array[ii] & CAN_TO_WIRESQ) > 0 ) { // Module needing setup is WIRE SEQUENCE

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

void widgetGenerate () {
  //  byte widget_number = random(2) + 4;
  //  battery_number = random(3) + random(3); // Number from 0-4, with a bias towards 2
  //  if (widget_number > battery_number) {
  //    ind_number = random(1 + widget_number - battery_number);
  //    port_number = widget_number - (battery_number + ind_number);
  //  } else {
  //    port_number = 0;
  //    ind_number = 0;
  //  }
  //  ind_car = false;
  //  ind_frk = false;
  //  bool isthisacopy = false;
  //  lcd.clear();
  //  lcd.setCursor(0, 0);
  //  for (byte i = 0; i < widget_number; i++) {
  //    if (i < battery_number) { // Add battery
  //      widget_types[i] = random(2); // AA or D
  //    } else if (i < (battery_number + ind_number)) { // Add indicator
  //      if (i > 0) {
  //        do {
  //          isthisacopy = false;
  //          widget_types[i] = random(23) + 10; // All inds (11 types), off and on (x2)
  //          for (byte j = 0; j < i; j++) {
  //            if (widget_types[i] % 2 == widget_types[j] % 2) {
  //              isthisacopy = true;
  //            }
  //          }
  //        } while (isthisacopy = true);
  //        if (widget_types[i] == 15) {
  //          ind_car = true; // 14 = off, 15 = on
  //        } else if (widget_types[i] == 17) {
  //          ind_frk = true; // 16 = off, 17 = on
  //        }
  //      }
  //    } else { // Add port
  //      widget_types[i] = random(4) + 40; // Parallel, serial, other
  //      if (widget_types[i] == 40) {
  //        parallel_port = true;
  //      }
  //    }
  //    if (i > 0) {
  //      lcd.print(":");
  //    }
  //    //    lcd.sprintf(widget_types[i], "%02d");
  //    lcd.print(widget_types[i], DEC);
  //  }
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
  //    Serial.print("Vowels?: ");
  //    Serial.println(serial_vowel);
  //    Serial.print("Even?: ");
  //    Serial.println(serial_odd);
  //    Serial.print("Odds?: ");
  //    Serial.println(serial_even);
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

// Managing timer

// Listening to modules

// Game win/loss

void gameRunning() {

  bool game_running = true;

  thismillis = millis();

  while (game_running) {

    // Loop through all checks to do in a single "frame" when the game is in progress

    // Strike update
    // Game win/loss check
    // Timer update
    timerUpdate();

  }



}

//**********************************************************************
// FUNCTIONS: Game functions
//**********************************************************************

void timerUpdate() {

  long delta_t = millis() - thismillis;
  thismillis += delta_t;

  timeleft = timeleft - (delta_t * (1 + strike_number * 0.25)); // Updates the time left with the change in the time since last updating, modified by the time scale due to strikes

  if (timeleft >= 60000) { // Over 1 minute on the clock...
    timestr[0] = (int)(timeleft / 600000) + 48;
    timestr[1] = (int)((timeleft % 600000) / 60000) + 48;
    timestr[2] = (int)((timeleft % 60000) / 10000) + 48;
    timestr[3] = (int)((timeleft % 10000) / 1000) + 48;
  }
  else if (timeleft < 0) { // Timer ran out
    timeleft = 0;
    //      for (int i = 0; i < 4; i++) {
    //        timestr[i] = "0";
    //      }
    timestr[0] = '-';
    timestr[1] = '-';
    timestr[2] = '-';
    timestr[3] = '-';
    //    game_lose(1);
  }
  else { // Under 1 minute left...
    timestr[0] = (int)(timeleft / 10000) + 48;
    timestr[1] = (int)((timeleft % 10000) / 1000) + 48;
    timestr[2] = (int)((timeleft % 1000) / 100) + 48;
    timestr[3] = (int)((timeleft % 100) / 10) + 48;
  }

}

void timerWrite() { // Write "timestr" char array to the timer display

  //  timer_display.writeDigitNum(0, (int)(timestr[0] - 48));
  //  timer_display.writeDigitNum(1, (int)(timestr[1] - 48));
  //  timer_display.writeDigitNum(3, (int)(timestr[2] - 48));
  //  timer_display.writeDigitNum(4, (int)(timestr[3] - 48));
  //  timer_display.drawColon(true);
  //  timer_display.writeDisplay();

}




//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANSend(int id, char msg_data[], byte msg_len) {
  Serial.print("Sending packet ... ");
  digitalWrite(PIN_LED, HIGH);

  CAN.beginExtendedPacket(id);
  for (int i = 0; i < msg_len; i++) {
    CAN.write(msg_data[i]);
  }
  //  CAN.write(msg_data, msg_len);
  CAN.endPacket();

  Serial.print(" with id: 0b");
  padZerosCAN(id);
  Serial.print(id, BIN);
  Serial.print(" - \"");
  Serial.print(msg_data);
  Serial.println("\"");
  digitalWrite(PIN_LED, LOW);
}

void onReceive(int packet_size) {

  if ((CAN.packetId() & CAN_MASK) == CAN_ID) {

    for (byte i = 0; i < packet_size; i++) {
      //    msg_received[i] = CAN.read();
      buffer_msg[i][buffer_pointer_w] = CAN.read();
    }
    //  msg_received[packet_size] = '\0';
    buffer_msg[packet_size][buffer_pointer_w] = '\0';

    buffer_can_id[buffer_pointer_w] = CAN.packetId();

    if (buffer_pointer_w == 15) {
      buffer_pointer_w = 0;
    } else {
      buffer_pointer_w++;
    }

  }

}

void CANReceive() {

  Serial.print("Read pointer: ");
  Serial.println(buffer_pointer_r);
  Serial.print("Write pointer: ");
  Serial.println(buffer_pointer_w);
  if (buffer_pointer_r != buffer_pointer_w) { // Outstanding messages to handle
    for (byte ii = 0; ii < 9; ii++) {
      msg_received[ii] = buffer_msg[ii][buffer_pointer_r];
    }
    msg_rec_id = buffer_can_id[buffer_pointer_r];

    if (buffer_pointer_r == 15) {
      buffer_pointer_r = 0;
    } else {
      buffer_pointer_r++;
    }

    Serial.print("Received ");
    Serial.print("packet with id 0b");
    padZerosCAN(msg_rec_id);
    Serial.print(msg_rec_id, BIN);

    Serial.print(" - \"");
    Serial.print(msg_received);
    Serial.println("\"");

    if (msg_received[0] == 'p' && gamemode == 1) {
      module_detected = true;
      Serial.println("Module detected!");
    }
    else if (msg_received[0] == 'i' && gamemode == 2) {
      module_inited = true;
      Serial.println("Module has declare it is setup!");
    }
    else if (msg_received[0] == 'c' && gamemode == 2) {
      Serial.println("Module is transmitting it's manual setup needs!");
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

void padZerosCAN(int id) {
  for (byte ii = 28; ii > 0; ii--) {
    if (id < (1 << ii)) {
      Serial.print("0");
    }
  }
}
 
