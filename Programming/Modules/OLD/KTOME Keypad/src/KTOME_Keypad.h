#ifndef KTOME_KEYPAD_H
#define KTOME_KEYPAD_H

#include <Arduino.h>
#include <KTOME_common.h>

#define KEYPAD_B1   GPIO_NUM_15
#define KEYPAD_B2   GPIO_NUM_2
#define KEYPAD_B3   GPIO_NUM_0
#define KEYPAD_B4   GPIO_NUM_4
#define KEYPAD_LR4   GPIO_NUM_16
#define KEYPAD_LR3   GPIO_NUM_17
#define KEYPAD_LR2   GPIO_NUM_5
#define KEYPAD_LR1   GPIO_NUM_18
#define KEYPAD_LG1   GPIO_NUM_19
#define KEYPAD_LG2   GPIO_NUM_21
#define KEYPAD_LG3   GPIO_NUM_22
#define KEYPAD_LG4   GPIO_NUM_23

class KTOME_Keypad {
	private:
		byte stage;
    byte selected_symbols[4];
    byte column_symbols[7];
    byte order_symbols[4];
    Led leds_green[4];
    LedBlinkable leds_red[4];
    Switch switches[4];
    byte pin_array[12] = {KEYPAD_LR1, KEYPAD_LR2, KEYPAD_LR3, KEYPAD_LR4, KEYPAD_LG1, KEYPAD_LG2, KEYPAD_LG3, KEYPAD_LG4, KEYPAD_B1, KEYPAD_B2, KEYPAD_B3, KEYPAD_B4};
    byte keypadPicker(byte col_select);
    bool hasButtonBeenPushed(byte button_number);
    byte logicCheck(byte button_number);
		
	public:
    void start(); // Initialise Keypad object
    void generate(); // Generate a game (i.e. keypad_setup)
    void reset();
		String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
    byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
    byte getStage();
    bool isDefused();
    void explode();
    void update();
    KTOME_Keypad();
		
};

#endif