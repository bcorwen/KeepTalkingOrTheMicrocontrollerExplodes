//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 23/12/20
//======================================================================
//
//  Module: The Keypad module (SLAVE)
//
//  version 0.2.0
//
//  Goal for this version: Get this thing going!
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_LEDBackpack.h>
//#include <Entropy.h>
//#include <LiquidCrystal.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
//#define PIN_PIEZO     GPIO_NUM_2
#define PIN_LED       GPIO_NUM_12
#define PIN_BUTTON_1  GPIO_NUM_32
#define PIN_BUTTON_2  GPIO_NUM_33

long button_1_debounce;
bool button_1_state;
long button_2_debounce;
bool button_2_state;
long button_debounce = 200;

//#define MOD_STATUS_R   GPIO_NUM_32
//#define MOD_STATUS_G   GPIO_NUM_33

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool game_running = false;

// Timer
long thismillis;
byte strikenumber = 0;

// Widgets
//char serial_number[7];
bool serial_vowel;
bool serial_odd;
bool serial_even;
byte battery_number;
bool parallel_port;
bool ind_frk;
bool ind_car;

// Keypad specific
#define KEYPAD_B1   GPIO_NUM_15
#define KEYPAD_B2   GPIO_NUM_2
#define KEYPAD_B3   GPIO_NUM_0
#define KEYPAD_B4   GPIO_NUM_4
#define KEYPAD_LR1   GPIO_NUM_16
#define KEYPAD_LR2   GPIO_NUM_17
#define KEYPAD_LR3   GPIO_NUM_5
#define KEYPAD_LR4   GPIO_NUM_18
#define KEYPAD_LG1   GPIO_NUM_19
#define KEYPAD_LG2   GPIO_NUM_21
#define KEYPAD_LG3   GPIO_NUM_22
#define KEYPAD_LG4   GPIO_NUM_23
byte keypad_stage;
byte keypad_symbols[4];
byte symbol_index[7];
byte keypad_order[4];
byte keypad_column_choice;
bool keypad_track_state;

// CAN                      [ Module type ][ID][ unused ]

//#define CAN_ID            0b00010000000000001000000000000 // ID for "Keypad" + 0100
//#define CAN_MASK          0b10010000000000001000000000000 // Filter for the "Keypad"
//
#define CAN_ID            0b00010000000000010000000000000 // ID for "Keypad" + 1000
#define CAN_MASK          0b10010000000000010000000000000 // Filter for the "Keypad"

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Module (Keypad) ==");

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

  // Randomiser
  //  Entropy.initialize();
  //  randomSeed(Entropy.random());
  esp_random();

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  button_1_state = digitalRead(PIN_BUTTON_1);
  button_2_state = digitalRead(PIN_BUTTON_2);
  Serial.println(button_1_state);
  Serial.println(button_2_state);
}

void loop() {

  switch (gamemode) {
    case 0:
      carPark();
      break;
    case 1: // Module poll
      Serial.println(F("Module search..."));
      initialisation();
      carPark();
      break;
    case 2: // Game in set-up
      Serial.println(F("Game set-up..."));
      //      gameSetup();
      carPark();
      break;
    case 3: // Game running
      Serial.println(F("Game starting!"));
      game_running = true;
      gameRunning();
      break;
    case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone
      game_ready = false;
      carPark();
      break;
  }
}

