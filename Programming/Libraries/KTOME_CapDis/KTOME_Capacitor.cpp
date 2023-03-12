//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 05/02/23
//======================================================================
//
//  Module: Capacitor Discharge (Slave, Needy, Vanilla)
//  version 0.8.0
//
//  Version goals: Split out module-specific library
//
//======================================================================

#define DEBUG 1

//**********************************************************************
// LIBRARIES
//**********************************************************************

#include <Arduino.h>
#include <KTOME_common.h>
#include <KTOME_Capacitor.h>
#include <ShiftDisplay.h>
// #include <Adafruit_MCP23X17.h>

#ifdef DEBUG
    #define DEBUG_SERIAL(x)     Serial.begin(x)
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTBIN(x)   Serial.print(x, BIN)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTLNBIN(x) Serial.println(x, BIN)
    #define DEBUG_PADZERO(x)    ktomeCAN.padZeros(x)
#else
    #define DEBUG_SERIAL(x)
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTBIN(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTLNBIN(x)
    #define DEBUG_PADZERO(x)
#endif

KTOME_CapDis::KTOME_CapDis() {
    
}

void KTOME_CapDis::start() {
    fled.init(&leds[0], FLED_LENGTH);
    FastLED.addLeds<WS2812B, CAP_LED, GRB>(leds, FLED_LENGTH).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( 16 );
    // pinMode(CAP_POT, ANALOG); // Only if using rotary potentiometer instead of microswitch
    switches.init(pin_array); // Only if using microswitch instead of rotary potentiometer
    // mcp.begin_I2C();
    for (byte ii = 0; ii < 10; ii++){
        // mcp.pinMode(led_array[ii], OUTPUT);
        // mcp.digitalWrite(led_array[ii], LOW);
    }
    reset();
}

void KTOME_CapDis::reset() {
    // module_defused = false;
    game_running = 0;
    cap_popped = 0;
    needy_active = 0;
    accept_input = false;
}

void KTOME_CapDis::powerOn() {
    Serial.println("Fading...");
    fled.fade(0, CRGB::Yellow, CRGB::Black, 350, 150, 150, 150, 0, 3, 0);
    // fled.blink(1, CRGB::White, CRGB::Black, 20, 2);
}

void KTOME_CapDis::generate() {
    
}

bool KTOME_CapDis::isManual() {
    return false;
}

String KTOME_CapDis::getManual() {
    DEBUG_PRINTLN("ERROR: Should not have been asked to fetch manual setup!");
    return "";
}

byte KTOME_CapDis::manualConfirm() { // No further confirm needed for this module, return 0
    return 0;
}

void KTOME_CapDis::strikeUpdate(byte strikes) {
    
}

byte KTOME_CapDis::timerUpdate(String timer_digits) {
    return 0;
}

void KTOME_CapDis::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    
}

void KTOME_CapDis::gameStart() {
    game_running = true;
}

void KTOME_CapDis::sleep() {
    DEBUG_PRINTLN("Module sleeps..");
    needy_active = 1;
    shift_timer.set("  ");
    shift_timer.show();
}

bool KTOME_CapDis::isSolved() {
    return module_solved;
}

