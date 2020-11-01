//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 4/7/20
//======================================================================
//
//  Module: The Keypad module (SLAVE)
//
//  version 0.1.0
//
//  Goal for this version: Get this thing going!
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEServer.h>
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

// Game
byte gamemode = 0;
bool game_ready = false;

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

#define MOD_STATUS_R   GPIO_NUM_32
#define MOD_STATUS_G   GPIO_NUM_33

// CAN                      [ Module type ][ID][ unused ]
#define CAN_ID            0b00010000000000001000000000000 // ID for "Keypad" + 0100
#define CAN_MASK          0b10010000000000001000000000000 // Filter for the "Keypad"

//#define CAN_ID            0b00010000000000010000000000000 // ID for "Keypad" + 1000
//#define CAN_MASK          0b10010000000000010000000000000 // Filter for the "Keypad"

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

char buffer_msg[9][16];
int buffer_can_id[16];
byte buffer_pointer_r;
byte buffer_pointer_w;
char msg_received[9];
int msg_rec_id;

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Module (Generic) ==");

  // Start CAN bus
  CAN.setPins(PIN_CAN_RX, PIN_CAN_TX);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  //  CAN.filterExtended(CAN_ID, CAN_MASK);
  CAN.onReceive(onReceive);

  Serial.print("My ID is:   0b");
  padZerosCAN(CAN_ID);
  Serial.println(CAN_ID, BIN);
  Serial.print("My mask is: 0b");
  padZerosCAN(CAN_MASK);
  Serial.println(CAN_MASK, BIN);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Randomiser
  //  Entropy.initialize();
  //  randomSeed(Entropy.random());
  esp_random();

}

void loop() {

  switch (gamemode) {
    case 0: // First time set-up
      initialisation();
      break;
    case 1: // Game in set-up
      Serial.println(F("Game set-up..."));
      gameSetup();
      break;
    case 2: // Game running
      Serial.println(F("Game starting!"));
      gameRunning();
      break;
    case 3: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone

      break;
  }
}

//**********************************************************************
// FUNCTIONS: Game Initialisation
//**********************************************************************

// Comm with modules to determine what's connected

void initialisation() {

  Serial.println("Listening out...");

  do {
    //    if (buffer_pointer_r != buffer_pointer_w) {
    CANReceive();
//    delay(500);
    //    }
  } while (gamemode == 0);

  char CAN_message[2];
  CAN_message[0] = 'p';
  CAN_message[1] = '\0';
  CANSend(CAN_TO_MASTER | CAN_ID, CAN_message, 1);

  //  Serial.println("Game mode changed to 1");

}

//**********************************************************************
// FUNCTIONS: Game Setup
//**********************************************************************

// Comm with phone to set up a game

void gameSetup() {

  bool game_ready = false;
  int packet_size;

  do {

//    if (buffer_pointer_r != buffer_pointer_w) {
      CANReceive();
//    }

    // Receive request to modules to generate games

    // Generate game

    // Send info back to Master


  } while (!game_ready);

}

