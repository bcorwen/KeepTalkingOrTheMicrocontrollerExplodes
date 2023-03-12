//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 05/02/23
//======================================================================
//
//  Module: Generic Needy module (Slave, Needy, Vanilla)
//  version 0.8.0
//
//  Version goals: Genericise the needy module main file
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
#include <defines.h>
// #include <Wire.h>
// #include <CAN.h>
// #include <KTOME_CAN.h>
// #include <config.h>
// #include <KTOME_common.h>

// #ifdef DEBUG
//     #define DEBUG_SERIAL(x)     Serial.begin(x)
//     #define DEBUG_PRINT(x)      Serial.print(x)
//     #define DEBUG_PRINTBIN(x)   Serial.print(x, BIN)
//     #define DEBUG_PRINTLN(x)    Serial.println(x)
//     #define DEBUG_PRINTLNBIN(x) Serial.println(x, BIN)
//     #define DEBUG_PADZERO(x)    ktomeCAN.padZeros(x)
// #else
//     #define DEBUG_SERIAL(x)
//     #define DEBUG_PRINT(x)
//     #define DEBUG_PRINTBIN(x)
//     #define DEBUG_PRINTLN(x)
//     #define DEBUG_PRINTLNBIN(x)
//     #define DEBUG_PADZERO(x)
// #endif

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

#define PIN_LED_STATUS  GPIO_NUM_2

// Game
byte game_mode = 0;
bool holding = false;
bool game_running = false;
bool module_solved = false;
long strike_light_flash = 1000;
long explode_flash = 100;
bool explosion_fx = false;
bool interacted_with = false;
bool game_aborted = false;
byte game_over = 0; // 0 = null, 1 = exploded, 2 = solved, 3 = aborted game

// Timing
long thismillis;

// Widgets
byte strike_count;
// char serial_number[7]; // Will this variable ever be needed by a module, when vowel, odd and even variables would do?
bool serial_vowel;
bool serial_odd;
bool serial_even;
byte battery_number;
bool serial_port;
bool parallel_port;
bool ind_frk;
bool ind_car;

// CAN
int CAN_ID;

// FastLEDs
// CRGB leds[FLED_LENGTH];
// FLedPWM fled;

// Fuction list
void powerOn();
void carPark();
void registration();
void isr();
void gameSetup();
void gameReset();
void gameRunning();
void strikeTrigger();
void solveTrigger();
void washUp();
void inputCheck();
void updateCheck();
void CANInbox();

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup()
{
    // Start serial connection
    DEBUG_SERIAL(115200);
    DEBUG_PRINTLN(module_name);

    // Start CAN bus
    CAN_ID = CONFIG_CAN_MODULE_TYPE | CONFIG_CAN_MODULE_NUM;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start(27, 26); // This is for dev board - leave blank for PCB
    DEBUG_PRINT("My ID is: 0b");
    DEBUG_PADZERO(CAN_ID);
    DEBUG_PRINTLNBIN(CAN_ID);

    // Randomiser
    esp_random();

    // Setup FastLED
    // fled.init(&leds[0], FLED_LENGTH);
    // FastLED.addLeds<WS2812B, PIN_LED_STATUS, GRB>(leds, FLED_LENGTH).setCorrection( TypicalLEDStrip );
    // FastLED.setBrightness( 16 );

    // Setup module
    module.start();
    if (module.needsIsr()) {
        attachInterrupt(digitalPinToInterrupt(module.intPin), isr, RISING);
    }
}

void loop()
{
    switch (game_mode)
    {
        case 0:
            powerOn();
            carPark();
            break;
        case 1: // Module poll
            DEBUG_PRINTLN("Mode 1: Module registration");
            gameReset();
            registration();
            carPark();
            break;
        case 2: // Game in set-up
            DEBUG_PRINTLN("Mode 2: Game set-up");
            carPark();
            break;
        case 3: // Game running
            DEBUG_PRINTLN("Mode 3: Game starting!");
            gameRunning();
            break;
        case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be triggered from phone
            DEBUG_PRINTLN("Mode 4: Game wash-up");
            washUp();
            carPark();
            break;
    }
}

void carPark()
{
    holding = true;
    DEBUG_PRINTLN("Script parked - waiting for direction...");
    while (holding)
    {
        do
        {
            CANInbox();
        } while (ktomeCAN.isMessageWaiting());

        module.update(); // Update the lights and displays of this module
        if (module.isOutbox()) { // Check if the module wants to send a message (e.g. Simon lights are flashing and wants to trigger a sound cue)
            String temp_str = module.outbox();
            char CAN_message[temp_str.length() + 1];
            temp_str.toCharArray(CAN_message, temp_str.length() + 1);
            ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, temp_str.length());
        }

        // fled.update();
        // FastLED.show();
        // delay(1);
    }
}

