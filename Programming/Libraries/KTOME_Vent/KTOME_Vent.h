#ifndef KTOME_VENT_H
#define KTOME_VENT_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <ShiftDisplay.h>

#define VENT_SWY   GPIO_NUM_17
#define VENT_SWN   GPIO_NUM_16
//#define VENT_OLED_CLK  GPIO_NUM_14
//#define VENT_OLED_DAT  GPIO_NUM_13
#define VENT_OLED_CS   GPIO_NUM_5 //GPIO_NUM_23
#define VENT_OLED_DC   GPIO_NUM_13 //GPIO_NUM_22
#define VENT_OLED_RST  GPIO_NUM_21
#define VENT_SR_L      GPIO_NUM_33
#define VENT_SR_C      GPIO_NUM_32
#define VENT_SR_D      GPIO_NUM_25

class KTOME_Vent {
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
		Switch switches[2]; // 0 = Yes, 1 = No
        int32_t needy_timer;
        int32_t msg_timer;
        byte msg_stage; // 0 = none, 1 - 3 = YES correct, 4 - 5 = NO incorrect
        byte button_pressed; // 0 = Yes, 1 = No
        byte needy_active; // 0 = Inert, 1 = Dormant, 2 = Awake
        byte pin_array[2] = {VENT_SWY, VENT_SWN};
        String word_list[9] = {"VENT GAS?", "DETONATE?", "Y/N", "YES", "NO", "VENTING", "PREVENTS", "EXPLOSIONS", "COMPLETE"};
        String display_line[3];
        byte prompt_type; // 0 = Vent gas, 1 = Detonate
        bool module_defused;
        int8_t display_timer;
        bool accept_input;
        byte sfx_alert;
        byte sfx_wake;
        bool hasButtonBeenPushed(byte button_number);
        byte stageChange();
        void sleep();
        void drawScreen();
        void clearScreen();
        // byte sendSound();

	public:
        // Standard
        bool game_running;
        bool module_solved;
        byte intPin = -1; // No interrupt pin needed;
        void start(); // Initialise Button object
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
        bool isActive();
        bool isAwake();
        void activate();

        // Specific
        KTOME_Vent();
		
};

// extern U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2;
extern U8G2_SSD1309_128X64_NONAME0_1_4W_HW_SPI u8g2;
extern ShiftDisplay shift_timer;

#endif