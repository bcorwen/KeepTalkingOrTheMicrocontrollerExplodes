//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 01/05/21
//======================================================================
//
//  Module: Timer (MASTER)
//
//  version 0.6.2
//
//
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
#include <KTOME_common.h>
#include <KTOME_Timer.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_LEDBackpack.h>
#include <config.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

//#define PIN_PIEZO     GPIO_NUM_2
#define PIN_LED_CAN GPIO_NUM_12

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool manual_check = 0;
bool manual_confirm = 0;
bool manual_ready = false;
bool manual_abort = false;
bool manual_start = false;
byte module_to_check = 0;
byte checking_stage = 0;
bool game_running = false;
bool game_win = false;
bool explosion_fx = false;

int module_array[12];
bool module_detected = false;
bool module_inited = false;
byte module_count = 0;
byte module_count_solvable = 0;
byte solved_count = 0;
bool module_solved_array[12];

bool send_time = false;

// Timer
bool strike_flag = false;
int strike_culprit; // NEW VARIABLE - track the last module which caused a strike, for debrief
char sec_tick_over; // DELETE?
// byte time_scale;    // Quadruple the time scale: 4 = 1x speed (normal), 5 = 1.25x speed (1 strike), etc...

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
int CAN_ID;

char CAN_message[9];

Led leds;
byte led_pin_array = PIN_LED_CAN;
KTOME_Timer timer;

//Functions

void carPark();
void phoneConnect();
void initialisation();
void phoneSetup();
void newGameMessage();
void moduleCheck();
void widgetGenerate();
String widgetGenerateBattery(byte widget_count, String BLE_msg);
String widgetGeneratePort(byte widget_count, String BLE_msg);
String widgetGenerateIndicator(byte widget_count, String BLE_msg);
void serialGenerate();
char charGenerator(bool alphanum_type);
void gameReset();
void gameRunning();
void explodeFX(bool target_state);
void timerCalc();
void strikeCalc();
void solveUpdate(uint32_t id);
void stopMessages();
void secondTick();
void CANInbox();
void BLESend(String msg_data);

// BLE
bool deviceConnected = false;
//bool deviceConnected_prev = false;
String BLE_value;
String BLE_state = "";
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
                    //4fafc201-1fb5-459e-8fcc-c5c9c331914b
#define CHARACTERISTIC_UUID_RX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_TX "beb5483f-36e1-4688-b7f5-ea07361b26a9"
                              //beb5483f-36e1-4688-b7f5-ea07361b26a8

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLEAdvertising *pAdvertising = NULL;
uint8_t txValue = 0;