byte KTOME_CapDis::update() {
    byte return_byte;

    if (game_running){
        int32_t time_step = millis() - this_millis;
        this_millis += time_step;
        if (needy_active == 2) {
            if (lever_down) {
                needy_timer += 6*time_step;
                if (needy_timer > this_millis+45000){
                    needy_timer = this_millis+45000;
                }
                fled.writergb(0, brightnesslut[int(127*((45000.0-(needy_timer-this_millis))/45000.0))], brightnesslut[int(127*((45000.0-(needy_timer-this_millis))/45000.0))], 0);
                // Serial.println(int(127*((45000.0-(needy_timer-this_millis))/45000.0)));
            } else {
                fled.write(0, CRGB::Black);
            }
            needy_timer_ms = needy_timer-this_millis;
            display_timer = (999 + needy_timer - this_millis)/1000;
            // DEBUG_PRINTLN(display_timer);
            shift_timer.set(display_timer);
            if (display_timer < 10) {
                // shift_timer.setAt(0, '0');
                // shift_timer.setAt(1, '0');
                String disp_low = "0" + String(display_timer);
                shift_timer.set(disp_low);
            }
            shift_timer.show();
            int8_t leds_on = (45000.0-(needy_timer-this_millis))/4500.0;
            // Serial.println(leds_on);
            for (byte ii = 0; ii < 10; ii++){
                if (ii <= leds_on){
                    // mcp.digitalWrite(led_array[ii], LOW);
                } else {
                    // mcp.digitalWrite(led_array[ii], HIGH);
                }
            }
        } else {
            shift_timer.set("  ");
            shift_timer.show();
            // Set LED array blank
        }

        if (this_millis > needy_timer && needy_active > 1) {
            DEBUG_PRINTLN("Needy timer runs out...");
            if (needy_active == 2) { // Needy is active so this indicates needy timer has run out...
                sleep();
                fled.blink(1, CRGB::White, CRGB::Black, 20, 2);
                cap_popped = 1;
                return 1;
            } else if (needy_active == 1) { // Needy is dormant, so wake this module up...
                // activate(); // Do not activate again, cap discharge should not awake after previously striking!
            }
        }
    }

    fled.update();
    FastLED.show();
       
    return 0;
}

byte KTOME_CapDis::inputCheck() {
    // DEBUG_PRINTLN("Inputs checked...");
    // byte result_byte = 0;

    if (isActive()){
        // lever_now = analogRead(CAP_POT); // Only if using rotary potentiometer instead of microswitch
        // DEBUG_PRINTLN(lever_now);
        // if (lever_now > 4000){
        //     lever_down = true;
        //     if (lever_last <= 4000){
        //         outbox_waiting = true;
        //         outbox_msg = "u+" + String(needy_timer_ms);
        //     }
        // } else if (lever_now <= 3900) {
        //     lever_down = false;
        //     if (lever_last > 3900){
        //         outbox_waiting = true;
        //         outbox_msg = "u-" + String(needy_timer_ms);
        //     }
        // }
        // lever_last = lever_now;
        if (switches.hasChanged()) {
            if (switches.isPressed()) {
                DEBUG_PRINTLN("Lever lowered!");
                lever_down = true;
                outbox_waiting = true;
                outbox_msg = "u+" + String(needy_timer_ms);
            } else {
                DEBUG_PRINTLN("Lever lifted!");
                lever_down = false;
                outbox_waiting = true;
                outbox_msg = "u-" + String(needy_timer_ms);
            }
        }
    }

    return 0;
}

byte KTOME_CapDis::logicCheck(){
    DEBUG_PRINTLN("Logic checked...");
    
}

bool KTOME_CapDis::isActive() {
	if (needy_active == 2) {
		return true;
	} else {
		return false;
	}
}

bool KTOME_CapDis::isAwake() {
	if (needy_active > 0) {
		return true;
	} else {
		return false;
	}
}

void KTOME_CapDis::activate() {
    DEBUG_PRINTLN("Module awakes...");
    needy_active = 2;
    cap_popped = 0;
    this_millis = millis();
    needy_timer = this_millis + 45000; // 45 seconds
    outbox_waiting = true;
    outbox_msg = "u-45000";
    accept_input = true;
    DEBUG_PRINTLN("Module awoken.");
}

void KTOME_CapDis::defuse() {
    DEBUG_PRINTLN("Defused method");
    game_running = false;
}

void KTOME_CapDis::explode() {
    DEBUG_PRINTLN("Explode!");
    game_running = false;
}

String KTOME_CapDis::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_CapDis::isOutbox() {
    return outbox_waiting;
}

bool KTOME_CapDis::needsIsr() {
    return false;
}

void KTOME_CapDis::isrHandler() {
    
}
