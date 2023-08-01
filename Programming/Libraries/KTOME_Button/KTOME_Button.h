#ifndef KTOME_BUTTON_H
#define KTOME_BUTTON_H

#include <Arduino.h>
#include <KTOME_common.h>

#define BUTTON_SWT   GPIO_NUM_33

class KTOME_Button {
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
        byte logicCheck();

        // Specific
        bool shortpushneeded;
        bool isthisapress;
        bool strip_on;
        bool wait_for_timer;
        byte button_colour;
        byte button_label;
        byte strip_colour;
        String button_labels[4] = {"Abort", "Detonate", "Hold", "Press"};
        String button_colours[4] = {"Blue", "White", "Yellow", "Red"};
        CRGB button_colour_hi[4] = {CRGB::Blue, CRGB::White, CRGB::Yellow, CRGB::Red};
        CRGB button_colour_lo[4] = {0x000055, 0x555555, 0x555500, 0x550000};
        FLedPWM *fleds_addr;
        Switch switches;
        byte pin_array = BUTTON_SWT;
        bool hasButtonBeenReleased();
        void stripOn();
        void findSolution(); // Call after receiving widget info to be able to determine the correct input
		
	public:
        // Standard
        bool game_running;
        bool module_solved;
        byte intPin = -1; // No interrupt pin needed;
        void start(FLedPWM *fleds); // Initialise Button object
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
        KTOME_Button();
		
};

#endif
