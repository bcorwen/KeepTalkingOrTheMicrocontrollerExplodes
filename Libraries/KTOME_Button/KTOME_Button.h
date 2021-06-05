#ifndef KTOME_BUTTON_H
#define KTOME_BUTTON_H

#include <Arduino.h>
#include <KTOME_common.h>

#define BUTTON_LED_R   GPIO_NUM_21
#define BUTTON_LED_G   GPIO_NUM_22
#define BUTTON_LED_B   GPIO_NUM_23
#define BUTTON_SWT   GPIO_NUM_0

class KTOME_Button {
	private:
    bool module_defused;
		bool shortpushneeded;
    bool isthisapress;
    bool strip_on;
    bool wait_for_timer;
    byte button_colour;
    byte button_label;
    byte strip_colour;
    String button_labels[4] = {"Abort", "Detonate", "Hold", "Press"};
    String button_colours[4] = {"Blue", "White", "Yellow", "Red"};
    LedPWM leds_button[3];
    Switch switches;
    byte pin_array[4] = {BUTTON_LED_R, BUTTON_LED_G, BUTTON_LED_B, BUTTON_SWT};
    bool hasButtonBeenReleased();
    byte logicCheck();
    void stripOn(bool strip_power);
		
	public:
    void start(); // Initialise Keypad object
    void generate(); // Generate a game (i.e. keypad_setup)
    void reset();
    byte timerDigits(String timer_digits);
    void findSolution(bool ind_frk, bool ind_car, byte bat_num); // Call after receiving widget info to be able to determine the correct input
	String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
    byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
    bool isDefused();
    void explode();
    void update();
    KTOME_Button();
		
};

#endif