void carPark() {
  holding = true;
  Serial.println("Script parked - waiting for direction...");
  while (holding) {
    do {
      CANInbox();
    } while (ktomeCAN.messageWaiting());
    delay (1);
  }
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : gamemode = 1
//**********************************************************************

// Comm with modules to determine what's connected

void initialisation() {

  char CAN_message[2];
  CAN_message[0] = 'p';
  CAN_message[1] = '\0';
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send(CAN_MASTER | CAN_ID, CAN_message, 1);
  digitalWrite(PIN_LED, LOW);

}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

void gameSetup() {

  game_ready = false;

  int packet_size;

  keypad_setup();

  game_ready = true;

}

void keypad_place() {
  char CAN_message[6];
  CAN_message[0] = 'c';
  CAN_message[1] = symbol_index[keypad_symbols[0]] + '0';
  CAN_message[2] = symbol_index[keypad_symbols[1]] + '0';
  CAN_message[3] = symbol_index[keypad_symbols[2]] + '0';
  CAN_message[4] = symbol_index[keypad_symbols[3]] + '0';
  CAN_message[5] = '\0';
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send(CAN_MASTER | CAN_ID, CAN_message, 5);
  digitalWrite(PIN_LED, LOW);
}

void keypad_setup() {

  byte col_select = B00000000;
  byte symbols_picked = 0;
  byte this_pick;

  keypad_column_choice = random(6);
  Serial.print(F("Keypad column: "));
  Serial.println(keypad_column_choice);

  switch (keypad_column_choice) {
    case 0:
      symbol_index[0] = 25;
      symbol_index[1] = 12;
      symbol_index[2] = 26;
      symbol_index[3] = 11;
      symbol_index[4] = 7;
      symbol_index[5] = 9;
      symbol_index[6] = 21;
      break;
    case 1:
      symbol_index[0] = 15;
      symbol_index[1] = 25;
      symbol_index[2] = 21;
      symbol_index[3] = 23;
      symbol_index[4] = 3;
      symbol_index[5] = 9;
      symbol_index[6] = 18;
      break;
    case 2:
      symbol_index[0] = 1;
      symbol_index[1] = 8;
      symbol_index[2] = 23;
      symbol_index[3] = 5;
      symbol_index[4] = 14;
      symbol_index[5] = 26;
      symbol_index[6] = 3;
      break;
    case 3:
      symbol_index[0] = 10;
      symbol_index[1] = 19;
      symbol_index[2] = 27;
      symbol_index[3] = 7;
      symbol_index[4] = 5;
      symbol_index[5] = 18;
      symbol_index[6] = 4;
      break;
    case 4:
      symbol_index[0] = 22;
      symbol_index[1] = 4;
      symbol_index[2] = 27;
      symbol_index[3] = 21;
      symbol_index[4] = 19;
      symbol_index[5] = 17;
      symbol_index[6] = 2;
      break;
    case 5:
      symbol_index[0] = 10;
      symbol_index[1] = 15;
      symbol_index[2] = 24;
      symbol_index[3] = 13;
      symbol_index[4] = 22;
      symbol_index[5] = 16;
      symbol_index[6] = 6;
      break;
  }

  while (symbols_picked < 4) {
    this_pick = keypad_picker(col_select);
    if (this_pick < 7) {
      keypad_symbols[symbols_picked] = this_pick;
      bitSet(col_select, this_pick);
      symbols_picked++;
    }
  }
  Serial.print(keypad_symbols[0]);
  Serial.print(" ");
  Serial.print(keypad_symbols[1]);
  Serial.print(" ");
  Serial.print(keypad_symbols[2]);
  Serial.print(" ");
  Serial.println(keypad_symbols[3]);

  Serial.print(symbol_index[keypad_symbols[0]]);
  Serial.print(" ");
  Serial.print(symbol_index[keypad_symbols[1]]);
  Serial.print(" ");
  Serial.print(symbol_index[keypad_symbols[2]]);
  Serial.print(" ");
  Serial.println(symbol_index[keypad_symbols[3]]);

  byte greater_number;
  for (byte i = 0; i < 4; i++) { // i is the element we are looking at
    greater_number = 0;
    for (byte j = 0; j < 4 ; j++) { // j is the other element we are comparing against
      if (keypad_symbols[i] > keypad_symbols[j]) {
        greater_number++;
      }
    }
    keypad_order[i] = greater_number;
  }

  char CAN_message[2];
  CAN_message[0] = 'i';
  CAN_message[1] = '\0';
  digitalWrite(PIN_LED, HIGH);
  ktomeCAN.send(CAN_MASTER | CAN_ID, CAN_message, 1);
  digitalWrite(PIN_LED, LOW);

}

byte keypad_picker(byte col_select) {
  byte looking_at_symbol;
  byte results;
  looking_at_symbol = random(7); // pick a number from 0-6 to choose from the symbol column.
  if (bitRead(col_select, looking_at_symbol) == 0) { //if this symbol hasn't been picked yet
    //    bitSet(col_select, looking_at_symbol);
    results = looking_at_symbol;
  } else {
    results = 7;
  }
  //  Serial.println(col_select, BIN);
  //  Serial.println(looking_at_symbol);
  return results;
}

//**********************************************************************
// FUNCTIONS: Game Managers
//**********************************************************************

// Managing timer

// Listening to modules

// Game win/loss

void gameRunning() {

  while (gamemode == 3) {

    thismillis = millis();

    // Input check
    if (digitalRead(PIN_BUTTON_1) != button_1_state) {
      if (button_1_debounce <= thismillis) {
        button_1_state = digitalRead(PIN_BUTTON_1);
        button_1_debounce = thismillis + button_debounce;
        if (button_1_state == LOW) {
          ktomeCAN.send(CAN_MASTER | CAN_ID, "x", 1);
          Serial.println("Red button pressed!");
        }
      }
    }
    Serial.print("RED button: ");
    Serial.println(button_1_state);
    if (digitalRead(PIN_BUTTON_2) != button_1_state) {
      if (button_2_debounce <= thismillis) {
        button_2_state = digitalRead(PIN_BUTTON_2);
        button_2_debounce = thismillis + button_debounce;
        if (button_2_state == LOW) {
          ktomeCAN.send(CAN_MASTER | CAN_ID, "d", 1);
          Serial.println("Green button pressed!");
        }
      }
    }
    Serial.print("GRN button: ");
    Serial.println(button_2_state);
    
    // Output update

    // Send CAN message
    CANInbox();

  }



}

//**********************************************************************
// FUNCTIONS: Game functions
//**********************************************************************






//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox() {
  if (ktomeCAN.messageWaiting()) { // Outstanding messages to handle
    ktomeCAN.receive();
    if (ktomeCAN.can_msg[0] == 'P') { // Init call
      Serial.println("This module is present!");
      //      digitalWrite(PIN_LED, HIGH);
      gamemode = 1;
      holding = false;

    } else if (ktomeCAN.can_msg[0] == 'I') { // Setup a game scenario
      Serial.println("This module was asked to setup a scenario!");
      gamemode = 2;
      gameSetup();
      holding = false;

    } else if (ktomeCAN.can_msg[0] == 'C' && gamemode == 2) { // Game manual setup call
      Serial.println("This module was asked about it's manual setup!");
      keypad_place();
      holding = false;

    } else if (ktomeCAN.can_msg[0] == 'A' && gamemode == 2) { // Game manual setup call
      Serial.println("This module was asked to start the game!");
      //
      gamemode = 3;
      holding = false;
    } else if (ktomeCAN.can_msg[0] == 'Z') { // Game manual setup call
      Serial.println("This module was asked to stop the game!");
      gamemode = 4;
      holding = false;
    }
  }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
