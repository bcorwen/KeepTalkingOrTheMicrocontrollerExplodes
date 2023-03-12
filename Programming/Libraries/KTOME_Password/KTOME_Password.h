#ifndef KTOME_PASSWORD_H
#define KTOME_PASSWORD_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
// #include <Keypad.h>

// #define PASSWORD_B1U   GPIO_NUM_12
// #define PASSWORD_B1D   GPIO_NUM_4
// #define PASSWORD_B2U   GPIO_NUM_14
// #define PASSWORD_B2D   GPIO_NUM_16
// #define PASSWORD_B3U   GPIO_NUM_25
// #define PASSWORD_B3D   GPIO_NUM_17
// #define PASSWORD_B4U   GPIO_NUM_33
// #define PASSWORD_B4D   GPIO_NUM_5
// #define PASSWORD_B5U   GPIO_NUM_32
// #define PASSWORD_B5D   GPIO_NUM_18
// #define PASSWORD_SUB   GPIO_NUM_15
// #define PASSWORD_BKL   GPIO_NUM_13
// #define PASSWORD_GLCD_C GPIO_NUM_19
// #define PASSWORD_GLCD_D GPIO_NUM_21
// #define PASSWORD_GLCD_L GPIO_NUM_22
// #define PASSWORD_GLCD_R GPIO_NUM_36

#define GLCD_DATA       23
#define GLCD_CLOCK      18
#define GLCD_LATCH      15
#define GLCD_RESET      16
#define GPIO_EXP_SDA    21
#define GPIO_EXP_SCL    22

class KTOME_Password {
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

        // Keypad kpd;
        // static const byte ROWS = 3;
        // static const byte COLS = 4;
        // char keyMatrix[ROWS][COLS] = {
        //     {'A','a','B','b'},
        //     {'C','c','D','d'},
        //     {'E','e','!','?'}
        // };
        // byte rowPins[ROWS] = {25, 33, 32};
        // byte colPins[COLS] = {4, 16, 17, 5};
		Switch switches[11];
        Led leds; // GLCD backlight
        // byte pin_array[12] = {PASSWORD_B1U, PASSWORD_B1D, PASSWORD_B2U, PASSWORD_B2D, PASSWORD_B3U, PASSWORD_B3D, PASSWORD_B4U, PASSWORD_B4D, PASSWORD_B5U, PASSWORD_B5D, PASSWORD_SUB, PASSWORD_BKL};
        String word_list[35] = {"ABOUT", "AFTER", "AGAIN", "BELOW", "COULD",
                                "EVERY", "FIRST", "FOUND", "GREAT", "HOUSE",
                                "LARGE", "LEARN", "NEVER", "OTHER", "PLACE",
                                "PLANT", "POINT", "RIGHT", "SMALL", "SOUND",
                                "SPELL", "STILL", "STUDY", "THEIR", "THERE",
                                "THESE", "THING", "THINK", "THREE", "WATER",
                                "WHERE", "WHICH", "WORLD", "WOULD", "WRITE"};
        char alphabet[26];
        char letter_reel[5][6];
        bool letter_pool[5][26];
        byte letter_left[5];
        byte selected_word;
        byte displayed_letters[5];
        // bool module_defused;
        void removeOtherWords(byte word_to_remove);
        void allocateSlots();
        bool hasButtonBeenPushed(byte button_number);
        void upDownButton(byte button_number);
        byte submitButton();
        // bool logicCheck();
        void drawLetter(char letter_to_draw, byte position_on_screen);
        void drawBox(byte left_offset, byte left_edge, byte top_edge, byte px_width, byte px_height);

	public:
        // Standard
        bool game_running;
        bool module_solved;
        void start(FLedPWM *fleds); // Initialise Button object
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

        // Specific
        KTOME_Password();
		
};

extern U8G2_ST7920_128X64_1_SW_SPI u8g2;
// extern Keypad kpd;

#endif