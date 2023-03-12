#ifndef KTOME_CAPACITOR_H
#define KTOME_CAPACITOR_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <ShiftDisplay.h>
#include <Adafruit_MCP23X17.h>

#define CAP_LED     GPIO_NUM_13
#define CAP_SR_L    GPIO_NUM_33
#define CAP_SR_C    GPIO_NUM_32
#define CAP_SR_D    GPIO_NUM_25
#define CAP_POT     GPIO_NUM_35 // Only if using rotary potentiometer instead of microswitch
#define CAP_SWT     GPIO_NUM_5  // Only if using microswitch instead of rotary potentiometer

class KTOME_CapDis {
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
		CRGB leds[1];
        FLedPWM fled;
        Switch switches; // Only if using microswitch instead of rotary potentiometer
        byte pin_array = CAP_SWT; // Only if using microswitch instead of rotary potentiometer
        byte led_array[10] = {0,1,2,3,4,5,6,7,8,9};
        int8_t brightnesslut[128] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 15, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34, 36, 37, 39, 40, 42, 44, 46, 47, 49, 52, 54, 56, 58, 61, 63, 66, 69, 72, 75, 78, 81, 84, 88, 92, 96, 100, 104, 108, 113, 117, 122, 127};
        int32_t needy_timer;
        int8_t needy_active; // 0 = Inert, 1 = Dormant, 2 = Awake
        bool module_defused;
        int8_t display_timer;
        int16_t lever_now;
        int16_t lever_last;
        int32_t this_millis;
        int32_t needy_timer_ms;
        bool lever_down;
        bool cap_popped;
        bool accept_input;
        void sleep();

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
        KTOME_CapDis();
		
};

extern ShiftDisplay shift_timer;
// extern Adafruit_MCP23X17 mcp;

#endif