String temp_holder;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        // delay(100);
    };
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        // delay(100);
        // pServer->startAdvertising();
        //      gamemode = 0;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pRxCharacteristic)
    {
        std::string rxValue = pRxCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            BLE_value = "";
            for (int i = 0; i < rxValue.length(); i++)
            {
                BLE_value = BLE_value + rxValue[i];
            }
            Serial.print("BLE_value = ");
            Serial.println(BLE_value);

            // Add in switch/case or if/else to set flags based on BLE_value input.
            BLE_state = "";
            if (BLE_value.substring(0, 2) == "t=")
            {
                float temp_float;
                temp_holder = BLE_value.substring(2);
                temp_float = temp_holder.toFloat();
                //          gamelength = long(temp_float * 60000);
                // gamelength = long(temp_float * 60000000);
                timer.setGameLength(int32_t(temp_float * (60 * timer.us_conv)));
                BLE_state = "ok";
            }
            else if (BLE_value.substring(0, 2) == "h=")
            { // Hardcore selector
                if (BLE_value.substring(2) == "0")
                {
                    timer.setHardcore(false);
                }
                else
                {
                    timer.setHardcore(true);
                }
                BLE_state = "ok";
            }
            else if (BLE_value == "I")
            {                 // App moving from BLE connection screen to Game Manager or manual re-poll of connected modules
                gamemode = 1; // Now (re-)start polling connected modules
                holding = false;
                //
            }
            else if (BLE_value == "C")
            { // App starting module manual setup
                gamemode = 2;
                manual_abort = true;
                manual_start = true;
                holding = false;
            }
            else if (BLE_value == ">")
            {
                manual_check = true;
                holding = false;
            }
            else if (BLE_value == "<")
            {
                manual_abort = true;
                holding = false;
            }
            else if (BLE_value == "A")
            {
                gamemode = 3;
                holding = false;
            }
            else if (BLE_value == "Z")
            {
                gamemode = 4;
                holding = false;
            }
            pTxCharacteristic->setValue(BLE_state.c_str()); // Return status
            pTxCharacteristic->notify();
            //        Serial.print("BLE_state = ");
            //        Serial.println(BLE_state);
        }
    }
};

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup()
{

    // Start serial connection
    Serial.begin(115200);
    // while (!Serial);
    Serial.println("== KTOME: Timer ==");

    // Start CAN bus
    CAN_ID = CONFIG_CAN_MODULE_TYPE | CONFIG_CAN_MODULE_NUM;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start(14, 13);
    // start the CAN bus at 500 kbps
    // if (!ktomeCAN.start())
    // {
    //     Serial.println("Starting CAN failed!");
    //     while (1);
    // } else {
    //     Serial.println("CAN started!");
    // }
    Serial.print("My ID is:   0b");
    ktomeCAN.padZeros(CAN_ID);
    Serial.println(CAN_ID, BIN);

    // Start BLE
    // BLEDevice::init("KTOME");
    // pServer = BLEDevice::createServer();
    // pServer->setCallbacks(new MyServerCallbacks());
    // BLEService *pService = pServer->createService(SERVICE_UUID);
    // pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    // pTxCharacteristic->addDescriptor(new BLE2902());
    // pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    // pRxCharacteristic->addDescriptor(new BLE2902());
    // pRxCharacteristic->setCallbacks(new MyCallbacks());
    // pService->start();
    // pServer->getAdvertising()->start();

    BLEDevice::init("KTOME");
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->addDescriptor(new BLE2902());
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pServer->setCallbacks(new MyServerCallbacks());
    pService->start();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    // Randomiser
    esp_random();

    timer.init();
    // timerInit();
}

