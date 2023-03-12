#ifndef KTOME_MORSE_H
#define KTOME_MORSE_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <AccelStepper.h>

// #define MORSE_LED   GPIO_NUM_19
#define MORSE_ENA   GPIO_NUM_15 // Dt
#define MORSE_ENB   GPIO_NUM_4  // Clk
#define MORSE_ENS   GPIO_NUM_12
#define MORSE_OLED_CLK  GPIO_NUM_18
#define MORSE_OLED_DAT  GPIO_NUM_23
#define MORSE_OLED_CS   GPIO_NUM_5
#define MORSE_OLED_DC   GPIO_NUM_17
#define MORSE_OLED_RST  GPIO_NUM_16
#define STEPPER_A       GPIO_NUM_21 // A-1A - blue
#define STEPPER_a       GPIO_NUM_22 // A-1B - black
#define STEPPER_B       GPIO_NUM_32 // B-1A - orange
#define STEPPER_b       GPIO_NUM_33 // B-1B - yellow
#define STEPPER_CALIBR  GPIO_NUM_27

class KTOME_Morse {
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
		Switch switches[2];
        ESP32Encoder encoder;
        // LedBlinkable leds;
        FLedPWM *fleds_addr;
        // byte pin_array[4] = {MORSE_LED, MORSE_ENA, MORSE_ENB, MORSE_ENS};
        String word_list[16] = {"shell", "halls", "slick", "trick", "boxes", "leaks", "strobe", "bistro",
                                "flick", "bombs", "break", "brick", "steak", "sting", "vector", "beats"};
        byte freq_list[16] = {5, 15, 22, 32, 35, 42, 45, 52, 55, 65, 72, 75, 82, 92, 95, 100};
        uint32_t morse_timings [4] = {283, 817, 1083, 1627}; // dot, dah, letter, word
        byte selected_word;
        byte morse_letter; // Tracks the letter being displayed;
        byte morse_dot_dah; // Tracks the part of the letter being displayed;
        int32_t morse_timer;
        int32_t encoder_tracker;
        byte freq_choice;
        bool stepper_calibrated = false;
        int16_t stepperSpeed = 600;
        int16_t stepperSpeedMax = 2000;
        void calibrateStepper();
        void drawScreen();
        void clearScreen();
        bool hasButtonBeenPushed();
        void blinkLetter();
    
    protected:
    // U8G2_ST7920_128X64_1_SW_SPI u8g2;

	public:
        // Standard
        bool game_running;
        bool module_solved;
        byte intPin = -1; // No interrupt pin needed;
        void start(FLedPWM *fleds); // Initialise Button object
        void powerOn();
        void generate(); // Generate a game
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
        KTOME_Morse();
		
};

extern U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2;
extern AccelStepper stepper;

#endif