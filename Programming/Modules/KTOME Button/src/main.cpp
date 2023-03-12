//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 01/06/22
//======================================================================
//
//  Module: The Button module (Slave, Standard, Vanilla)
//  version 0.7.0
//
//  Version goals: Support for FastLED library, working on PCB
//
//======================================================================

#define DEBUG 1 // Comment this line to disable Serial messages

#ifdef DEBUG
    #define DEBUG_SERIAL(x)     Serial.begin(x)
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTBIN(x)   Serial.print(x, BIN)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTLNBIN(x) Serial.println(x, BIN)
    #define DEBUG_PADZERO(x)    ktomeCAN.padZeros(x)
#else
    #define DEBUG_SERIAL(x)
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTBIN(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTLNBIN(x)
    #define DEBUG_PADZERO(x)
#endif

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_LEDBackpack.h>
//#include <Entropy.h>
//#include <LiquidCrystal.h>
#include <KTOME_common.h>
#include <KTOME_Button.h>
#include <config.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

// #define PIN_LED_CAN   GPIO_NUM_12
// #define MOD_STATUS_R  GPIO_NUM_32
// #define MOD_STATUS_G  GPIO_NUM_33
#define PIN_LED_STATUS   GPIO_NUM_2

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool game_running = false;
bool module_defused = false;
long strike_light_timer;
long strike_light_flash = 1000;
bool strike_light_state;
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
byte battery_number;
//bool parallel_port;
bool ind_frk;
bool ind_car;

// Module specific
KTOME_Button button;
String timer_digits;

// CAN
int CAN_ID;

// Led* leds = new Led[11];
// LedBlinkable leds[3]; // Move back to pointer array?!
// byte led_pin_array[] = {PIN_LED_CAN, MOD_STATUS_R, MOD_STATUS_G};
byte led_length = 10;
CRGB leds[2];
FLedPWM fled;

// Fuction list
void powerOn();
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
    DEBUG_SERIAL(115200);
    while (!Serial);
    DEBUG_PRINTLN("== KTOME: Module (Button) ==");

    // Start CAN bus
    CAN_ID = CONFIG_CAN_MODULE_TYPE | CONFIG_CAN_MODULE_NUM;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start(14, 13);
    // start the CAN bus at 500 kbps
    // if (!ktomeCAN.start()) {
    //     DEBUG_PRINTLN("Starting CAN failed!");
    //     while (1);
    // }
    DEBUG_PRINT("My ID is:   0b");
    DEBUG_PADZERO(CAN_ID);
    DEBUG_PRINTLNBIN(CAN_ID);

    // Randomiser
    esp_random();

    DEBUG_PRINTLN("Init'ing fled and FastLED...");
    fled.init(&leds[0], led_length);
    FastLED.addLeds<WS2812B, PIN_LED_STATUS, GRB>(leds, led_length).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( 16 );

    // Setup objects
    DEBUG_PRINTLN("Starting button...");
    button.start(&fled);

//   for (byte ii = 0; ii < 3; ii++) {
//     leds[ii].init(led_pin_array[ii]);
//   }
    // DEBUG_PRINTLN("Init'ing fled and FastLED...");
    // fled.init(&leds[0], led_length);
    // FastLED.addLeds<WS2812B, PIN_LED_STATUS, GRB>(leds, led_length).setCorrection( TypicalLEDStrip );
    // FastLED.setBrightness( 16 );
}

void loop() {
    switch (gamemode) {
        case 0:
            powerOn();
            carPark();
            break;
        case 1: // Module poll
            DEBUG_PRINTLN(F("Module search..."));
            gameReset();
            initialisation();
            carPark();
            break;
        case 2: // Game in set-up
            DEBUG_PRINTLN(F("Game set-up..."));
            carPark();
            break;
        case 3: // Game running
            DEBUG_PRINTLN(F("Game starting!"));
            game_running = true;
            gameRunning();
            break;
        case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone
            game_ready = false;
            if (explosion_fx) {
                explodeFX(true);
            } else if (game_aborted) {
                explodeFX(false);
            }
            carPark();
            break;
    }
}

void carPark() {
  holding = true;
  DEBUG_PRINTLN("Script parked - waiting for direction...");
  while (holding) {
    do {
      CANInbox();
    } while (ktomeCAN.isMessageWaiting());

    // Start test
    if (button.inputCheck()){
        fled.fade(1, CRGB::Blue, 0x000055, 500, 166, 333, 333);
        Serial.print("Button press detected!");
    }
    // End test

    button.update();
    // leds[1].update();
    fled.update();
    FastLED.show();
    delay (1);
  }
}

//**********************************************************************
// FUNCTIONS: Game powered : gamemode = 0
//**********************************************************************

// Module has been plugged in and finished setup()

