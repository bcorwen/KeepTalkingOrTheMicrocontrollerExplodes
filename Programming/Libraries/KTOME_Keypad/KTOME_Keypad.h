#ifndef KTOME_KEYPAD_H
#define KTOME_KEYPAD_H

#include <Arduino.h>
#include <KTOME_common.h>

#define KEYPAD_B1   GPIO_NUM_19
#define KEYPAD_B2   GPIO_NUM_18
#define KEYPAD_B3   GPIO_NUM_5
#define KEYPAD_B4   GPIO_NUM_17

class KTOME_Keypad {
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
        byte logicCheck(byte button_number);

        // Specific
		byte stage;
        byte selected_symbols[4];
        byte column_symbols[7];
        byte order_symbols[4];
        FLedPWM *fleds_addr;
        Switch switches[4];
        byte pin_array[4] = {KEYPAD_B1, KEYPAD_B2, KEYPAD_B3, KEYPAD_B4};
        byte keypadPicker(byte col_select);
        bool hasButtonBeenPushed(byte button_number);
		
	public:
        // Standard
        bool game_running;
        bool module_solved;
		byte intPin = -1; // No interrupt pin needed;
        void start(FLedPWM *fleds); // Initialise Keypads object
        void powerOn();
        void generate(); // Generate a game (i.e. button_setup)
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
        byte getStage(); // IS THIS NEEDED?
        KTOME_Keypad();
};

#endif