//**********************************************************************
// FUNCTIONS: Game powered : game_mode = 0
//**********************************************************************

// Module has been plugged in and finished setup()

void powerOn()
{
    module.powerOn();
    // fled.write(0, CRGB::Green); // FastLD testing only...
    // fled.blink(0, CRGB::Red, CRGB::Green, 1000, 1000, 2);
    
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : game_mode = 1
//**********************************************************************

// Comm with modules to determine what's connected

void registration()
{
    char CAN_message[2];
    CAN_message[0] = 'p';
    CAN_message[1] = '\0';
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
}

void isr() {
    module.isrHandler();
}

//**********************************************************************
// FUNCTIONS: Game Setup : game_mode = 2
//**********************************************************************

// Comm with phone to set up a game

void gameReset()
{
    DEBUG_PRINTLN("Resetting... ");
    module_solved = false;
    interacted_with = false;
    game_running = false;
    // module.reset(); // Is this necessary? There is an initial reset when module is called, then a reset on receiving 'I' message. Polling shouldn't need a reset as setup will surely handle this...

    // fled.write(0, CRGB::Black);
    DEBUG_PRINTLN("Reset complete!");
}

void gameSetup()
{
    DEBUG_PRINTLN("Generating game...");
    gameReset();
    module.reset();
    module.generate(); // Not needed for needy

    char CAN_message[2] = "i";
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
    DEBUG_PRINTLN("Game generated!");
}

void manualCheck()
{
    if (module.isManual()) {
        String temp_str = module.getManual();
        char CAN_message[temp_str.length() + 1];
        temp_str.toCharArray(CAN_message, temp_str.length() + 1);
        ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, temp_str.length());
        // fled.blink(0, CRGB::Yellow, CRGB::Black, 500);
        DEBUG_PRINTLN("Manual setup required...");
    } else {
        DEBUG_PRINTLN("Manual setup not required!");
    }
}

void manualConfirm()
{
    byte return_byte = module.manualConfirm();
    if (return_byte == 1) { // Manual confirmation needed and it returned successfully
        char CAN_message[3] = "m1";
        ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 2);
        // fled.write(0, CRGB::Black);
    } else if (return_byte == 2) { // Manual confirmation needed and it was rejected
        char CAN_message[3] = "m0";
        ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 2);
    } else { // Manual confirmation not required
        // fled.write(0, CRGB::Black);
    }
}

//**********************************************************************
// FUNCTIONS: Game Managers and functions : game_mode = 3
//**********************************************************************

void gameRunning()
{
    game_running = true;
    module.gameStart();
    // byte return_byte; // 0 = No significant output, 
    
    while (game_mode == 3)
    {
        thismillis = millis();
        // if (!module_solved) // NEED TO PUT SOLVE CHECK IN INPUT CHECK FOR EACH MODULE, IF IT CAN BE INTERACTED WITH AFTER DEFUSAL

        /* Input check */
        inputCheck();

        /* Output update */
        updateCheck();
        /* Status light update */
        // fled.update(); // Updates colours to display
        // FastLED.show(); // Updated the LEDs to the new colours
        
        /* CAN message out check */
        if (module.isOutbox()) { // Use this to check if CAN messages should be sent (e.g. Simon sound cues)
            String temp_str = module.outbox();
            char CAN_message[temp_str.length() + 1];
            temp_str.toCharArray(CAN_message, temp_str.length() + 1);
            ktomeCAN.send(can_ids.Master | can_ids.Widgets | CAN_ID, CAN_message, temp_str.length());
        }
        
        /* Check incoming messages */
        CANInbox();
    }
}

void inputCheck()
{
    byte return_byte;
    return_byte = module.inputCheck(); // 0 = no action, 1 = strike, 2 = solve
    if (return_byte == 1) {
        DEBUG_PRINTLN("Input caused strike!");
        strikeTrigger();
    } else if (return_byte == 2) {
        DEBUG_PRINTLN("Input caused solve!");
        if (!module_solved && module.isSolved()) {
            solveTrigger();
        }
    }
}

void updateCheck()
{
    byte return_byte = module.update(); // 0 = no action, 1 = strike, 2 = solve
    if (return_byte == 1) {
        DEBUG_PRINTLN("Update caused strike!");
        strikeTrigger();
    } else if (return_byte == 2) {
        DEBUG_PRINTLN("Update caused sleep!");
        // if (!module_solved && module.isSolved()) {
        //     solveTrigger();
        // }
    }
}

void strikeTrigger()
{
    char CAN_message[] = "x";
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);

    if (module_solved) {
        // fled.blink(0, CRGB::Red, CRGB::Green, 500, 2);
    } else {
        // fled.blink(0, CRGB::Red, CRGB::Black, 500, 2);
    }
    DEBUG_PRINTLN("Strike!");
}

