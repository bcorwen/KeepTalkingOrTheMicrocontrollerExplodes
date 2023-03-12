//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 03/06/22
//======================================================================
//
//  Module: Button (Slave, Solvable, Vanilla)
//  version 0.8.0
//
//  Version goals: Split out module-specific library
//
//======================================================================

// #define DEBUG 1

//**********************************************************************
// LIBRARIES
//**********************************************************************

#include <Arduino.h>
#include <KTOME_Button.h>
#include <KTOME_common.h>

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

KTOME_Button::KTOME_Button() {

}

void KTOME_Button::start(FLedPWM *fleds) {
    fleds_addr = fleds; // Store pointer to leds
    switches.init(pin_array);
    reset();
}

void KTOME_Button::reset() {
    fleds_addr->write(1, CRGB::Black);
    strike_count = 0;
    game_running = false;
    module_solved = false;
    wait_for_timer = false;
    strip_on = false;
    stripOn();
}

void KTOME_Button::powerOn() {
    fleds_addr->fade(1, CRGB::Green, CRGB::Black, 350, 150, 150, 150, 0, 3, 0);
}

void KTOME_Button::generate() {
    button_colour = random(4);
    button_label = random(4);
    DEBUG_PRINT(F("Button colour: "));
    DEBUG_PRINTLN(button_colours[button_colour]);
    DEBUG_PRINT(F("Button label: "));
    DEBUG_PRINTLN(button_labels[button_label]);
}

bool KTOME_Button::isManual() { // Required manual setup
    return true;
}

String KTOME_Button::getManual() {
    String manual_string = "c";
    manual_string = manual_string + char(button_colour+'0') + char(button_label+'0');
    return manual_string;
}

byte KTOME_Button::manualConfirm() { // No further confirm needed for this module, return 0
    return 0;
}

void KTOME_Button::findSolution() {
    if (button_colour == 0 && button_label == 0) {
        shortpushneeded = false;
    } else if (battery_number > 1 && button_label == 1) {
        shortpushneeded = true;
    } else if (ind_car == 1 && button_colour == 1) {
        shortpushneeded = false;
    } else if (battery_number > 2 && ind_frk == 1) {
        shortpushneeded = true;
    } else if (button_colour == 2) {
        shortpushneeded = false;
    } else if (button_colour == 3 && button_label == 2) {
        shortpushneeded = true;
    } else {
        shortpushneeded = false;
    }

    DEBUG_PRINT("CAR: ");
    DEBUG_PRINT(ind_car);
    DEBUG_PRINT(" | FRK: ");
    DEBUG_PRINT(ind_frk);
    DEBUG_PRINT(" | Batts: ");
    DEBUG_PRINTLN(battery_number);

    DEBUG_PRINT("Button colour: ");
    DEBUG_PRINT(button_colours[button_colour]);
    DEBUG_PRINT(" | Button label: ");
    DEBUG_PRINTLN(button_labels[button_label]);

    DEBUG_PRINT(F("Short push?: "));
    DEBUG_PRINTLN(shortpushneeded);

    strip_colour = random(4);
    DEBUG_PRINT(F("Strip colour: "));
    DEBUG_PRINTLN(button_colours[strip_colour]);

    DEBUG_PRINT(F("Release on "));
    if (strip_colour == 0) {
        DEBUG_PRINTLN("4.");
    } else if (strip_colour == 2) {
        DEBUG_PRINTLN("5.");
    } else {
        DEBUG_PRINTLN("1.");
    }
}

void KTOME_Button::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    this->serial_vowels = vowels;
    this->serial_odd = odds;
    this->battery_number = batts;
    this->ind_car = cars;
    this->ind_frk = frks;
    this->port_s = serials;
    this->port_p = parallels;
    findSolution();
}

void KTOME_Button::strikeUpdate(byte strikes) {

}

