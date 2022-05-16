#include <Arduino.h>
#ifndef FastLED
#include <FastLED.h>
#endif
#include <KTOME_common.h>

Led::Led() {
	
}

void Led::init(byte pin) {
	this->pin = pin;
	this->led_cat = 0;
	pinMode(pin, OUTPUT);
	write(false);
}

void Led::init(byte pin, byte led_cat) {
	this->pin = pin;
	this->led_cat = led_cat;
	pinMode(pin, OUTPUT);
	write(false);
}

void Led::write(bool set_state) {
	state = set_state;
	digitalWrite(pin, state);
	blink_enable = false;
}

void Led::write(float set_state) {
	if (set_state < 0.5) {
		state = false;
	} else {
		state = true;
	}
	digitalWrite(pin, state);
	blink_enable = false;
}

void Led::write(uint32_t set_state) {
	if (set_state < 50) {
		state = false;
	} else {
		state = true;
	}
	digitalWrite(pin, state);
	blink_enable = false;
}

bool Led::read() {
	return state;
}

byte Led::getCat() {
	return led_cat;
}

LedBlinkable::LedBlinkable() : Led() {

}

void LedBlinkable::blink(bool start_state, uint32_t blink_time_on) {
	this->blink_time_on = blink_time_on;
	this->blink_time_off = blink_time_on;
	this->blink_cycles = -1;
	this->blink_change_time = millis() + blink_time_on;
	blinkStart(start_state);
}

void LedBlinkable::blink(bool start_state, uint32_t blink_time_on, int8_t blink_cycles) {
	this->blink_time_on = blink_time_on;
	this->blink_time_off = blink_time_on;
	this->blink_cycles = blink_cycles;
	this->blink_change_time = millis() + blink_time_on;
	blinkStart(start_state);
}

void LedBlinkable::blink(bool start_state, uint32_t blink_time_on, uint32_t blink_time_off, int8_t blink_cycles) {
	this->blink_time_on = blink_time_on;
	this->blink_time_off = blink_time_off;
	this->blink_cycles = blink_cycles;
	this->blink_change_time = millis() + blink_time_on;
	blinkStart(start_state);
}

void LedBlinkable::blinkStart(bool start_state) {
	blink_enable = true;
	writeBlink(start_state);
}

void LedBlinkable::update() {
	if (blink_enable) {
		uint32_t thismillis = millis();
		// Serial.print(thismillis);
		// Serial.print(" : ");
		// Serial.println(blink_change_time);
		if (thismillis >= blink_change_time) {
			if (state) {
				blink_change_time += blink_time_off;
				if (blink_cycles > 0) {
					blink_cycles --;
				}
			} else {
				blink_change_time += blink_time_on;
			}
			writeBlink(!state);
		}
	}
	if (blink_cycles == 0) {
		blink_enable = false;
	}
}

void LedBlinkable::writeBlink(bool set_state) {
	state = set_state;
	digitalWrite(pin, state);
	if (state) {
		blink_change_time = millis() + blink_time_on;
	} else {
		blink_change_time = millis() + blink_time_off;
	}
}

LedPWM::LedPWM() {

}

void LedPWM::init(byte pin, int32_t pwm_freq, byte pwm_channel, byte pwm_res) {
	this->pin = pin;
	this->led_cat = 0;
	this->pwm_freq = pwm_freq;
	this->pwm_channel = pwm_channel;
	this->pwm_res = pwm_res;
	pwm_res_max = pow(2, pwm_res)-1;
	ledcSetup(pwm_channel, pwm_freq, pwm_res);
	ledcAttachPin(pin, pwm_channel);
	setDutyCycle(0);
}

void LedPWM::init(byte pin, int32_t pwm_freq, byte pwm_channel) {
	this->pin = pin;
	this->led_cat = 0;
	this->pwm_freq = pwm_freq;
	this->pwm_channel = pwm_channel;
	pwm_res = 8;
	pwm_res_max = 255;
	ledcSetup(pwm_channel, pwm_freq, 8);
	ledcAttachPin(pin, pwm_channel);
	setDutyCycle(0);
}

void LedPWM::setDutyCycle(int32_t duty_cycle) {
	this->duty_cycle = duty_cycle;
	pulse_enable = false;
	blink_enable = false;
	ledcWrite(pwm_channel, duty_cycle);
}

