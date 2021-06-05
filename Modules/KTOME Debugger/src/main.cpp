//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 28/03/21
//======================================================================
//
//  Module: Debugger (MASTER)
//
//  version 0.6.0
//
//  
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <BLE2902.h>
#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>
#include <KTOME_common.h>
#include <LiquidCrystal.h>
#include <ESP32Encoder.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************

#define PIN_LED       GPIO_NUM_2
// #define PIN_STRIKE_1  GPIO_NUM_18
// #define PIN_STRIKE_2  GPIO_NUM_19
#define PIN_ENCODER_1 GPIO_NUM_32
#define PIN_ENCODER_2 GPIO_NUM_33
#define PIN_ENCODER_B GPIO_NUM_25
#define PIN_LCD_RS GPIO_NUM_23
#define PIN_LCD_E GPIO_NUM_22
#define PIN_LCD_D0 GPIO_NUM_21
#define PIN_LCD_D1 GPIO_NUM_19
#define PIN_LCD_D2 GPIO_NUM_18
#define PIN_LCD_D3 GPIO_NUM_5

// Game
byte gamemode = 0;
bool game_ready = false;
bool holding = false;
bool manual_check = false;
bool game_running = false;
bool game_win = false;

bool module_detected = false;
bool module_inited = false;

bool send_time = false;

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
byte time_scale; // Quadruple the time scale: 4 = 1x speed (normal), 5 = 1.25x speed (1 strike), etc...

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
#define CAN_ID            can_ids.Master
#define CAN_MASK          can_ids.Master

char CAN_message[9];

//Other
ESP32Encoder encoder;
uint32_t encoderlastToggled;
Switch encoder_press;

LiquidCrystal myLCD(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3);

int8_t menu_selected = 0;
const byte menu_max = 13;

String menu_choice[menu_max] = {"Poll modules  ",
                                "Init game     ",
                                "Request manual",
                                "Confirm manual",
                                "Game start    ",
                                "Game stop (x) ",
                                "Game stop (o) ",
                                "Send edgework ",
                                "Send serial   ",
                                "Send strikes  ",
                                "Send heartbeat",
                                "Set time: 0238",
                                "Set time: 0451",
                                };

char menu_can_msg[menu_max][9] = {"P",
                                  "I",
                                  "C",
                                  "M",
                                  "A",
                                  "Z1",
                                  "Z0",
                                  "W0000000",
                                  "SXX0XX0",
                                  "X0",
                                  "H",
                                  "T0238",
                                  "T0451",
                                  };

byte menu_can_lng[menu_max] = {1, 1, 1, 1, 1, 2, 2, 8, 7, 2, 1, 5, 5};

char time_to_send[6] = "T0238";

void CANInbox();

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************
void setup() {

  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Debugger ==");

  myLCD.begin(20,4);
  myLCD.clear();
  myLCD.setCursor(0,0);
  myLCD.print("KTOME : Debugger!");
  myLCD.setCursor(0,1);
  myLCD.print("Starting...");

  // Start CAN bus
  ktomeCAN.setId(CAN_ID, CAN_MASK);
  ktomeCAN.start();
  // start the CAN bus at 500 kbps
  if (!ktomeCAN.start()) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.print("My ID is:   0b");
  ktomeCAN.padZeros(CAN_ID);
  Serial.println(CAN_ID, BIN);
  Serial.print("My mask is: 0b");
  ktomeCAN.padZeros(CAN_MASK);
  Serial.println(CAN_MASK, BIN);

  // Randomiser
  esp_random();

  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.setFilter(1024);
  encoder.attachHalfQuad(PIN_ENCODER_2, PIN_ENCODER_1);
  encoder.setCount(0);
	// Serial.println("Encoder Start = " + String((int32_t)encoder.getCount()));
	// set the lastToggle
	encoderlastToggled = millis();
  encoder_press.init(PIN_ENCODER_B);

  myLCD.clear();
  myLCD.setCursor(0,0);
  myLCD.print(menu_choice[menu_selected]);
  myLCD.setCursor(0,1);
  myLCD.print("> \"        \"");

}

