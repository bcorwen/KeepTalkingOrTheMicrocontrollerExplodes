#ifndef KTOME_MORSE_H
#define KTOME_MORSE_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>

#define MORSE_LED   GPIO_NUM_19
#define MORSE_ENA   GPIO_NUM_5
#define MORSE_ENB   GPIO_NUM_17
#define MORSE_ENS   GPIO_NUM_16
#define MORSE_OLED_CLK  GPIO_NUM_14
#define MORSE_OLED_DAT  GPIO_NUM_13
#define MORSE_OLED_CS   GPIO_NUM_23
#define MORSE_OLED_DC   GPIO_NUM_22
#define MORSE_OLED_RST  GPIO_NUM_21

class KTOME_Morse {
	private:
		Switch switches;
        LedBlinkable leds;
        ESP32Encoder encoder;
        byte pin_array[4] = {MORSE_LED, MORSE_ENA, MORSE_ENB, MORSE_ENS};
        String word_list[16] = {"shell", "halls", "slick", "trick", "boxes", "leaks", "strobe", "bistro",
                                "flick", "bombs", "break", "brick", "steak", "sting", "vector", "beats"};
        byte freq_list[16] = {5, 15, 22, 32, 35, 42, 45, 52, 55, 65, 72, 75, 82, 92, 95, 100};
        uint32_t morse_timings [4] = {283, 817, 1083, 1627}; // dot, dah, letter, word-letter
        byte selected_word;
        byte morse_letter; // Tracks the letter being displayed;
        byte morse_dot_dah; // Tracks the part of the letter being displayed;
        int32_t morse_timer;
        int32_t encoder_tracker;
        byte freq_choice;
        bool module_defused;

        bool hasButtonBeenPushed();
        bool logicCheck();
        void blinkLetter();
    
    protected:
    // U8G2_ST7920_128X64_1_SW_SPI u8g2;

	public:
        void start(); // Initialise Keypad object
        void generate(); // Generate a game (i.e. keypad_setup)
        void reset();
        byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
        void drawScreen();
        void clearScreen();
        void update();
        void gameStart();
        bool isDefused();
        void defused();
        void explode();
        KTOME_Morse();
		
};

// extern U8G2_ST7920_128X64_1_SW_SPI u8g2;
extern U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2;

#endif