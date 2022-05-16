#ifndef KTOME_SIMON_H
#define KTOME_SIMON_H

#include <Arduino.h>
#include <KTOME_common.h>

#define SIMON_S_R   GPIO_NUM_12
#define SIMON_S_B   GPIO_NUM_2
#define SIMON_S_G   GPIO_NUM_0
#define SIMON_S_Y   GPIO_NUM_4
#define SIMON_L_R   GPIO_NUM_16
#define SIMON_L_B   GPIO_NUM_17
#define SIMON_L_G   GPIO_NUM_5
#define SIMON_L_Y   GPIO_NUM_18

class KTOME_Simon {
	private:
		byte stages; // The number of lights in the game
        byte stage; // How many lights are being displayed? How far through the game are we?
        byte step; // Which button is currently expected in the sequence? This resets after a while, when the lights demo
        byte sequence_lights[5]; // The sequence of lights
        byte button_to_light[4]; // The number of the button to press given the light. E.g. First element = 2 means a Red light requires a Green to be pressed.
        byte disp_step; // Tracks the current part of the sequence which is being lit
        bool user_interrupt;
        uint32_t light_timing; // Tracks the time for the next light to be displayed
        uint32_t button_timing; // Tracks time of last button press, for module to restart flashing sequence 
        bool serial_vowel;
        byte strike_count;
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
        void findSolution();
        bool hasButtonBeenPushed(byte button_number);
        byte logicCheck(byte button_number);
        void lightsOut();
		
	public:
        bool game_running;
        void start(); // Initialise Keypad object
        void generate(); // Generate a game (i.e. keypad_setup)
        void reset();
        void vowelSolution(bool serial_vowel);
        void strikeSolution(byte strike_count);
        byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
        byte getStage();
        bool isDefused();
        void explode();
        byte update();
        KTOME_Simon();
		
};

#endif