void LedPWM::write(bool set_state) {
	if (set_state == false) {
		duty_cycle = 0;
	} else {
		duty_cycle = pwm_res_max;
	}
	writePulse(0);
	// digitalWrite(pin, state);
	pulse_enable = false;
	blink_enable = false;
}

void LedPWM::write(float set_state) {
	duty_cycle = constrain(set_state,0.0,1.0)*(pwm_res_max);
	writePulse(0);
	// digitalWrite(pin, state);
	pulse_enable = false;
	blink_enable = false;
}

void LedPWM::write(uint32_t set_state) {
	duty_cycle = constrain(set_state,0,100)*(pwm_res_max);
	writePulse(0);
	// digitalWrite(pin, state);
	pulse_enable = false;
	blink_enable = false;
}

void LedPWM::setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t rising_time, uint32_t falling_time) {
	pulse_enable = true;
	blink_enable = false;
	duty_timer = millis();
    blink_cycles = -1;
	pulse_stage = 0;
	this->duty_low = duty_low;
	this->duty_high = duty_high;
	this->rising_time = rising_time;
	this->falling_time = falling_time;
	low_time = 5;
	high_time = 5;
	writePulse(duty_low);
	duty_timer += low_time;
	pulse_stage = 0;
}

void LedPWM::setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t rising_time, uint32_t falling_time, bool start_high){
	pulse_enable = true;
	blink_enable = false;
	duty_timer = millis();
    blink_cycles = -1;
	pulse_stage = 0;
	this->duty_low = duty_low;
	this->duty_high = duty_high;
	this->rising_time = rising_time;
	this->falling_time = falling_time;
	low_time = 5;
	high_time = 5;
	if (start_high){
		writePulse(duty_high);
		duty_timer += high_time;
		pulse_stage = 2;
	} else {
		writePulse(duty_low);
		duty_timer += low_time;
		pulse_stage = 0;
	}
}
void LedPWM::setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time){
	pulse_enable = true;
	blink_enable = false;
	duty_timer = millis();
    blink_cycles = -1;
	pulse_stage = 0;
	this->duty_low = duty_low;
	this->duty_high = duty_high;
	this->rising_time = rising_time;
	this->falling_time = falling_time;
	this->low_time = low_time;
	this->high_time = high_time;
	writePulse(duty_low);
	duty_timer += low_time;
	pulse_stage = 0;
}

void LedPWM::setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time, bool start_high){
	pulse_enable = true;
	blink_enable = false;
	duty_timer = millis();
    blink_cycles = -1;
	pulse_stage = 0;
	this->duty_low = duty_low;
	this->duty_high = duty_high;
	this->rising_time = rising_time;
	this->falling_time = falling_time;
	this->low_time = low_time;
	this->high_time = high_time;
	if (start_high){
		writePulse(duty_high);
		duty_timer += high_time;
		pulse_stage = 2;
	} else {
		writePulse(duty_low);
		duty_timer += low_time;
		pulse_stage = 0;
	}
}

void LedPWM::setPulse(uint32_t duty_high, uint32_t duty_low, uint32_t low_time, uint32_t rising_time, uint32_t high_time, uint32_t falling_time, bool start_high, int8_t blink_cycles){
    pulse_enable = true;
	blink_enable = false;
	duty_timer = millis();
    this->blink_cycles = blink_cycles;
	pulse_stage = 0;
	this->duty_low = duty_low;
	this->duty_high = duty_high;
	this->rising_time = rising_time;
	this->falling_time = falling_time;
	this->low_time = low_time;
	this->high_time = high_time;
	if (start_high){
		writePulse(duty_high);
		duty_timer += high_time;
		pulse_stage = 2;
	} else {
		writePulse(duty_low);
		duty_timer += low_time;
		pulse_stage = 0;
	}
}

