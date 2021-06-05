#ifndef KTOME_COMMON_H
#define KTOME_COMMON_H

#include <Arduino.h>

// class Led_category {
// 	public:
// 		static const byte module_red		=	1;
// 		static const byte module_grn		=	2;
// 		static const byte other				=	0;
// };

class Led {
	protected:
		byte pin;
		bool state;
		bool blink_enable;
		byte led_cat;
	public:
		// Led(byte pin, byte led_cat); // Maybe set some more arguments and variables so we can look up if this is a light to flash on explode for example
		Led();
		void init(byte pin);
		void init(byte pin, byte led_cat);
		void write(bool set_state);
		void write(float set_state);
		void write(uint32_t set_state);
		bool read();
		byte getCat();
};

class LedBlinkable : public Led {
	protected:
		uint32_t blink_change_time;
		uint32_t blink_time_on;
		uint32_t blink_time_off;
		int8_t blink_cycles;
		void blinkStart(bool start_state);
		void writeBlink(bool set_state);
	public:
		LedBlinkable();
		void blink(bool start_state, uint32_t blink_time_on);
		void blink(bool start_state, uint32_t blink_time_on, int8_t blink_cycles);
		void blink(bool start_state, uint32_t blink_time_on, uint32_t blink_time_off, int8_t blink_cycles);
		void update();
};

class LedPWM : public LedBlinkable {
	protected:
		uint32_t pwm_freq;
		byte pwm_channel;
		byte pwm_res;
		uint16_t pwm_res_max;
		bool pulse_enable;
		byte pulse_stage; // 0 = low, 1 = rising, 2 = high, 3 = falling
		int16_t duty_cycle;
		uint32_t duty_timer;
		uint32_t duty_low;
		uint32_t duty_high;
		uint32_t rising_time;
		uint32_t falling_time;
		uint32_t high_time;
		uint32_t low_time;
		void writePulse(int32_t duty_cycle);
	public:
		LedPWM();
		void init(byte pin, int32_t pwm_freq, byte pwm_channel, byte pwm_res);
		void init(byte pin, int32_t pwm_freq, byte pwm_channel);
		void setDutyCycle(int32_t duty_cycle);
		void write(bool set_state);
		void write(float set_state);
		void write(uint32_t set_state);
		void setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t rising_time, uint32_t falling_time);
		void setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t rising_time, uint32_t falling_time, bool start_high);
		void setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time);
		void setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time, bool start_high);
        void setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time, bool start_high, int8_t blink_cycles);
		void update();
};

class Switch {
	private:
		byte pin;
		bool state;
		bool state_prev;
		uint32_t debounce_delay;
		uint32_t debounce_timer;
	public:
		// Switch(byte pin);
		Switch();
		// void init();
		void init(byte pin);
		void init(byte pin, int16_t custom_debounce);
		void read();
		bool hasChanged();
		bool isPressed();
		uint32_t lastChange();
};

#endif