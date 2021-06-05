//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 13/04/21
//======================================================================
//
//  Module: The Simon module (Slave, Standard, Vanilla)
//  version 0.6.1
//
//  Version goals: Object-oriented implementation of Simon logic
//
//======================================================================

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
#include <KTOME_Simon.h>
#include <config.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

#define PIN_LED_CAN GPIO_NUM_12
#define MOD_STATUS_R GPIO_NUM_32
#define MOD_STATUS_G GPIO_NUM_33

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
bool simon_interacted = false;
bool game_aborted = false;

// Timer
long thismillis;
// byte strikenumber = 0;

// Widgets
// char serial_number[7]; // Will this variable ever be needed by a module, when
// vowel, odd and even variables would do?
bool serial_vowel;
// bool serial_odd;
// bool serial_even;
// byte battery_number;
// bool parallel_port;
// bool ind_frk;
// bool ind_car;
byte strike_count;

// Simon specific
KTOME_Simon simon;

// CAN
int CAN_ID;

// Led* leds = new Led[11];
LedBlinkable leds[3]; // Move back to pointer array?!
byte led_pin_array[] = {PIN_LED_CAN, MOD_STATUS_R, MOD_STATUS_G};

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
void setup()
{
    // Start serial connection
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("== KTOME: Module (Simon) ==");

    // Start CAN bus
    CAN_ID = CONFIG_CAN_MODULE_TYPE | CONFIG_CAN_MODULE_NUM;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start();
    // start the CAN bus at 500 kbps
    if (!ktomeCAN.start())
    {
        Serial.println("Starting CAN failed!");
        while (1)
            ;
    }
    Serial.print("My ID is:   0b");
    ktomeCAN.padZeros(CAN_ID);
    Serial.println(CAN_ID, BIN);

    // Randomiser
    esp_random();

    // Setup objects
    simon.start();

    for (byte ii = 0; ii < 3; ii++)
    {
        leds[ii].init(led_pin_array[ii]);
    }
}

void loop()
{
    switch (gamemode)
    {
        case 0:
            carPark();
            break;
        case 1: // Module poll
            Serial.println(F("Module search..."));
            gameReset();
            initialisation();
            carPark();
            break;
        case 2: // Game in set-up
            Serial.println(F("Game set-up..."));
            carPark();
            break;
        case 3: // Game running
            Serial.println(F("Game starting!"));
            game_running = true;
            gameRunning();
            break;
        case 4: // Game wash-up: stand-by state, showing outcome and waiting for new
                // game to be trigger from phone
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

void carPark()
{
    holding = true;
    Serial.println("Script parked - waiting for direction...");
    while (holding)
    {
        do
        {
            CANInbox();
        } while (ktomeCAN.isMessageWaiting());

        byte return_byte;
        return_byte = simon.update();
        if ((return_byte < 4) && (game_running))
        {
            Serial.print("return byte: ");
            Serial.println(return_byte, BIN);
            byte can_num = 4;
            for (byte ii = 0; ii < 4; ii++)
            {
                if (bitRead(return_byte, ii) == 1)
                {
                    can_num = ii;
                }
            }
            if (simon_interacted){
                char CAN_message[4];
                CAN_message[0] = 'u';
                CAN_message[1] = can_num + '0'; // 0 = red, 1 = b, 2 = g, 3 = y
                CAN_message[2] = '1';
                CAN_message[3] = '\0';
                leds[0].write(true);
                ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, 3);
                leds[0].write(false);
            }
        }

        leds[1].update();
        delay(1);
    }
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : gamemode = 1
//**********************************************************************

// Comm with modules to determine what's connected

void initialisation()
{
    char CAN_message[2];
    CAN_message[0] = 'p';
    CAN_message[1] = '\0';
    leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    leds[0].write(false);
}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

void gameSetup()
{
    game_ready = false;
    simon.reset();
    simon.generate();

    char CAN_message[2] = "i";
    leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 5);
    leds[0].write(false);
    game_ready = true;
}

// void manualCheck() {
//   String temp_msg;
//   temp_msg = simon.getManual();

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

void gameReset()
{
    module_defused = false;
    simon_interacted = false;
    simon.reset(); // Is this needed if reset is done before game gen?
    // manual_blink = false;
    strike_light_state = false;
    for (byte ii = 0; ii < 3; ii++)
    {
        leds[ii].write(false);
    }
}

void gameRunning()
{

    simon.game_running = true;

    while (gamemode == 3)
    {
        thismillis = millis();
        if (!module_defused)
        {
            /* Input check */
            inputCheck();
            /* Defuse check */
            module_defused = simon.isDefused();
        }
        /* Output update */
        byte return_byte;
        return_byte = simon.update();
        if (return_byte > 0)
        {
            byte can_num = 4;
            for (byte ii = 0; ii < 4; ii++)
            {
                if (bitRead(return_byte, ii) == 1)
                {
                    can_num = ii;
                }
            }
            if (simon_interacted){
                char CAN_message[4];
                CAN_message[0] = 'u';
                CAN_message[1] = can_num + '0'; // 0 = red, 1 = b, 2 = g, 3 = y
                CAN_message[2] = '1';
                CAN_message[3] = '\0';
                leds[0].write(true);
                ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, 3);
                leds[0].write(false);
            }
        }

        leds[1].update();

        // Check incoming messages
        CANInbox();
    }
}