byte KTOME_Button::timerUpdate(String timer_digits) {
    byte return_byte = 0;
    if (wait_for_timer){
        return_byte = 1;
        if (strip_colour == 0) { // 4
            for (byte ii = 0; ii < 4; ii++){
                if (timer_digits.charAt(ii) == '4'){
                    return_byte = 2;
                    DEBUG_PRINTLN("Correctly released on a '4'");
                }
            }
        } else if (strip_colour == 2) { // 5
            for (byte ii = 0; ii < 4; ii++){
                if (timer_digits.charAt(ii) == '5'){
                    return_byte = 2;
                    DEBUG_PRINTLN("Correctly released on a '5'");
                }
            }
        } else if (strip_colour == 1 || strip_colour == 3) { // 1
            for (byte ii = 0; ii < 4; ii++){
                if (timer_digits.charAt(ii) == '1'){
                    return_byte = 2;
                    DEBUG_PRINTLN("Correctly released on a '1'");
                }
            }
        }
    } else {
        DEBUG_PRINTLN("Was not waiting for timer message...");
    }
    if (return_byte == 1) { DEBUG_PRINTLN("Incorrectly released!"); }
    wait_for_timer = false;
    return return_byte;
}

void KTOME_Button::gameStart() {
    game_running = true;
}

byte KTOME_Button::inputCheck() { // Button can still be pressed, strips lights and trigger strike even after a solve
    byte return_byte = 0;
    if (hasButtonBeenReleased()) {
        return_byte = logicCheck();
    }
    // DEBUG_PRINTLN(wait_for_timer);
    return return_byte;
}

byte KTOME_Button::logicCheck(){
    if (isthisapress && shortpushneeded) { // Correct short press - win
        wait_for_timer = false;
        module_solved = true;
        return 2;
    } else if (!isthisapress && !shortpushneeded) { // Correct long hold - check time
        wait_for_timer = true;
        DEBUG_PRINTLN("Waiting for timer to determine action...");
        outbox_waiting = true;
        outbox_msg = "t";
        return 0;
    } else { // Incorrect hold or press
        wait_for_timer = false;
        return 1;
    }
}

bool KTOME_Button::hasButtonBeenReleased() {
    if (switches.hasChanged()) {
        if (switches.isPressed()) {
            wait_for_timer = false;
            isthisapress = true;
            DEBUG_PRINTLN("Button pressed!");
            return false;
        } else {
            DEBUG_PRINTLN("Button released!");
            strip_on = false;
            stripOn();
            return true;
        }
    } else {
        if ((switches.isPressed()) && ((switches.lastChange() + 1000) <= millis()) && (!strip_on)) {
            isthisapress = false;
            strip_on = true;
            stripOn();
        }
        return false;
    }
}

void KTOME_Button::stripOn(){
    if (strip_on) {
        byte this_colour;
        if (shortpushneeded) {
            // Use random colour from button_colours
            this_colour = random(4);
        } else {
            // Use strip_colour
            this_colour = strip_colour;
        }
        DEBUG_PRINT("Strip colour: ");
        DEBUG_PRINTLN(this_colour);
        if (this_colour >= 1) { //Add red
            // DEBUG_PRINTLN("Adding red...");
            // leds_button[0].setPulse(255,127,166,333,500,333);
        }
            if (this_colour == 1 || this_colour == 2) { //Add green
            // DEBUG_PRINTLN("Adding green...");
            // leds_button[1].setPulse(153,76,166,333,500,333);
        }      
        if (this_colour == 0 || this_colour == 1) { //Add blue
            // DEBUG_PRINTLN("Adding blue...");
            // leds_button[2].setPulse(153,76,166,333,500,333);
        }
        fleds_addr->fade(1, button_colour_hi[this_colour], button_colour_lo[this_colour], 500, 166, 333, 333);
    } else {
        // Turn lights off
        for (byte ii = 0; ii < 3; ii++){
            // leds_button[ii].setPulse(0,0,166,333,500,333);
            // leds_button[ii].write(false);
            fleds_addr->write(1, CRGB::Black);
        }
    }
}

bool KTOME_Button::isSolved() {
    return module_solved;
}

byte KTOME_Button::update() { // Update anything not controlled by FastLED
    return 0;
}

void KTOME_Button::explode() {
    game_running = false;
    fleds_addr->blink(1, CRGB::Red, CRGB::Black, 100, 2);
}

void KTOME_Button::defuse() {
    game_running = false;
}

String KTOME_Button::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Button::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Button::needsIsr() {
    return false;
}

void KTOME_Button::isrHandler() {
    
}