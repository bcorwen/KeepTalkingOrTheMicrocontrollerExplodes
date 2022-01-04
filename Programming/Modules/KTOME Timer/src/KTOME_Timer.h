#ifndef KTOME_TIMER_H
#define KTOME_TIMER_H

#include <KTOME_common.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#define STRIKE_L_1	GPIO_NUM_18
#define STRIKE_L_2	GPIO_NUM_19

class KTOME_Timer {
	private:
		int32_t time_left = 300000000;
		int32_t time_length = 300000000;
		int32_t this_macros;
		String timer_display = "----";
		String timer_display_prev = "----";
        byte time_scale;    // Quadruple the time scale: 4 = 1x speed (normal), 5 = 1.25x speed (1 strike), etc...
        int32_t blink_timer;
        int32_t blink_start;
        void convertTimeToStr();
        void blinkTime(bool game_won);
		void drawDisplay();
		bool draw_colon;
		Adafruit_7segment timerdisp;

        bool hardcore_mode = false;
		byte strike_count;
        byte strike_limit = 3;
		LedBlinkable leds[2];
		const byte pin_array[2] = {STRIKE_L_1, STRIKE_L_2};
        void assignStrikes();
		
	public:
        const int32_t us_conv = 1000000;
        const int32_t ms_conv = 1000;
        void setGameLength(int32_t game_length);
		void updateTimer(byte gamemode, bool game_win);
		bool hasSecondTickedOver();
		int32_t getTimeLeft();
        String getTimeStr();
        bool hasTimerExpired();

        void setHardcore(bool hardcore_mode);
		void setStrikes(byte strike_count);
        void addStrikes(byte strike_add);
		byte getStrikes();
        void updateStrikes();
        bool reachedStrikeLimit();

        void init();
		void reset();
        void gameStart();
        void explode();
        void defuse();
		KTOME_Timer();
};

#endif