void strikeTrigger()
{
    char CAN_message[] = "x";
    leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    leds[0].write(false);

    strike_light_timer = thismillis + strike_light_flash;
    leds[1].blink(true, 500, 1);
}

void defuseTrigger()
{
    char CAN_message[] = "d";
    leds[0].write(true);
    ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
    leds[0].write(false);
    leds[1].write(false);
    leds[2].write(true);

    module_defused = true;
    simon.game_running = false;
}

void explodeFX(bool target_state)
{
    if (target_state)
    { // Only turn off, never flash this when exploding
        leds[2].write(false);
        leds[1].blink(true, 100, 1);
        simon.explode();
    }  else {
        leds[1].write(false);
        leds[2].write(false);
    }
}

void inputCheck()
{
    byte return_byte;
    return_byte = simon.inputCheck(); // defuse bit, strike bit, no action,
                                      // bit, red bit, blue bit, green bit, yellow bit.
                                      // Return 0 if not interacted with
    if (return_byte > 0)
    {
        simon_interacted = true;
        byte can_num = 4;
        for (byte ii = 0; ii < 4; ii++)
        {
            if (bitRead(return_byte, ii) == 1)
            {
                can_num = ii;
            }
        }
        if (simon_interacted){
            char CAN_message[4];
            CAN_message[0] = 'u';
            CAN_message[1] = can_num + '0'; // 0 = red, 1 = b, 2 = g, 3 = y
            CAN_message[2] = '0';
            CAN_message[3] = '\0';
            leds[0].write(true);
            ktomeCAN.send(can_ids.Widgets | CAN_ID, CAN_message, 3);
            leds[0].write(false);
        }
    }
    if (bitRead(return_byte, 5) == 1)
    { // Wrong
        strikeTrigger();
    }
    else if (bitRead(return_byte, 6) == 1)
    { // Right
        if (simon.isDefused())
        {
            defuseTrigger();
        }
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
            Serial.println("This module is present!");
            gamemode = 1;
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'I')
        { // Setup a game scenario
            Serial.println("This module was asked to setup a scenario!");
            gamemode = 2;
            gameSetup();
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'C' &&
                 gamemode == 2)
        { // Game manual setup call
            Serial.println("This module was asked about it's manual setup!");
            Serial.println("ERROR: This module should not have been asked for a "
                           "manual setup!");
            // manualCheck();
            //      holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'M' &&
                 gamemode == 2)
        { // Game manual check call
            Serial.println("This module was asked if manual setup was successful!");
            leds[1].write(false);
        }
        else if (ktomeCAN.can_msg[0] == 'A' && gamemode == 2)
        { // Game start
            Serial.println("This module was asked to start the game!");
            //
            gamemode = 3;
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'Z')
        { // Game stop
            Serial.println("This module was asked to stop the game!");
            gamemode = 4;
            game_running = false;
            simon.game_running = false;
            Serial.println(ktomeCAN.can_msg[1]);
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
        }
        else if (ktomeCAN.can_msg[0] == 'W')
        { // Edgework setup
            if (ktomeCAN.can_msg[1] == '0')
            {
                serial_vowel = false;
            }
            else
            {
                serial_vowel = true;
            }
            simon.vowelSolution(serial_vowel);
        }
        else if (ktomeCAN.can_msg[0] == 'S')
        { // Serial number
        }
        else if (ktomeCAN.can_msg[0] == 'X')
        { // Strike count
            strike_count = ktomeCAN.can_msg[1] - '0';
            simon.strikeSolution(strike_count);
        }
        else if (ktomeCAN.can_msg[0] == 'H')
        { // Heartbeat on second tick
        }
        else if (ktomeCAN.can_msg[0] == 'T')
        { // Time on display
        }
    }
}

//**********************************************************************
// FUNCTIONS: Misc. Functions
//**********************************************************************