void LedPWM::update() {
	if (pulse_enable && blink_cycles != 0) {
		uint32_t thismillis = millis();
		switch (pulse_stage){
			case 0: // low time
				if (thismillis < (duty_timer + low_time)) {
					writePulse(duty_low);
					// Serial.print("Low     : ");
					// Serial.println(duty_low);
				} else {
					pulse_stage = 1;
					duty_timer = thismillis;
				}
				break;
			case 1: // pulse rising
				if (thismillis < (duty_timer + rising_time)) {
					writePulse(duty_high-((float(duty_timer + rising_time) - thismillis)/float(rising_time))*(duty_high-duty_low));
					// Serial.print("Rising  : ");
					// Serial.println(duty_high-((float(duty_timer + rising_time) - thismillis)/float(rising_time))*(duty_high-duty_low));
				} else {
					pulse_stage = 2;
					duty_timer = thismillis;
				}
				break;
			case 2: // high time
				if (thismillis < (duty_timer + high_time)) {
					writePulse(duty_high);
					// Serial.print("High    : ");
					// Serial.println(duty_high);
				} else {
					pulse_stage = 3;
					duty_timer = thismillis;
				}
				break;
			case 3:
				if (thismillis < (duty_timer + falling_time)) {
					writePulse(duty_low+((float(duty_timer + falling_time) - thismillis)/float(falling_time))*(duty_high-duty_low));
					// Serial.print("Falling : ");
					// Serial.println(duty_low+((float(duty_timer + falling_time) - thismillis)/float(falling_time))*(duty_high-duty_low));
				} else {
					pulse_stage = 0;
					duty_timer = thismillis;
                    if (blink_cycles > 0) {
                        blink_cycles --;
                    }
				}
				break;
		}
	} else if (blink_enable) {
		uint32_t thismillis = millis();
		// Serial.print(thismillis);
		// Serial.print(" : ");
		// Serial.println(blink_change_time);
		if (thismillis >= blink_change_time) {
			if (state) {
				blink_change_time += blink_time_off;
				if (blink_cycles > 0) {
					blink_cycles --;
				}
			} else {
				blink_change_time += blink_time_on;
			}
			writeBlink(!state);
		}
    }
    if (blink_cycles == 0) {
    blink_enable = false;
    pulse_enable = false;
    write(false); // Is this a good idea? Test this line!
    }
}

void LedPWM::writePulse(int32_t duty_cycle) {
	ledcWrite(pwm_channel, constrain(duty_cycle, 0, pwm_res_max));
}

FLed::FLed () {

}

void FLed::init(CRGB *leds, byte led_length) {
    this->led_length = led_length;
    leds_addr = leds;
    // leds[led_length];
    FastLED.addLeds<WS2812B, 13, GRB>(leds_addr, led_length).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( 8 );
}

void FLed::write(byte led_no, CRGB colour) {
    leds_addr[led_no] = colour;
    this->blink_enable[led_no] = 0;
}

void FLed::writergb(byte led_no, uint8_t clr_r, uint8_t clr_g, uint8_t clr_b) {
    leds_addr[led_no].setRGB(clr_r, clr_g, clr_b);
    this->blink_enable[led_no] = 0;
}

CRGB FLed::read(byte led_no) {
    return leds_addr[led_no];
}

void FLed::update() {
    // FastLED.show();
}

FLedBlinkable::FLedBlinkable() : FLed() {

}

void FLedBlinkable::blink(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time) {
	this->blink_time_1[led_no] = blink_time;
	this->blink_time_2[led_no] = blink_time;
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
	this->blink_cycles[led_no] = -1;
	this->blink_change_time[led_no] = millis() + blink_time;
    this->blink_enable[led_no] = 1;
    this->state[led_no] = 0;
	// blinkStart(led_no, start_state);
}

void FLedBlinkable::blink(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time, int8_t blink_cycles) {
	this->blink_time_1[led_no] = blink_time;
	this->blink_time_2[led_no] = blink_time;
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
	this->blink_cycles[led_no] = blink_cycles;
	this->blink_change_time[led_no] = millis() + blink_time;
    this->blink_enable[led_no] = 1;
    this->state[led_no] = 0;
	// blinkStart(led_no, start_state);
}

void FLedBlinkable::blink(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time_1, uint32_t blink_time_2, int8_t blink_cycles) {
	this->blink_time_1[led_no] = blink_time_1;
	this->blink_time_2[led_no] = blink_time_2;
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
	this->blink_cycles[led_no] = blink_cycles;
	this->blink_change_time[led_no] = millis() + blink_time_1;
    this->blink_enable[led_no] = 1;
    this->state[led_no] = 0;
	// blinkStart(led_no, start_state);
}

void FLedBlinkable::update() {
    for (byte ii = 0; ii < led_length; ii++) {
        if (blink_enable[ii] == 1) {
            uint32_t thismillis = millis();
            if (thismillis >= blink_change_time[ii]) {
                if (state[ii] == 0) { // Need to switch to colour 1
                    blink_change_time[ii] += blink_time_1[ii];
                    leds_addr[ii] = colour1[ii];
                    state[ii] = 1;
                    // Serial.printf("Led %i changed to %lu\n", ii, colour1);
                } else { // Need to switch to colour 2
                    blink_change_time[ii] += blink_time_2[ii];
                    leds_addr[ii] = colour2[ii];
                    state[ii] = 0;
                    // Serial.printf("Led %i changed to %lu\n", ii, colour2);
                }
                if (blink_cycles[ii] > 0) {
                    blink_cycles[ii] --;
                }
            }
        }
        if (blink_cycles[ii] == 0) {
            blink_enable[ii] = 0;
        }
    }
    // FastLED.show();
}

