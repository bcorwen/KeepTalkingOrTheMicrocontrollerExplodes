#ifndef KTOME_SIMON_H
#define KTOME_SIMON_H

#include <Arduino.h>
#include <KTOME_common.h>

#define SIMON_S_R   GPIO_NUM_33
#define SIMON_S_B   GPIO_NUM_32
#define SIMON_S_G   GPIO_NUM_27
#define SIMON_S_Y   GPIO_NUM_26
#define SIMON_L_R   GPIO_NUM_23
#define SIMON_L_B   GPIO_NUM_18
#define SIMON_L_G   GPIO_NUM_16
#define SIMON_L_Y   GPIO_NUM_17

class KTOME_Simon {
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
        byte logicCheck(byte button_number); //byte logicCheck(byte button_number);

        // Specific
		byte stages; // The number of lights in the game
        byte stage; // How many lights are being displayed? How far through the game are we?
        byte step; // Which button is currently expected in the sequence? This resets after a while, when the lights demo
        byte sequence_lights[5]; // The sequence of lights
        byte button_to_light[4]; // The number of the button to press given the light. E.g. First element = 2 means a Red light requires a Green to be pressed.
        byte disp_step; // Tracks the current part of the sequence which is being lit
        bool user_interrupt;
        uint32_t light_timing; // Tracks the time for the next light to be displayed
        uint32_t button_timing; // Tracks time of last button press, for module to restart flashing sequence 
        bool user_interacted; // Track whether this module is making noise yet
        LedPWM leds[4];
        Switch switches[4];
        const byte pin_array[8] = {SIMON_L_R, SIMON_L_B, SIMON_L_G, SIMON_L_Y, SIMON_S_R, SIMON_S_B, SIMON_S_G, SIMON_S_Y};
        const String light_colours[4] = {"Red", "Blue", "Green", "Yellow"};
        const uint16_t led_on_time = 500; // Time LED will light up
        const uint16_t led_off_time = 300; // Time between LEDs lighting
        const uint32_t strike_time = 4100; // Time between LED sequence repeats
        const uint32_t advance_time = 1600; // Time between finishing previous input and displaying next stage
        const uint32_t reset_time = 4750; // Time between last user input and LEDs repeating sequence
        const byte translation[3][2][4] = {{{1, 0, 3, 2}, {1, 3, 2, 0}},  // 0 strikes - Vowel | no vowel
                                        {{3, 2, 1, 0}, {0, 1, 3, 2}},  // 1 strike
                                        {{2, 0, 3, 1}, {2, 3, 1, 0}}}; // 2 strikes
        bool hasButtonBeenPushed(byte button_number);
        void lightsOut();
        void findSolution();
		
	public:
        // Standard
        bool game_running;
        bool module_solved;
		byte intPin = -1; // No interrupt pin needed;
        void start(FLedPWM *fleds); // Initialise Keypad object
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
        KTOME_Simon();
		
};

#endif