//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 30/10/21
//======================================================================
//
//  Module: The Venting Gas module (Slave, Needy, Vanilla)
//  version 0.7.1
//
//======================================================================

// #define DEBUG_ON // Comment this to disable Serial monitor

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
// #include <Adafruit_GFX.h>
// #include <u8g2lib.h>
#include <KTOME_common.h>
#include <KTOME_Vent.h>
#include <config.h>

#ifdef DEBUG_ON
#define DebugSerial(...) Serial.printf(__VA_ARGS__)
#else 
#define DebugSerial(...)
#endif

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

// #define PIN_LED_CAN   GPIO_NUM_12
// #define MOD_STATUS_R  GPIO_NUM_32
// #define MOD_STATUS_G  GPIO_NUM_33

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool game_running = false;
bool module_defused = false;
// bool manual_blink = false;
bool explosion_fx = false;
bool game_aborted = false;

// Timer
long thismillis;
//byte strikenumber = 0;

// Widgets
//char serial_number[7]; // Will this variable ever be needed by a module, when vowel, odd and even variables would do?
//bool serial_vowel;
//bool serial_odd;
//bool serial_even;
//byte battery_number;
//bool parallel_port;
//bool ind_frk;
//bool ind_car;

// Vent specific
KTOME_Vent vent;

// CAN
int CAN_ID;

// Led* leds = new Led[11];
// LedBlinkable leds[3]; // Move back to pointer array?!
// byte led_pin_array[] = {PIN_LED_CAN, MOD_STATUS_R, MOD_STATUS_G};

// Fuction list
void carPark();
void initialisation();
void gameSetup();
void gameReset();
void gameRunning();
void strikeTrigger();
void defuseTrigger();
void explodeFX(bool target_state);
void inputCheck();
void CANInbox();

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {
  // Start serial connection
  #ifdef DEBUG_ON
  Serial.begin(115200);
  while (!Serial);
  #endif
  DebugSerial("\n== KTOME: Module (Venting Gas) ==\n");

  // Start CAN bus
  CAN_ID = CONFIG_CAN_MODULE_TYPE | CONFIG_CAN_MODULE_NUM;
  ktomeCAN.setId(CAN_ID);
  ktomeCAN.start();
  // start the CAN bus at 500 kbps
  if (!ktomeCAN.start()) {
    DebugSerial("Starting CAN failed!\n");
    while (1);
  }
//   DebugSerial("My ID is:   0b");
//   ktomeCAN.padZeros(CAN_ID);
//   DebugSerial(CAN_ID, BIN);
    DebugSerial("My ID is: %08X\n", CAN_ID);

  // Randomiser
  esp_random();

  // Setup objects
  vent.start();

  // for (byte ii = 0; ii < 3; ii++) {
  //   leds[ii].init(led_pin_array[ii]);
  // }
}

void loop() {
    switch (gamemode) {
        case 0:
            carPark();
            break;
        case 1: // Module poll
            DebugSerial("Module search...\n");
            gameReset();
            initialisation();
            carPark();
            break;
        case 2: // Game in set-up
            DebugSerial("Game set-up...\n");
            carPark();
            break;
        case 3: // Game running
            DebugSerial("Game starting!\n");
            game_running = true;
            gameRunning();
            // carPark();
            break;
        case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone
            game_ready = false;
            if (explosion_fx) {
                explodeFX(true);
            } else if (game_aborted) {
                explodeFX(false);
            }
            vent.reset();
            carPark();
            break;
    }
}

void carPark() {
  holding = true;
  DebugSerial("Script parked - waiting for direction...\n");
  while (holding) {
    do {
      CANInbox();
    } while (ktomeCAN.isMessageWaiting());
    // vent.update();
    // leds[1].update();
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
  // leds[0].write(true);
  ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
  // leds[0].write(false);
}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

// void gameSetup() {
//   game_ready = false;
//   vent.reset();
//   vent.generate();

//   char CAN_message[2] = "i";
//   // leds[0].write(true);
//   ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
//   // leds[0].write(false);
//   game_ready = true;
// }

// void manualCheck() {
//   String temp_msg;
//   temp_msg = vent.getManual();

//   char CAN_message[6];
//   temp_msg.toCharArray(CAN_message,6);
  
//   leds[0].write(true);
//   ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
//   leds[0].write(false);
//   leds[1].blink(true, 1000);
// }

//**********************************************************************
// FUNCTIONS: Game Managers and functions : gamemode = 3 (mostly!)
//**********************************************************************

void gameReset() {
  module_defused = false;
  vent.reset(); // Is this needed if reset is done before game gen?
  // manual_blink = false;
  // strike_light_state = false;
  // for (byte ii = 0; ii < 3; ii++) {
  //   leds[ii].write(false);
  // }
}

void gameRunning() {

    // vent.gameStart();
    while (gamemode == 3) {
        thismillis = millis();

        if (vent.isAwake()) {
            /* Input check */
            if (vent.isActive()) {
              inputCheck();
            }
            /* Defuse check */

            /* Output update */
            if (vent.update() == 1) {
              strikeTrigger();
            }
          
            byte return_byte = vent.sendSound();
            if (return_byte == 1) {
              char CAN_message[] = "u6";
              DebugSerial("Trigger sound: Activate\n");
              ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, 3);
            } else if (return_byte == 2) {
              char CAN_message[] = "u5";
              DebugSerial("Trigger sound: Warning\n");
              ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, 3);
            }
            // leds[1].update();
        }
        // Check incoming messages
        CANInbox();
    }
}