FLedPWM::FLedPWM() : FLedBlinkable() {

}

void FLedPWM::fade(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time_1, uint32_t blink_time_2, uint32_t blink_time_1_2, uint32_t blink_time_2_1) {
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
    this->blink_time_1[led_no] = blink_time_1;
	this->blink_time_2[led_no] = blink_time_2;
    this->blink_time_1_2[led_no] = blink_time_1_2;
    this->blink_time_2_1[led_no] = blink_time_2_1;
	this->blink_cycles[led_no] = -1;
	this->blink_change_time[led_no] = millis() + blink_time_1;
    this->blink_enable[led_no] = 2;
    this->state[led_no] = 0;
    this->pulse_end_state[led_no] = 0;
}

void FLedPWM::fade(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time_1, uint32_t blink_time_2, uint32_t blink_time_1_2, uint32_t blink_time_2_1, byte start_state) {
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
    this->blink_time_1[led_no] = blink_time_1;
	this->blink_time_2[led_no] = blink_time_2;
    this->blink_time_1_2[led_no] = blink_time_1_2;
    this->blink_time_2_1[led_no] = blink_time_2_1;
	this->blink_cycles[led_no] = -1;
    this->blink_enable[led_no] = 2;
    this->state[led_no] = start_state;
    if (start_state == 0) {
        this->blink_change_time[led_no] = millis() + blink_time_1;
    } else if (start_state == 1) {
        this->blink_change_time[led_no] = millis() + blink_time_1_2;
    } else if (start_state == 2) {
        this->blink_change_time[led_no] = millis() + blink_time_2;
    } else if (start_state == 3) {
        this->blink_change_time[led_no] = millis() + blink_time_2_1;
    }
    this->pulse_end_state[led_no] = 0;
}

void FLedPWM::fade(byte led_no, CRGB colour1, CRGB colour2, uint32_t blink_time_1, uint32_t blink_time_2, uint32_t blink_time_1_2, uint32_t blink_time_2_1, byte start_state, byte pulse_cycles, byte end_state) {
    this->colour1[led_no] = colour1;
    this->colour2[led_no] = colour2;
    this->blink_time_1[led_no] = blink_time_1;
	this->blink_time_2[led_no] = blink_time_2;
    this->blink_time_1_2[led_no] = blink_time_1_2;
    this->blink_time_2_1[led_no] = blink_time_2_1;
	this->blink_cycles[led_no] = pulse_cycles;
    this->blink_enable[led_no] = 2;
    this->state[led_no] = start_state;
    if (start_state == 0) {
        this->blink_change_time[led_no] = millis() + blink_time_1;
    } else if (start_state == 1) {
        this->blink_change_time[led_no] = millis() + blink_time_1_2;
    } else if (start_state == 2) {
        this->blink_change_time[led_no] = millis() + blink_time_2;
    } else if (start_state == 3) {
        this->blink_change_time[led_no] = millis() + blink_time_2_1;
    }
    this->pulse_end_state[led_no] = end_state; // should be either 1 or 3. 0 or 2 means fading doesn't stop - unhandled
}