void loop() {

  int32_t encoder_reading = encoder.getCount();

  if (encoder_reading != 0) {
    // Serial.print("Encoder count = " + String(encoder_reading));
    menu_selected += encoder_reading;
    if (menu_selected < 0) {
      menu_selected = menu_max-abs(menu_selected%menu_max);
    } else if (menu_selected >= menu_max) {
      menu_selected = abs(menu_selected%menu_max);
    }
    encoder.clearCount();
    myLCD.setCursor(0,0);
    myLCD.print(menu_choice[menu_selected]);

    // Serial.println(" | Menu option = " + String(menu_selected));
  }
  
  if (encoder_press.hasChanged()) {
    if (encoder_press.isPressed()) {
      // Serial.println("Encoder press");
      if (menu_selected < 11) {
        digitalWrite(PIN_LED, HIGH);
        ktomeCAN.send((can_ids.All_modules), menu_can_msg[menu_selected], menu_can_lng[menu_selected]);
        digitalWrite(PIN_LED, LOW);
      } else if (menu_selected == 11) {
        time_to_send[2] = '2';
        time_to_send[3] = '3';
        time_to_send[4] = '8';
      } else {
        time_to_send[2] = '4';
        time_to_send[3] = '5';
        time_to_send[4] = '1';
      }
    }
  }

  bool message_waiting = false;
  message_waiting = ktomeCAN.isMessageWaiting();
  if (message_waiting) {
    do { // There are outstanding messages
      CANInbox();
      // uint32_t module_array = (ktomeCAN.can_msg_id & (CAN_ALL_MOD | CAN_MUID_ALL));
      // Serial.print("Module id: 0b");
      // ktomeCAN.padZeros(module_array);
      // Serial.print(module_array, BIN);
      // Serial.print(" | Message: \"");
      // Serial.print(ktomeCAN.can_msg);
      // Serial.println("\"");
      if (send_time) {
        digitalWrite(PIN_LED, HIGH);
        ktomeCAN.send((can_ids.All_modules), time_to_send, 5);
        digitalWrite(PIN_LED, LOW);
        send_time = false;
      }
      module_detected = false;
      myLCD.setCursor(0,1);
      myLCD.print("> \"        \"");
      myLCD.setCursor(3, 1);
      myLCD.print(ktomeCAN.can_msg);
      message_waiting = ktomeCAN.isMessageWaiting();
    } while (message_waiting);
  }
  
}

// //**********************************************************************
// // FUNCTIONS: Communications
// //**********************************************************************

void CANInbox() {
  if (ktomeCAN.isMessageWaiting()) { // Outstanding messages to handle
    ktomeCAN.receive();
    // if (ktomeCAN.can_msg[0] == 'p' && gamemode == 1) {
    //   module_detected = true;
    //   Serial.println("Module detected!");
    // }
    // else if (ktomeCAN.can_msg[0] == 'i' && gamemode == 2) {
    //   module_inited = true;
    //   Serial.println("Module has declared it is setup!");
    // }
    // else if (ktomeCAN.can_msg[0] == 'c' && gamemode == 2) {
    //   Serial.println("Module is transmitting it's manual setup needs!");
    // }
    // else if (ktomeCAN.can_msg[0] == 'x' && gamemode == 3) {
    //   Serial.println("Module announces a strike!");
    //   strike_flag = true;
    //   strike_culprit = ktomeCAN.can_msg_id;
    // }
    // else if (ktomeCAN.can_msg[0] == 'd' && gamemode == 3) {
    //   Serial.println("Module announces its defusal!");
    // }
    if (ktomeCAN.can_msg[0] == 't') {
      Serial.println("Module wants the time!");
      send_time = true;
    }

  }
}

// void BLESend(String msg_data) {
//   String BLE_output(msg_data);
//   Serial.print("Sending \"");
//   Serial.print(msg_data);
//   Serial.println("\" to phone app...");
//   pTxCharacteristic->setValue(BLE_output.c_str());
//   pTxCharacteristic->notify();
// }

// //**********************************************************************
// // FUNCTIONS: Misc. Functions
// //**********************************************************************