void strikeTrigger() {
  char CAN_message[] = "x";
  // leds[0].write(true);
  ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
  // leds[0].write(false);

  // strike_light_timer = thismillis + strike_light_flash;
  // leds[1].blink(true, 500, 1);
}

void defuseTrigger() {
  char CAN_message[] = "d";
  // leds[0].write(true);
  ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
  // leds[0].write(false);
  // leds[1].write(false);
  // leds[2].write(true);

  module_defused = true;
  vent.defused();
}

void explodeFX(bool target_state) {
  if (target_state) { // Only turn off, never flash this when exploding
    // leds[2].write(false);
    // leds[1].blink(true, 100, 1);
    vent.explode();
  } else {
    // leds[1].write(false);
    // leds[2].write(false);
    vent.reset();
  }
}

void inputCheck() {
  if (vent.isActive()) {
    // byte return_byte;
    vent.inputCheck();
    // if (return_byte == 2) { // Right
    //   vent.sleep();
    // } else if (return_byte == 1) { // Wrong and strike
    //   strikeTrigger();
    // } else { // Wrong, no strike

    // }
  }
}

//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox() {
    if (ktomeCAN.isMessageWaiting()) { // Outstanding messages to handle
        ktomeCAN.receive();
                        // u8g2.firstPage();  
                        // do {
                        //         u8g2.setCursor(5,25);
                        //         u8g2.print(ktomeCAN.can_msg);
                        // } while( u8g2.nextPage() );
        if (ktomeCAN.can_msg[0] == 'P') { // Init call
            DebugSerial("This module is present!\n");
            gamemode = 1;
            holding = false;

        } else if (ktomeCAN.can_msg[0] == 'I') { // Setup a game scenario

        } else if (ktomeCAN.can_msg[0] == 'C' && gamemode == 2) { // Game manual setup call

        } else if (ktomeCAN.can_msg[0] == 'M' && gamemode == 2) { // Game manual check call

        } else if (ktomeCAN.can_msg[0] == 'A' /*&& gamemode == 2*/) { // Game start
            DebugSerial("This module was asked to start the game!\n");
            //
            gamemode = 3;
            holding = false;
        } else if (ktomeCAN.can_msg[0] == 'Z') { // Game stop
            DebugSerial("This module was asked to stop the game!\n");
            gamemode = 4;
            if (ktomeCAN.can_msg[1] == '1') { // Explosion
                explosion_fx = true;
                // strike_light_timer = thismillis + 100;
            } else if (ktomeCAN.can_msg[1] == '0') { // Defused
                explosion_fx = false;
            } else { // Aborted
                explosion_fx = false;
                game_aborted = true;
            }
            holding = false;
        } else if (ktomeCAN.can_msg[0] == 'W') { // Edgework setup

        } else if (ktomeCAN.can_msg[0] == 'S') { // Serial number

        } else if (ktomeCAN.can_msg[0] == 'X' && gamemode == 3) { // Strike count
            DebugSerial("Received X\n");
            if (!vent.isAwake()) {
                if (random(10) > 0){
                    vent.activate();
                }
            }
        } else if (ktomeCAN.can_msg[0] == 'H') { // Heartbeat on second tick

        } else if (ktomeCAN.can_msg[0] == 'T') { // Time on display

        } else if (ktomeCAN.can_msg[0] == 'N') { // Needy activation time
          if (!vent.isAwake()) {
            vent.activate();
          }
        }
        else if (ktomeCAN.can_msg[0] == 'D' && gamemode == 3) { // Other module defused
            DebugSerial("Received D\n");
            if (!vent.isAwake()) {
                if (random(10) > 0){
                vent.activate();
                }
            }
        }
    }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
