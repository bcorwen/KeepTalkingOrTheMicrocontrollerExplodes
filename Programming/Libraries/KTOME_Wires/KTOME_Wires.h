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
        // Standard
        byte strike_count;
        bool outbox_waiting;
        String outbox_msg;
        bool serial_vowels;
        bool serial_odd;
        byte battery_number;
        bool ind_car;
        bool ind_frk;
        bool port_s;
        bool port_p;
        byte logicCheck(byte wire_cut);
        // byte logicCheck(byte wire_cut);

        // Specific
        byte wire_colour[6];
        bool wire_interacted[6];
        bool wire_state[6];
        byte wire_numbers;
        byte correct_wire;
        String wire_colours[6] = {"Blank", "Blue", "White", "Yellow", "Red", "Black"};
        Switch switches[6];
        byte pin_array[6] = {WIRES_1, WIRES_2, WIRES_3, WIRES_4, WIRES_5, WIRES_6};
        FLedPWM *fleds_addr;
        byte hasWireBeenCut();
        void findSolution(); // Call after receiving widget info to be able to determine the correct input
		
	public:
        // Standard
        bool game_running;
        bool module_solved;
		byte intPin = -1; // No interrupt pin needed;
        void start(FLedPWM *fleds); // Initialise Wires object
        void powerOn();
        void generate(); // Generate a game (i.e. keypad_setup)
        void reset();
        bool isManual();
        byte manualConfirm();
        byte timerUpdate(String timer_digits);
        void widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels);
        void strikeUpdate(byte strikes);
        String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
        void gameStart();
        byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
        bool isSolved();
        void explode();
        void defuse();
        String outbox();
        bool isOutbox();
        byte update();
        bool needsIsr();
        void isrHandler();

        // Specific
        KTOME_Wires();
		
};

#endif