void powerOn()
{
    fled.blink(0, CRGB::Green, CRGB::Black, 500, 2);
    button.powerOn();
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : gamemode = 1
//**********************************************************************

// Comm with modules to determine what's connected

void initialisation() {
    char CAN_message[2];
    CAN_message[0] = 'p';
    CAN_message[1] = '\0';
//   leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
//   leds[0].write(false);
}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

void gameSetup() {
    game_ready = false;
    button.reset();
    button.generate();

    char CAN_message[2] = "i";
    // leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
    // leds[0].write(false);
    game_ready = true;
}

void manualCheck() {
    String temp_msg;
    temp_msg = button.getManual(); // NO MANUAL CHECK NEEDED!

    char CAN_message[6];
    temp_msg.toCharArray(CAN_message,6);
    
    // leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
    // leds[0].write(false);
    // leds[1].blink(true, 1000);
    fled.blink(0, CRGB::Yellow, CRGB::Black, 500);

}

//**********************************************************************
// FUNCTIONS: Game Managers and functions : gamemode = 3 (mostly!)
//**********************************************************************

void gameReset() {
    module_defused = false;
    game_aborted = false;
    button.reset(); // Is this needed if reset is done before game gen?
    // manual_blink = false;
    strike_light_state = false;
    // for (byte ii = 0; ii < 3; ii++) {
    //     leds[ii].write(false);
    // }
    fled.write(0, CRGB::Black);
}

void gameRunning() {

    while (gamemode == 3) {
        thismillis = millis();
        // if (!module_defused) {
        /* Input check */
        inputCheck();
        /* Defuse check */
        module_defused = button.isDefused();
        /* Output update */
        button.update();
        // leds[1].update();
        fled.update();
        FastLED.show();
        // }

        // Check incoming messages
        CANInbox();
    }
}

void strikeTrigger() {
    char CAN_message[] = "x";
    // leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    // leds[0].write(false);

    strike_light_timer = thismillis + strike_light_flash;
    // leds[1].blink(true, 500, 1);
    fled.blink(0, CRGB::Red, CRGB::Black, 500, 2);
    }

    void defuseTrigger() {
    char CAN_message[] = "d";
    // leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    // leds[0].write(false);
    // leds[1].write(false);
    // leds[2].write(true);
    fled.write(0, CRGB::Green);

    module_defused = true;
}

void explodeFX(bool target_state) {
    if (target_state) { // Only turn off, never flash this when exploding
        // leds[2].write(false);
        // leds[1].blink(true, 100, 1);
        fled.blink(0, CRGB::Red, CRGB::Black, 100, 2);
        button.explode();
    } else {
        // leds[1].write(false);
        // leds[2].write(false);
        fled.write(0, CRGB::Black);
    }

}

void inputCheck() {
    byte return_byte;
    return_byte = button.inputCheck();
    if (return_byte == 1) { // Timer to be requested
        // DEBUG_PRINT("Send time: ");
        // DEBUG_PRINTLN(millis());
        char CAN_message[] = "t";
        // leds[0].write(true);
        ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
        // leds[0].write(false);
    } else if (return_byte == 2) { // Timer not needed, success
        defuseTrigger();
    } else if (return_byte == 3) { // Strike
        strikeTrigger();
    } else { // Button not released

    }
}

//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox() {
    if (ktomeCAN.isMessageWaiting()) { // Outstanding messages to handle
        ktomeCAN.receive();
        if (ktomeCAN.can_msg[0] == 'P') { // Init call
            DEBUG_PRINTLN("This module is present!");
            gamemode = 1;
            holding = false;

        } else if (ktomeCAN.can_msg[0] == 'I') { // Setup a game scenario
            DEBUG_PRINTLN("This module was asked to setup a scenario!");
            gamemode = 2;
            gameSetup();
            holding = false;

        } else if (ktomeCAN.can_msg[0] == 'C' && gamemode == 2) { // Game manual setup call
            DEBUG_PRINTLN("This module was asked about it's manual setup!");
            manualCheck();
            //      holding = false;

        } else if (ktomeCAN.can_msg[0] == 'M' && gamemode == 2) { // Game manual check call
            DEBUG_PRINTLN("This module was asked if manual setup was successful!");
            //   leds[1].write(false);
            fled.write(0, CRGB::Black);

        } else if (ktomeCAN.can_msg[0] == 'A' && gamemode == 2) { // Game start
            DEBUG_PRINTLN("This module was asked to start the game!");
            //
            gamemode = 3;
            holding = false;
        } else if (ktomeCAN.can_msg[0] == 'Z') { // Game stop
            DEBUG_PRINTLN("This module was asked to stop the game!");
            gamemode = 4;
            if (ktomeCAN.can_msg[1] == '1') { // Explosion
                explosion_fx = true;
                strike_light_timer = thismillis + 100;
            } else if (ktomeCAN.can_msg[1] == '0') { // Defused
                explosion_fx = false;
            } else { // Aborted
                explosion_fx = false;
                game_aborted = true;
            }
        holding = false;
        } else if (ktomeCAN.can_msg[0] == 'W') { // Edgework setup
            battery_number = ktomeCAN.can_msg[3] - '0';
            if (ktomeCAN.can_msg[4] == '1'){
                ind_car = true;
            } else { ind_car = false;}
            if (ktomeCAN.can_msg[5] == '1'){
                ind_frk = true;
            } else { ind_frk = false;}
            button.findSolution(ind_frk, ind_car, battery_number);
        } else if (ktomeCAN.can_msg[0] == 'S') { // Serial number

        } else if (ktomeCAN.can_msg[0] == 'X') { // Strike count

        } else if (ktomeCAN.can_msg[0] == 'H') { // Heartbeat on second tick

        } else if (ktomeCAN.can_msg[0] == 'T') { // Time on display
            // DEBUG_PRINT("Return time: ");
            // DEBUG_PRINTLN(millis());
            // DEBUG_PRINT("serial + millis delay: ");
            // DEBUG_PRINTLN(millis());
            timer_digits = "";
            for (byte ii = 1; ii < 5; ii++){
                timer_digits = timer_digits + ktomeCAN.can_msg[ii];
            }
            byte return_byte = button.timerDigits(timer_digits);
            if (return_byte == 1) {
                defuseTrigger();
            } else if (return_byte == 0) {
                strikeTrigger();
            }
        }
    }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
