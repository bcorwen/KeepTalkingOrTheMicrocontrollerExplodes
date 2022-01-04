#ifndef KTOME_WIRES_H
#define KTOME_WIRES_H

#include <Arduino.h>
#include <KTOME_common.h>

#define WIRES_1     GPIO_NUM_19
#define WIRES_2     GPIO_NUM_18
#define WIRES_3     GPIO_NUM_5
#define WIRES_4     GPIO_NUM_17
#define WIRES_5     GPIO_NUM_16
#define WIRES_6     GPIO_NUM_4

class KTOME_Wires {
	private:
    bool module_defused;
    byte wire_colour[6];
    bool wire_interacted[6];
    bool wire_state[6];
    byte wire_numbers;
    byte correct_wire;
    String wire_colours[6] = {"Blank", "Blue", "White", "Yellow", "Red", "Black"};
    Switch switches[6];
    byte pin_array[6] = {WIRES_1, WIRES_2, WIRES_3, WIRES_4, WIRES_5, WIRES_6};
    byte hasWireBeenCut();
    byte logicCheck(byte wire_cut);
		
	public:
    void start(); // Initialise Keypad object
    void generate(); // Generate a game (i.e. keypad_setup)
    void reset();
    byte timerDigits(String timer_digits);
    void findSolution(bool serial_odd); // Call after receiving widget info to be able to determine the correct input
	String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
    bool manualConfirm();
    byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
    bool isDefused();
    void explode();
    // void update();
    KTOME_Wires();
		
};

#endif