void keypad_place() {
  char CAN_message[6];
  CAN_message[0] = 'c';
  CAN_message[1] = symbol_index[keypad_symbols[0]]+'0';
  CAN_message[2] = symbol_index[keypad_symbols[1]]+'0';
  CAN_message[3] = symbol_index[keypad_symbols[2]]+'0';
  CAN_message[4] = symbol_index[keypad_symbols[3]]+'0';
  CAN_message[5] = '\0';
  CANSend(CAN_TO_MASTER | CAN_ID, CAN_message, 5);
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
      symbol_index[0] = 1;
      symbol_index[1] = 2;
      symbol_index[2] = 3;
      symbol_index[3] = 4;
      symbol_index[4] = 5;
      symbol_index[5] = 6;
      symbol_index[6] = 7;
      break;
    case 1:
      symbol_index[0] = 8;
      symbol_index[1] = 1;
      symbol_index[2] = 7;
      symbol_index[3] = 9;
      symbol_index[4] = 10;
      symbol_index[5] = 6;
      symbol_index[6] = 11;
      break;
    case 2:
      symbol_index[0] = 12;
      symbol_index[1] = 13;
      symbol_index[2] = 9;
      symbol_index[3] = 14;
      symbol_index[4] = 15;
      symbol_index[5] = 3;
      symbol_index[6] = 11;
      break;
    case 3:
      symbol_index[0] = 16;
      symbol_index[1] = 17;
      symbol_index[2] = 18;
      symbol_index[3] = 5;
      symbol_index[4] = 14;
      symbol_index[5] = 11;
      symbol_index[6] = 19;
      break;
    case 4:
      symbol_index[0] = 20;
      symbol_index[1] = 19;
      symbol_index[2] = 18;
      symbol_index[3] = 21;
      symbol_index[4] = 17;
      symbol_index[5] = 22;
      symbol_index[6] = 23;
      break;
    case 5:
      symbol_index[0] = 16;
      symbol_index[1] = 8;
      symbol_index[2] = 24;
      symbol_index[3] = 25;
      symbol_index[4] = 20;
      symbol_index[5] = 26;
      symbol_index[6] = 27;
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
  CANSend(CAN_TO_MASTER | CAN_ID, CAN_message, 1);

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

  bool game_running = true;

  thismillis = millis();

  while (game_running) {

    // Input check
    // Output update

    // Send CAN message

  }



}

//**********************************************************************
// FUNCTIONS: Game functions
//**********************************************************************






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

//  Serial.print("Read pointer: ");
//  Serial.println(buffer_pointer_r);
//  Serial.print("Write pointer: ");
//  Serial.println(buffer_pointer_w);
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

    if (msg_received[0] == 'P' && gamemode == 0) { // Init call
      Serial.println("This module has been called!");
      //      digitalWrite(PIN_LED, HIGH);
      gamemode = 1;

    } else if (msg_received[0] == 'I' && gamemode == 1) { // Setup a game scenario
      Serial.println("This module was asked to setup a scenario!");
      keypad_setup();

    } else if (msg_received[0] == 'C' && gamemode == 1) { // Game manual setup call
      Serial.println("This module was asked about it's manual setup!");
      keypad_place();
    }
  }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************

void padZerosCAN(int id) {
  if (id < (1 << 28)) {
    Serial.print("0");
  }
  if (id < (1 << 27)) {
    Serial.print("0");
  }
  if (id < (1 << 26)) {
    Serial.print("0");
  }
  if (id < (1 << 25)) {
    Serial.print("0");
  }
  if (id < (1 << 24)) {
    Serial.print("0");
  }
  if (id < (1 << 23)) {
    Serial.print("0");
  }
  if (id < (1 << 22)) {
    Serial.print("0");
  }
  if (id < (1 << 21)) {
    Serial.print("0");
  }
  if (id < (1 << 20)) {
    Serial.print("0");
  }
  if (id < (1 << 19)) {
    Serial.print("0");
  }
  if (id < (1 << 18)) {
    Serial.print("0");
  }
  if (id < (1 << 17)) {
    Serial.print("0");
  }
  if (id < (1 << 16)) {
    Serial.print("0");
  }
  if (id < (1 << 15)) {
    Serial.print("0");
  }
  if (id < (1 << 14)) {
    Serial.print("0");
  }
  if (id < (1 << 13)) {
    Serial.print("0");
  }
  if (id < (1 << 12)) {
    Serial.print("0");
  }
  if (id < (1 << 11)) {
    Serial.print("0");
  }
  if (id < (1 << 10)) {
    Serial.print("0");
  }
  if (id < (1 << 9)) {
    Serial.print("0");
  }
  if (id < (1 << 8)) {
    Serial.print("0");
  }
  if (id < (1 << 7)) {
    Serial.print("0");
  }
  if (id < (1 << 6)) {
    Serial.print("0");
  }
  if (id < (1 << 5)) {
    Serial.print("0");
  }
  if (id < (1 << 4)) {
    Serial.print("0");
  }
  if (id < (1 << 3)) {
    Serial.print("0");
  }
  if (id < (1 << 2)) {
    Serial.print("0");
  }
  if (id < (1 << 1)) {
    Serial.print("0");
  }
  //  if (id < (1)) {
  //    Serial.print("0");
  //  }
}