void solveTrigger()
{
    char CAN_message[] = "d";
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    // fled.write(0, CRGB::Green);

    module_solved = true;
}

//**********************************************************************
// FUNCTIONS: Game wrap up : game_mode = 4
//**********************************************************************

void washUp()
{
    if (game_over == 1) { // exploded
        // fled.blink(0, CRGB::Red, CRGB::Black, 100, 2);
        module.explode();
    } else if (game_over == 2) { // solved
        module.defuse();
    } else if (game_over == 3) { // aborted
        // fled.write(0, CRGB::Black);
        gameReset();
        module.reset();
    }
}

//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox()
{
    if (ktomeCAN.isMessageWaiting())
    { // Outstanding messages to handle
        ktomeCAN.receive();
        if (ktomeCAN.can_msg[0] == 'P')
        { // Init call
            DEBUG_PRINTLN("This module is present!");
            game_mode = 1;
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'I')
        { // Setup a game scenario
            DEBUG_PRINTLN("This module was asked to setup a scenario!");
            game_mode = 2;
            gameSetup();
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'C' &&
                 game_mode == 2)
        { // Game manual setup call
            DEBUG_PRINTLN("This module was asked about it's manual setup!");
            if (module.isManual()) {
                manualCheck();
            } else {
                DEBUG_PRINTLN("ERROR: This module should not have been asked for a "
                           "manual setup!");
            }
            // manualCheck();
            //      holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'M' &&
                 game_mode == 2)
        { // Game manual check call
            DEBUG_PRINTLN("This module was asked if manual setup was successful!");
            if (module.isManual()) {
                manualConfirm();
            } else {
                // fled.write(0, CRGB::Black);
            }
        }
        else if (ktomeCAN.can_msg[0] == 'A' && game_mode == 2)
        { // Game start
            // DEBUG_PRINTLN("This module was asked to start the game!");
            //
            game_mode = 3;
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'Z')
        { // Game stop
            DEBUG_PRINTLN("This module was asked to stop the game!");
            game_mode = 4;
            game_running = false;
            DEBUG_PRINTLN(ktomeCAN.can_msg[1]);
            if (ktomeCAN.can_msg[1] == '1') { // Explosion
                game_over = 1;
            } else if (ktomeCAN.can_msg[1] == '0') { // Solved
                game_over = 2;
            } else { // Aborted
                game_over = 3;
            }
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'W')
        { // Edgework setup
            if (ktomeCAN.can_msg[1] == '0') { serial_vowel = false; }
            else { serial_vowel = true; }
            if (ktomeCAN.can_msg[2] == '1') { serial_odd = true;}
            else { serial_odd = false; }
            battery_number = ktomeCAN.can_msg[3] - '0';
            if (ktomeCAN.can_msg[4] == '1') { ind_car = true;}
            else { ind_car = false; }
            if (ktomeCAN.can_msg[5] == '1') { ind_frk = true;}
            else { ind_frk = false; }
            if (ktomeCAN.can_msg[6] == '1') { serial_port = true;}
            else { serial_port = false; }
            if (ktomeCAN.can_msg[7] == '1') { parallel_port = true;}
            else { parallel_port = false; }
            module.widgetUpdate(serial_vowel, serial_odd, battery_number, ind_car, ind_frk, serial_port, parallel_port);
        }
        else if (ktomeCAN.can_msg[0] == 'S') { // Serial number

        }
        else if (ktomeCAN.can_msg[0] == 'X') { // Strike count
            strike_count = ktomeCAN.can_msg[1] - '0';
            module.strikeUpdate(strike_count);
            DEBUG_PRINTLN("Received X\n");
            if (!module.isAwake()) {
                if (random(10) > 0){
                    module.activate();
                }
            }
        }
        else if (ktomeCAN.can_msg[0] == 'H') { // Heartbeat on second tick

        }
        else if (ktomeCAN.can_msg[0] == 'T') { // Time on display
            String timer_digits = "";
            for (byte ii = 1; ii < 5; ii++){
                timer_digits = timer_digits + ktomeCAN.can_msg[ii];
            }
            byte return_byte = module.timerUpdate(timer_digits);
            if (return_byte == 1) {
                strikeTrigger();
            } else if (return_byte == 2) {
                solveTrigger();
            }
        } else if (ktomeCAN.can_msg[0] == 'N' && game_mode == 3) { // Needy activation time
            if (!module.isAwake()) {
                module.activate();
            }
        }
        else if (ktomeCAN.can_msg[0] == 'D' && game_mode == 3) { // Other module defused
            DEBUG_PRINTLN("Received D\n");
            if (!module.isAwake()) {
                if (random(10) > 0){
                module.activate();
                }
            }
        }
    }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