void loop()
{

    switch (gamemode)
    {
    case 0: // Connect to phone
        phoneConnect();
        carPark();
        break;
    case 1: // Module poll
        Serial.println(F("Module search..."));
        gameReset();
        initialisation();
        carPark();
        break;
    case 2: // Game (manual) set-up
        Serial.println(F("Game set-up..."));
        phoneSetup();        
        // visual_fx(false);
        if (!manual_start) {carPark();}
        break;
    case 3: // Game running
        Serial.println(F("Game starting!"));
        gameRunning();
        break;
    case 4: // Game wash-up: stand-by state, showing outcome and waiting for new game to be trigger from phone
        // Show outcome
        stopMessages();
        if (explosion_fx)
        {
            explodeFX(true);
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
        do {
            CANInbox();
        } while (ktomeCAN.isMessageWaiting());
        timer.updateTimer(gamemode, game_win);
        timer.updateStrikes();
        delay(1);
    }
}

//**********************************************************************
// FUNCTIONS: Companion App Setup : gamemode = 0
//**********************************************************************

void phoneConnect()
{
    Serial.println("Waiting for phone connection...");
    while (!deviceConnected)
    {
        delay(1);
        // Serial.println("waiting...");
    }
    Serial.println("Phone connected!");
}

//**********************************************************************
// FUNCTIONS: Game Initialisation : gamemode = 1
//**********************************************************************

void initialisation()
{ // Comm with modules to determine what's connected

    // byte max_module_copies = 11;
    char CAN_message[2];
    memset(module_array, 0, sizeof(module_array));
    memset(module_solved_array, 0, sizeof(module_solved_array));

    Serial.println("Starting search for modules...");
    module_count = 0;
    module_count_solvable = 0;

    Serial.println("CAN poll sending, time to wait and see...");

    // Why are we cycling through all modules and sending a message for each? Just send one message to all!
    CAN_message[0] = 'P';
    CAN_message[1] = '\0';
    digitalWrite(PIN_LED_CAN, HIGH);
    ktomeCAN.send((can_ids.All_modules), CAN_message, 1);
    digitalWrite(PIN_LED_CAN, LOW);

    // delay(1000);

    bool message_waiting = false;
    long wait_time = millis() + 1000;
    while(wait_time > millis()) {
        // Serial.println("One second while we wait for messages...");
        do
        { // There are outstanding messages
            CANInbox();
            if (module_detected)
            {
                module_array[module_count] = (ktomeCAN.can_msg_id & (can_ids.All_modules | can_ids.MUID_15));
                Serial.print("Module count: ");
                Serial.println(module_count + 1);
                Serial.print("Module id: 0b");
                ktomeCAN.padZeros(module_array[module_count]);
                Serial.println(module_array[module_count], BIN);
                if ((module_array[module_count] & (can_ids.All_standards)) > 0)
                {
                    module_count_solvable++;
                }
                module_count++;
                module_detected = false;
            }
            message_waiting = ktomeCAN.isMessageWaiting();
            // delay(100);
        } while (message_waiting);
    }

    Serial.println("All modules polled. Connected modules: ");
    for (byte ii = 0; ii < module_count; ii++)
    {
        Serial.print("0b");
        ktomeCAN.padZeros(module_array[ii]);
        Serial.println(module_array[ii], BIN);
    }
    Serial.print("Total connected modules: ");
    Serial.println(module_count);
    Serial.print("Solvable modules: ");
    Serial.println(module_count_solvable);

    for (byte msg_part = 0; msg_part < 2; msg_part++)
    {
        String BLE_msg;
        byte spec_mod_count;
        BLE_msg = "i ";
        BLE_msg += (msg_part + 1);
        for (byte msg_mod = (1 + (7 * msg_part)); msg_mod < (8 + (7 * msg_part)); msg_mod++)
        {
            spec_mod_count = 0;
            for (byte msg_num = 0; msg_num < module_count; msg_num++)
            {
                if ((module_array[msg_num] & can_ids.All_modules) == (can_ids.Master >> msg_mod))
                {
                    spec_mod_count++;
                }
            }
            BLE_msg += ' ';
            BLE_msg += char(spec_mod_count + '0'); // Space=separated list of numbers corresponding to the amount of modules detected in the bomb
        }
        BLESend(BLE_msg);
        delay(200);
    }
}

//**********************************************************************
// FUNCTIONS: Game Setup : gamemode = 2
//**********************************************************************

// Comm with phone to set up a game

void phoneSetup()
{
    if (manual_start)
    {
        game_ready = false;
        manual_abort = false;
        manual_check = false;
        manual_start = false;

        timer.setGameLength(timer.getTimeLeft());

        // while (!manual_abort)
        // {
            newGameMessage(); // Get modules to generate a new game
            if (!manual_abort) {
                serialGenerate(); // Generate serial #
                widgetGenerate(); // Generate widgets
            }
            if (!manual_abort) {
                moduleCheck(); // Look through all of the polled modules and pass through manual setup checks
            }

            if (!manual_abort) { // If manual setup is completed successfully, distribute widget/serial info to all modules
                char CAN_message[9];
                CAN_message[0] = 'W';
                if (serial_vowel) {CAN_message[1] = '1';}
                else {CAN_message[1] = '0';}

                if (serial_odd) {CAN_message[2] = '1';}
                else {CAN_message[2] = '0';}

                CAN_message[3] = (battery_number + '0');

                if (ind_car) {CAN_message[4] = '1';}
                else{CAN_message[4] = '0';}

                if (ind_frk) {CAN_message[5] = '1';}
                else{CAN_message[5] = '0';
                }
                if (port_parallel){CAN_message[6] = '1';}
                else{CAN_message[6] = '0';}

                if (port_serial){CAN_message[7] = '1';}
                else{CAN_message[7] = '0';}

                CAN_message[8] = '\0';

                digitalWrite(PIN_LED_CAN, HIGH);
                ktomeCAN.send((can_ids.All_modules), CAN_message, 8);
                digitalWrite(PIN_LED_CAN, LOW);
            }
        // }
    }
}

void newGameMessage()
{
    char CAN_message[2];

    // Send request to modules to generate games
    CAN_message[0] = 'I';
    CAN_message[1] = '\0';
    digitalWrite(PIN_LED_CAN, HIGH);
    ktomeCAN.send((can_ids.All_modules), CAN_message, 1);
    digitalWrite(PIN_LED_CAN, LOW);
    // delay(300);
    // Need to check that all modules have replied that they have completed their setup!

    byte mods_setup = 0;
    do
    { // There are outstanding modules to generate a game
        CANInbox();
        if (module_inited)
        {
            Serial.print("Module 0b");
            ktomeCAN.padZeros(ktomeCAN.can_msg_id - can_ids.Master);
            Serial.print(ktomeCAN.can_msg_id - can_ids.Master, BIN);
            Serial.println(" has set-up a scenario!");
            mods_setup++;
            Serial.print(module_count - mods_setup);
            Serial.println(" modules are left to report in.");
            module_inited = false;
        }
        if (manual_abort)
        {
            break;
        }
    } while (mods_setup != module_count);
}

void moduleCheck()
{
    // bool manual_error = false;
    String BLE_msg;
    module_to_check = 0;
    checking_stage = 1;
    bool awaiting_message;

    do {
        awaiting_message = true;

        if (checking_stage == 1) // Next module to check, if manual needed sent to module over CAN for info to feed to app
        {
            Serial.print("Checking manual for module ");
            Serial.print(module_to_check + 1);
            Serial.print(" of ");
            Serial.println(module_count);

            BLE_msg = "c ";
            // Depending on the module, fetch its manual set up info and pass on to the app
            if ((module_array[module_to_check] & can_ids.All_manuals) > 0) // This module needs manual setup
            { 
                Serial.println("Needs manual setup!");
                CAN_message[0] = 'C';
                CAN_message[1] = '\0';
                digitalWrite(PIN_LED_CAN, HIGH);
                ktomeCAN.send(module_array[module_to_check], CAN_message, 1);
                digitalWrite(PIN_LED_CAN, LOW);

                checking_stage = 2; // CAN sent to module, now awaiting response
                awaiting_message = true;
            }
            else // Module doesn't need manual setup, so skip to next module
            {
                Serial.println("Doesn't need manual setup, next module...");
                module_to_check++;
                awaiting_message = false;
            }
            
        }
        else if (checking_stage == 2) // Manual info has been sent to app, app has just responded with confirmation prompt
        {
            Serial.print("Reply from module ");
            Serial.println(module_to_check + 1);

            if ((module_array[module_to_check] & can_ids.Wires) > 0)
            { // Module needing setup is WIRES
                Serial.println("Wires setup incoming...");
                Serial.print("Wire colours: ");
                byte wire_colours[6];
                for (byte jj = 0; jj<6; jj++){
                    wire_colours[jj] = ktomeCAN.can_msg[jj+1] - '0';
                    Serial.print(wire_colours[jj]);
                }
                Serial.println();
                BLE_msg += 'w';
                for (byte jj = 0; jj < 6; jj++)
                {
                    BLE_msg += ' ';
                    BLE_msg += ktomeCAN.can_msg[jj+1] + 1 - '0';
                }
                BLESend(BLE_msg);
                Serial.println("Starting wires check loop:");
            }

            else if ((module_array[module_to_check] & can_ids.Button) > 0)
            { // Module needing setup is BUTTON
                Serial.println("Button setup incoming...");
                byte button_r1 = ktomeCAN.can_msg[1] - '0';
                byte button_r2 = ktomeCAN.can_msg[2] - '0';
                Serial.print("Button IDs: ");
                Serial.print(button_r1);
                Serial.print(", ");
                Serial.println(button_r2);
                BLE_msg += 'b';
                for (byte jj = 0; jj < 2; jj++)
                {
                    BLE_msg += ' ';
                    BLE_msg += ktomeCAN.can_msg[jj + 1] - '0';
                }
                BLESend(BLE_msg);
                Serial.println("Starting button check loop:");
            }

            else if ((module_array[module_to_check] & can_ids.Keypad) > 0)
            { // Module needing setup is KEYPAD
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
                for (byte jj = 0; jj < 4; jj++)
                {
                    BLE_msg += ' ';
                    BLE_msg += ktomeCAN.can_msg[jj + 1] - '0';
                }
                BLESend(BLE_msg);
                Serial.println("Starting keypad check loop:");

            }
            else if ((module_array[module_to_check] & can_ids.CWires) > 0)
            { // Module needing setup is COMPLICATED WIRES

            }
            else if ((module_array[module_to_check] & can_ids.WireSeq) > 0)
            { // Module needing setup is WIRE SEQUENCE

            }
            checking_stage = 3; // Info sent to app, awaiting confirmation from app
            awaiting_message = true;
        }
        else if (checking_stage == 3) // Info sent to app, confirmation sent back from app
        {
            CAN_message[0] = 'M';
            CAN_message[1] = '\0';
            digitalWrite(PIN_LED_CAN, HIGH);
            ktomeCAN.send(module_array[module_to_check], CAN_message, 1);
            digitalWrite(PIN_LED_CAN, LOW);
            if ((module_array[module_to_check] & can_ids.Button) > 0 || (module_array[module_to_check] & can_ids.Keypad) > 0) // Confirm from app received, all done so next module
            {
                Serial.println("User thinks it is set up, next module...");
                checking_stage = 1;
                module_to_check++;
                awaiting_message = false;
            }
            else // Module needs 2nd manual check
            {
                Serial.println("User thinks it is set up, begin 2nd manual check...");
                checking_stage = 4;
                awaiting_message = true;
            }
        }
        else if (checking_stage == 4) // 2nd manual check sent, module replies with if it's ok or needs a re-attempt
        {
            if (!manual_confirm) // Manual with second checking rejects set up is correct
            {
                Serial.println("Module believes set up is wrong! Message app to try again...");
                BLE_msg = "c X";
                BLESend(BLE_msg);
                checking_stage = 3;
                awaiting_message = true;
            } else if (manual_confirm) // Manual module replied that set up is complete (including second checking)
            {
                Serial.println("Module believes it is set up! Will exit while loop and continue on to next module...");
                checking_stage = 1;
                module_to_check++;
                awaiting_message = false;
            }
        }

        if (awaiting_message)
        {
            carPark();
            if(!manual_check)
            {
                CAN_message[0] = 'M';
                CAN_message[1] = '\0';
                digitalWrite(PIN_LED_CAN, HIGH);
                ktomeCAN.send(module_array[module_to_check], CAN_message, 1);
                digitalWrite(PIN_LED_CAN, LOW);
                break;
            }
            manual_check = false;
        }

    } while (module_to_check < module_count);

    if (!manual_abort)
    {
        BLE_msg = "c Y";
        BLESend(BLE_msg);
    }

}

void widgetGenerate()
{
    battery_number = 0;
    port_serial = false;
    port_serial = false;
    ind_car = false;
    ind_frk = false;
    byte widgets_total = 5;
    byte widget_type[3] = {0,0,0};
    String BLE_msg = "c e";

    for (byte ii = 0; ii < widgets_total; ii++)
    {
        widget_type[random(3)]++;
    }

    // Serial.print("widgets: ");
    // Serial.print(widget_type[0]);
    // Serial.print(", ");
    // Serial.print(widget_type[1]);
    // Serial.print(", ");
    // Serial.println(widget_type[2]);

    if (widget_type[0]>0){
        BLE_msg = widgetGenerateBattery(widget_type[0], BLE_msg);
    }
    if (widget_type[1]>0){
        BLE_msg = widgetGeneratePort(widget_type[1], BLE_msg);
    }
    if (widget_type[2]>0){
        BLE_msg = widgetGenerateIndicator(widget_type[2], BLE_msg);
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
    carPark(); // Wait for user to complete manual setup - this is odd...
    if (manual_check)
    { // User states this module is setup
        Serial.println("Widgets generated!");
    }
    else
    {
        Serial.println("Expecting module check, but received another message!");
           manual_abort = true;
        //    break;
    }

}

String widgetGenerateBattery(byte widget_count, String BLE_msg)
{
    for (byte ii = 0; ii < widget_count; ii++)
    {
        Serial.println("Generating battery...");
        byte selection_pick = 0;
        if (random(2) < 1)
        { //50% chance of a D battery
            battery_number += 1;
        }
        else
        {
            selection_pick = 1;
            battery_number += 2;
        }
        BLE_msg = BLE_msg + " b" + selection_pick;
        Serial.println(BLE_msg);
    }
    return BLE_msg;
}

String widgetGeneratePort(byte widget_count, String BLE_msg)
{
    for (byte ii = 0; ii < widget_count; ii++)
    {
        Serial.println("Generating Port...");
        byte selection_pick = 0;
        if (random(2) < 1) { // Serial/parallel ports
            if (random(2) < 1)
            { //50% chance of parallel
                selection_pick += 2;
                port_parallel = true;
            }
            if (random(2) < 1)
            { //50% chance of a serial
                selection_pick += 1;
                port_serial = true;
            }
        }
        BLE_msg = BLE_msg + " p" + selection_pick;
        Serial.println(BLE_msg);
    }
    return BLE_msg;
}

String widgetGenerateIndicator(byte widget_count, String BLE_msg)
{
    byte list_indicators[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // SND, CLR, CAR, IND, FRQ, SIG, NSA, MSA, TRN, BOB, FRK
    for (byte ii = 0; ii < widget_count; ii++)
    {
        Serial.println("Generating Indicator...");
        byte selection_pick = 0;
        do
        {
            selection_pick = list_indicators[random(11)]; // Pick from the indicator list
        } while (selection_pick == 0);                       // Try again if this has already been selected previously
        list_indicators[selection_pick-1] = 0;

        if (random(3) < 1) // 1/3 chance indicator is off
        {
            selection_pick += 96;
        }
        else // 2/3 chance indicator is on
        {
            selection_pick += 64;
        }

        if (selection_pick == 24)
        { // CAR lit
            ind_car = true;
        }
        else if (selection_pick == 32)
        { // FRK lit
            ind_frk = true;
        }

        BLE_msg = BLE_msg + " i" + char(selection_pick);
        Serial.println(BLE_msg);
    }
    return BLE_msg;
}

void serialGenerate()
{
    serial_vowel = 0;
    serial_odd = 0;
    serial_even = 0;

    for (int i = 0; i < 6; i++)
    {
        if (i == 3 || i == 4)
        {serial_number[i] = charGenerator(1);}
        else if (i == 2)
        {serial_number[i] = charGenerator(0);}
        else if (i == 5)
        {
            serial_number[i] = charGenerator(0);
            if (serial_number[i] % 2 == 1)
            {serial_odd = 1;}
            else
            {serial_even = 1;}
        }
        else
        {serial_number[i] = charGenerator(2);}
    }
    serial_number[6] = '\0';
    Serial.print(F("Serial #: "));
    Serial.println(serial_number);

    char can_msg[8];
    can_msg[0] = 'S';
    for (byte i = 0; i < 6; i++)
    {
        can_msg[i + 1] = serial_number[i];
    }
    can_msg[7] = '\0';
    digitalWrite(PIN_LED_CAN, HIGH);
    ktomeCAN.send(can_ids.Widgets, can_msg, 7);
    digitalWrite(PIN_LED_CAN, LOW);

    Serial.print("Vowels?: ");
    Serial.println(serial_vowel);
    Serial.print("Even?: ");
    Serial.println(serial_even);
    Serial.print("Odds?: ");
    Serial.println(serial_odd);
}

char charGenerator(bool alphanum_type)
{
    char temp;
    byte dice_roll;

    if (alphanum_type == 0)
    {dice_roll = random(0, 10);}
    else if (alphanum_type == 1)
    {dice_roll = random(10, 35);}
    else
    {dice_roll = random(0, 35);}

    if (dice_roll < 10)
    {temp = dice_roll + '0';}
    else
    {
        temp = dice_roll + 'A' - 10;
        if (temp == 'A' || temp == 'E' || temp == 'I' || temp == 'O' || temp == 'U')
        {
            serial_vowel = 1;
            if (temp == 'O')
            {temp = 'E';}
        }
        else if (temp == 'Y')
        {temp = 'Z';}
    }
    return temp;
}

//**********************************************************************
// FUNCTIONS: Game Managers
//**********************************************************************

void gameReset()
{

    game_win = false;
    game_running = false;
    solved_count = 0;
    
    timer.reset();

    memset(module_solved_array, 0, sizeof(module_solved_array));
}

void gameRunning()
{

    bool message_waiting = false;

    String BLE_msg = "m ";
    BLE_msg = BLE_msg + module_count_solvable;
    BLESend(BLE_msg);

    BLE_msg = "s 0";
    BLESend(BLE_msg);

    delay(random(2000) + 3000);

    CAN_message[0] = 'A';
    CAN_message[1] = '\0';
    digitalWrite(PIN_LED_CAN, HIGH);
    ktomeCAN.send((can_ids.All_modules), CAN_message, 1);
    digitalWrite(PIN_LED_CAN, LOW);

    game_running = true;
    // thismillis = micros();
    timer.gameStart();

    while (gamemode == 3)
    {
        // Check CAN inbox
        do
        {
            CANInbox();
            message_waiting = ktomeCAN.isMessageWaiting();
        } while (message_waiting);

        // Solve update

        // Strike update
        strikeCalc();

        // Timer update
        timerCalc();

        // Handle CAN replies
        if (send_time) {
            char can_msg[6];
            String time_str = timer.getTimeStr();
            can_msg[0] = 'T';
            can_msg[1] = time_str.charAt(0);
            can_msg[2] = time_str.charAt(1);
            can_msg[3] = time_str.charAt(2);
            can_msg[4] = time_str.charAt(3);
            can_msg[5] = '\0';
            Serial.print("Char array to send: ");
            Serial.println(can_msg);
            digitalWrite(PIN_LED_CAN, HIGH);
            ktomeCAN.send((can_ids.All_modules), can_msg, 5);
            digitalWrite(PIN_LED_CAN, LOW);
            send_time = false;
        }
    }
}

void explodeFX(bool target_state)
{
    // bool draw_colon;
    if (target_state)
    { // Triggers on explosion - flash lights on and ready for them to switch off
        timer.explode();
    }
    else
    { // Triggers on defusal and reset
        if (game_win)
        {
            timer.defuse();
        }
        else
        {
            timer.reset();
        }
    }
}

//**********************************************************************
// FUNCTIONS: Game functions
//**********************************************************************

void timerCalc()
{

    timer.updateTimer(gamemode, game_win);

    if (timer.hasTimerExpired())
    {
        // Lose Game!
        strike_culprit = can_ids.Master;
        gamemode = 4;
    }

    secondTick();
}

void strikeCalc()
{

    if (strike_flag)
    { //
        strike_flag = false;

        timer.addStrikes(1);

        Serial.print("Strike from 0b");
        ktomeCAN.padZeros(strike_culprit);
        Serial.println(strike_culprit, BIN);
        Serial.print("Current number of strikes: ");
        Serial.println(timer.getStrikes());

        String BLE_msg = "x ";
        BLE_msg = BLE_msg + timer.getStrikes();
        BLESend(BLE_msg);

        // if (strike_number == 2)
        // {
        //     blinktime = millis() + blinkperiod;
        //     blinkbool = true;
        // }

        if (!timer.reachedStrikeLimit())
        {
            // Play buzzer
            char CAN_message[3] = "X ";
            CAN_message[1] = '0' + timer.getStrikes();
            digitalWrite(PIN_LED_CAN, HIGH);
            ktomeCAN.send((can_ids.All_modules), CAN_message, 2);
            digitalWrite(PIN_LED_CAN, LOW);
        }
        else
        {
            // Game over
            gamemode = 4; // Send to wash-up
        }
    }

    timer.updateStrikes();
}

void solveUpdate(uint32_t id)
{

    id = (id & (can_ids.All_modules | can_ids.MUID_15));

    Serial.print("Defused module is 0b");
    ktomeCAN.padZeros(id);
    Serial.println(id, BIN);

    for (byte ii = 0; ii < 12; ii++)
    {
        Serial.println((id & (can_ids.All_modules | can_ids.MUID_15)), BIN);
        Serial.println(module_array[ii], BIN);
        if (id == module_array[ii])
        {
            module_solved_array[ii] = true;
            Serial.println("FOUND");
            solved_count++;
            ii = 12;
            String BLE_msg = "s ";
            BLE_msg = BLE_msg + String(solved_count);
            BLESend(BLE_msg);
        }
        else
        {
            Serial.println("Module reported to be solved but it wasn't registered!");
        }
    }

    Serial.print("Number of solved modules: ");
    Serial.println(solved_count);

    if (solved_count == module_count_solvable)
    {
        // Game win condition
        Serial.println("Game won! All solvable modules have been defused.");
        gamemode = 4;
        game_win = true;
    }
}

void stopMessages()
{

    char CAN_message[] = "Z0";
    String BLE_msg = "";
    game_running = false;
    if (game_win)
    { // Game is won
        BLE_msg = "z d";
    }
    else if (timer.reachedStrikeLimit())
    { // Game lost due to strike-out
        BLE_msg = "z ";
        for (byte ii = 0; ii < 15; ii++)
        {
            if ((((strike_culprit & can_ids.All_standards) << ii) & can_ids.All_standards) == 0)
            {
                BLE_msg = BLE_msg + ii;
                ii = 15;
            }
        }
        CAN_message[1] = '1';
        // timerblinktime = thismillis + 100;
    }
    else if (timer.hasTimerExpired())
    { //  Game lost due to timer
        BLE_msg = "z 0";
        CAN_message[1] = '1';
        // timerblinktime = thismillis + 100;
    }
    else
    { // Game aborted
        BLE_msg = "z Z";
        CAN_message[1] = '2';
        timer.setStrikes(0);
    }
    BLESend(BLE_msg);
    digitalWrite(PIN_LED_CAN, HIGH);
    ktomeCAN.send((can_ids.All_modules), CAN_message, 2);
    digitalWrite(PIN_LED_CAN, LOW);
}

//**********************************************************************
// FUNCTIONS: Hardware
//**********************************************************************

void secondTick()
{

    if (timer.hasSecondTickedOver()){
        String BLE_msg;
        BLE_msg = "t ";
        String timestr = timer.getTimeStr();
        if (timer.getTimeLeft() >= (60 * timer.us_conv))
        { // Over a minute left - look at whole timer string
            BLE_msg = BLE_msg + timestr.charAt(0) + timestr.charAt(1) + " " + timestr.charAt(2) + timestr.charAt(3);
            BLESend(BLE_msg);
            if (game_running)
            {
                CAN_message[0] = 'H';
                CAN_message[1] = '\0';
                digitalWrite(PIN_LED_CAN, HIGH);
                ktomeCAN.send((can_ids.Master), CAN_message, 1);
                digitalWrite(PIN_LED_CAN, LOW);
            }
        }
        else
        { // Under a minute, just look at first two chars
                BLE_msg = BLE_msg + "00 " + timestr.charAt(0) + timestr.charAt(1);
                BLESend(BLE_msg);
                if (game_running)
                {
                    CAN_message[0] = 'H';
                    CAN_message[1] = '\0';
                    digitalWrite(PIN_LED_CAN, HIGH);
                    ktomeCAN.send((can_ids.Master), CAN_message, 1);
                    digitalWrite(PIN_LED_CAN, LOW);
                }
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
        if (ktomeCAN.can_msg[0] == 'p' && gamemode == 1)
        {
            module_detected = true;
            Serial.println("Module detected!");
        }
        else if (ktomeCAN.can_msg[0] == 'i' && gamemode == 2)
        {
            module_inited = true;
            Serial.println("Module has declared it is setup!");
        }
        else if (ktomeCAN.can_msg[0] == 'c' && gamemode == 2)
        {
            manual_check = true;
            holding = false;
            Serial.println("Module is transmitting it's manual setup needs!");
        }
        else if (ktomeCAN.can_msg[0] == 'm' && gamemode == 2)
        {
            manual_check = true;
            if (ktomeCAN.can_msg[1] == '0')
            {
                Serial.println("Module does not agree manual setup is correct!");
                manual_confirm = false;
            }
            else
            {
                Serial.println("Module agrees that manual setup is correct!");
                manual_confirm = true;
            }
            holding = false;
        }
        else if (ktomeCAN.can_msg[0] == 'x' && gamemode == 3)
        {
            Serial.println("Module announces a strike!");
            strike_flag = true;
            strike_culprit = ktomeCAN.can_msg_id;
        }
        else if (ktomeCAN.can_msg[0] == 'd' && gamemode == 3)
        {
            Serial.println("Module announces its defusal!");
            solveUpdate(ktomeCAN.can_msg_id);
        }
        else if (ktomeCAN.can_msg[0] == 't' && gamemode == 3)
        {
            Serial.println("Module wants the time!");
            send_time = true;
        }
    }
}

void BLESend(String msg_data)
{
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
