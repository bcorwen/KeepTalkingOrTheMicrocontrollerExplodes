//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 03/06/21
//======================================================================
//
//  Module: Generic Solvable module (Slave, Standard, Vanilla)
//  version 0.8.0
//
//======================================================================

#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>

#define MODULE_TIMER        1
#define MODULE_WIDGETS      2
#define MODULE_WIRES        3
#define MODULE_BUTTON       4
#define MODULE_KEYPAD       5
#define MODULE_SIMON        6
#define MODULE_WHOS         7
#define MODULE_MEMORY       8
#define MODULE_MORSE        9
#define MODULE_CWIRES       10
#define MODULE_WIRESEQ      11
#define MODULE_MAZE         12
#define MODULE_PASSWORD     13
#define MODULE_VENT         14
#define MODULE_CAPACITOR    15
#define MODULE_KNOB         16

#include <config.h>
#include <KTOME_CAN.h>

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
// Module-specific definitions and includes
//**********************************************************************

// #ifdef MODULE_WIRES
#if MODULE_TYPE == MODULE_WIRES
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Wires
    #define FLED_LENGTH 1
    #include <KTOME_Wires.h>
    KTOME_Wires module;
    String module_name = "=== KTOME: WIRES ===";
#elif MODULE_TYPE == MODULE_BUTTON
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Button
    #define FLED_LENGTH 2
    #include <KTOME_Button.h>
    KTOME_Button module;
    String module_name = "=== KTOME: BUTTON ===";
#elif MODULE_TYPE == MODULE_KEYPAD
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Keypad
    #define FLED_LENGTH 10
    #include <KTOME_Keypad.h>
    KTOME_Keypad module;
    String module_name = "=== KTOME: KEYPADS ===";
#elif MODULE_TYPE == MODULE_SIMON
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Simon
    #define fled_length 1
    #include <KTOME_Simon.h>
    KTOME_Simon module;
    String module_name = "=== KTOME: SIMON ===";
#elif MODULE_TYPE == MODULE_WHOS
    // #include <KTOME_Whos.h>
    KTOME_Whos module;
#elif MODULE_TYPE == MODULE_MEMORY
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Memory
    #define fled_length 5
    #include <KTOME_Memory.h>
    KTOME_Memory module;
    String module_name = "=== KTOME: MEMORY ===";
#elif MODULE_TYPE == MODULE_MORSE
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Morse
    #define fled_length 2
    #include <KTOME_Morse.h>
    KTOME_Morse module;
    String module_name = "=== KTOME: MORSE ===";
#elif MODULE_TYPE == MODULE_CWIRES
    // #include <KTOME_CWires.h>
    KTOME_CWires module;
#elif MODULE_TYPE == MODULE_WIRESEQ
    // #include <KTOME_WireSeq.h>
    KTOME_WireSeq module;
#elif MODULE_TYPE == MODULE_MAZE
    // #include <KTOME_Maze.h>
    KTOME_Maze module;
#elif MODULE_TYPE == MODULE_PASSWORD
    #define CONFIG_CAN_MODULE_TYPE  can_ids.Simon
    #define fled_length 1
    // #include <KTOME_Password.h>
    KTOME_Password module;
    String module_name = "=== KTOME: PASSWORD ===";
#elif MODULE_TYPE == MODULE_VENT
    // #include <KTOME_Vent.h>
    KTOME_Vent module;
#elif MODULE_TYPE == MODULE_CAPACITOR
    // #include <KTOME_Capacitor.h>
    KTOME_Capacitor module;
#elif MODULE_TYPE == MODULE_KNOB
    // #include <KTOME_Knob.h>
    KTOME_Knob module;
#endif

#if MODULE_MUID == 1
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_01
#elif MODULE_MUID == 2
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_02
#elif MODULE_MUID == 3
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_03
#elif MODULE_MUID == 4
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_04
#elif MODULE_MUID == 5
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_05
#elif MODULE_MUID == 6
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_06
#elif MODULE_MUID == 7
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_07
#elif MODULE_MUID == 8
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_08
#elif MODULE_MUID == 9
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_09
#elif MODULE_MUID == 10
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_10
#elif MODULE_MUID == 11
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_11
#elif MODULE_MUID == 12
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_12
#elif MODULE_MUID == 13
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_13
#elif MODULE_MUID == 14
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_14
#elif MODULE_MUID == 15
    #define CONFIG_CAN_MODULE_NUM   can_ids.MUID_15
#endif

#include <KTOME_common.h>

#endif