void FLedPWM::update() {
    for (byte ii = 0; ii < led_length; ii++) {
        if (blink_enable[ii] == 1) {
            uint32_t thismillis = millis();
            if (thismillis >= blink_change_time[ii]) {
                if (state[ii] == 0) { // Need to switch to colour 1
                    blink_change_time[ii] += blink_time_1[ii];
                    leds_addr[ii] = colour1[ii];
                    state[ii] = 1;
                    // Serial.printf("Led %i changed to %lu\n", ii, colour1);
                } else { // Need to switch to colour 2
                    blink_change_time[ii] += blink_time_2[ii];
                    leds_addr[ii] = colour2[ii];
                    state[ii] = 0;
                    // Serial.printf("Led %i changed to %lu\n", ii, colour2);
                }
                if (blink_cycles[ii] > 0) {
                    blink_cycles[ii] --;
                }
            }
        } else if (blink_enable[ii] == 2) {
            uint32_t thismillis = millis();
            // Check if the pulse state needs to change
            if (thismillis >= blink_change_time[ii]) {
                if (state[ii] == 0) { // Need to fade from 2 to 1
                    blink_change_time[ii] += blink_time_2_1[ii];
                    // leds_addr[ii] = colour1[ii];
                    state[ii] = 1;
                } else if (state[ii] == 1) { // Need to hold colour 1
                    blink_change_time[ii] += blink_time_1[ii];
                    leds_addr[ii] = colour1[ii];
                    state[ii] = 2;
                } else if (state[ii] == 2) { // Need to fade from 1 to 2
                    blink_change_time[ii] += blink_time_1_2[ii];
                    // leds_addr[ii] = colour2[ii];
                    state[ii] = 3;
                } else { // Need to hold colour 2
                    blink_change_time[ii] += blink_time_2[ii];
                    leds_addr[ii] = colour2[ii];
                    state[ii] = 0;
                }
                if (state[ii] == 0 && blink_cycles[ii] > 0) {
                    blink_cycles[ii] --;
                }
                Serial.printf("Led %i now state %i\n", ii, state[ii]);
            }
            // Update colours on fade up/down
            if (state[ii] == 1) {
                float cmix2 = (float(blink_change_time[ii]-thismillis)/float(blink_time_1_2[ii]));
                float cmix1 = (float(thismillis-(blink_change_time[ii]-blink_time_1_2[ii]))/float(blink_time_1_2[ii]));
                uint8_t this_r = colour1[ii].r * cmix1 + colour2[ii].r * cmix2;
                uint8_t this_g = colour1[ii].g * cmix1 + colour2[ii].g * cmix2;
                uint8_t this_b = colour1[ii].b * cmix1 + colour2[ii].b * cmix2;
                leds_addr[ii].setRGB(this_r, this_g, this_b);
                // Serial.printf("Led %i is %f %% the way through. Colour = %f + %f\n", ii, cmix1, cmix1, cmix2);
                // Serial.printf("Led %i = %i, %i, %i\n", ii, this_r, this_g, this_b);
            } else if (state[ii] == 3) {
                float cmix1 = (float(blink_change_time[ii]-thismillis)/blink_time_2_1[ii]);
                float cmix2 = (float(thismillis-(blink_change_time[ii]-blink_time_2_1[ii]))/blink_time_2_1[ii]);
                uint8_t this_r = colour1[ii].r * cmix1 + colour2[ii].r * cmix2;
                uint8_t this_g = colour1[ii].g * cmix1 + colour2[ii].g * cmix2;
                uint8_t this_b = colour1[ii].b * cmix1 + colour2[ii].b * cmix2;
                leds_addr[ii].setRGB(this_r, this_g, this_b);
                // Serial.printf("Led %i is %f %% the way through. Colour = %f + %f\n", ii, cmix2, cmix1, cmix2);
                // Serial.printf("Led %i = %i, %i, %i\n", ii, this_r, this_g, this_b);
            }
        }
        if (blink_enable[ii] == 1) {
            if (blink_cycles[ii] == 0) {
                blink_enable[ii] = 0;
            }
        } else if (blink_enable[ii] == 2) {
            if (pulse_end_state[ii] == state[ii]) {
                if (blink_cycles[ii] == 0) {
                    blink_enable[ii] = 0;
                }
            }
        }
    }
}

Switch::Switch() {
	
}

void Switch::init(byte pin) {
	this->pin = pin;
	state_prev = HIGH;
	pinMode(pin, INPUT_PULLUP);
	debounce_delay = 50;
	debounce_timer = 0;
	read();
}

void Switch::init(byte pin, int16_t custom_debounce) {
	this->pin = pin;
	state_prev = HIGH;
	pinMode(pin, INPUT_PULLUP);
	debounce_delay = custom_debounce;
	debounce_timer = 0;
	read();
}

void Switch::read() {
	uint32_t thismillis;
	thismillis = millis();
	if (thismillis >= debounce_timer) {
		bool reading = digitalRead(pin);
		if (reading != state) {
			state = reading;
			debounce_timer = thismillis + debounce_delay;
		}
	}
}

bool Switch::hasChanged() {
	read();
	if (state != state_prev) {
		state_prev = state;
		return true;
	} else {
		return false;
	}
}

bool Switch::isPressed() {
	return (state == LOW);
}

uint32_t Switch::lastChange() {
	return debounce_timer;
}












