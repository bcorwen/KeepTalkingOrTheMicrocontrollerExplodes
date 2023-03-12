//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 03/06/22
//======================================================================
//
//  Module: Simon (Slave, Solvable, Vanilla)
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
#include <KTOME_Simon.h>

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

KTOME_Simon::KTOME_Simon() {

}

void KTOME_Simon::start(FLedPWM *fleds) {
    for (byte ii = 0; ii < 4; ii++)
    {
        leds[ii].init(pin_array[ii], 5000, ii);
        switches[ii].init(pin_array[ii + 4], 200);
    }
    reset();
}

void KTOME_Simon::reset() {
    lightsOut();
    stage = 0;
    strike_count = 0;
    game_running = false;
    module_solved = false;
    user_interacted = false;
    user_interrupt = false;
    stage = 0;
    step = 0;
}

void KTOME_Simon::powerOn() {
    leds[0].setPulse(255, 0, 1, 100, 300, 100, false, 1);
    leds[1].setPulse(255, 0, 100, 100, 300, 100, false, 1);
    leds[2].setPulse(255, 0, 150, 100, 300, 100, false, 1);
    leds[3].setPulse(255, 0, 250, 100, 300, 100, false, 1);
}

void KTOME_Simon::generate() {
    stages = random(2) + 3;
    DEBUG_PRINT(F("Number of stages: "));
    DEBUG_PRINTLN(stages);

    DEBUG_PRINT("Sequence: ");
    for (byte ii = 0; ii < stages; ii++)
    {
        sequence_lights[ii] = random(4);
        DEBUG_PRINT(light_colours[sequence_lights[ii]]);
        if (ii < (stages - 1))
        {
            DEBUG_PRINT(" > ");
        }
        else
        {
            DEBUG_PRINTLN();
        }
    }
}

bool KTOME_Simon::isManual() {
    return false;
}

String KTOME_Simon::getManual() {
    DEBUG_PRINTLN("ERROR: Should not have been asked to fetch manual setup!");
    return "";
}

byte KTOME_Simon::manualConfirm() { // No further confirm needed for this module, return 0
    return 0;
}

void KTOME_Simon::findSolution() {
    byte vowel_row;
    if (serial_vowels)
    {
        vowel_row = 0;
    }
    else
    {
        vowel_row = 1;
    }
    DEBUG_PRINT("Current Simon solution (for vowel=");
    DEBUG_PRINT(serial_vowels);
    DEBUG_PRINT(" & strikes=");
    DEBUG_PRINT(strike_count);
    DEBUG_PRINT("): ");
    for (byte ii = 0; ii < 4; ii++)
    {
        button_to_light[ii] = translation[strike_count][vowel_row][ii];
    }
    for (byte ii = 0; ii < stages; ii++)
    {
        DEBUG_PRINT(light_colours[button_to_light[sequence_lights[ii]]]);
        if (ii < (stages - 1))
        {
            DEBUG_PRINT(" > ");
        }
        else
        {
            DEBUG_PRINTLN();
        }
    }
}

void KTOME_Simon::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    this->serial_vowels = vowels;
    this->serial_odd = odds;
    this->battery_number = batts;
    this->ind_car = cars;
    this->ind_frk = frks;
    this->port_s = serials;
    this->port_p = parallels;
    findSolution();
}

void KTOME_Simon::strikeUpdate(byte strikes) {
    this->strike_count = strikes;
    findSolution();
}

byte KTOME_Simon::timerUpdate(String timer_digits) {
    return 0;
}

void KTOME_Simon::gameStart() {
    game_running = true;
}

byte KTOME_Simon::inputCheck() { // 0 no action, 1 = strike, 2 = solve

    byte return_byte = 0;
    for (byte ii = 0; ii < 4; ii++) {
        if (hasButtonBeenPushed(ii)) {
            if (game_running && !module_solved) {
                return_byte = logicCheck(ii);
            }
        }
    }
    return return_byte;
}

byte KTOME_Simon::logicCheck(byte button_number) {

    uint32_t this_time = millis();

    DEBUG_PRINT("Stage: ");
    DEBUG_PRINT(stage);
    DEBUG_PRINT(" | Step: ");
    DEBUG_PRINTLN(step);

    if (button_to_light[sequence_lights[step]] == button_number) {                                           // Correct button pressed
        light_timing = this_time + reset_time;  // Push back time for demo to begin
        button_timing = this_time + reset_time; // Push back time for user's input to be reset
        step++;
        if (step > stage) {
            stage = step;
            light_timing = this_time + advance_time;
            button_timing = this_time + advance_time;
            DEBUG_PRINTLN("Correct button, stage complete!");
            if (stage == stages) {
                module_solved = true;
                return 2;
            }
        } else {
            DEBUG_PRINTLN("Correct button, next step...");
        }
        return 0; // Correct input
    } else {
        light_timing = this_time + strike_time; // Demo to start up after shorter time
        button_timing = this_time;              // Assume end of user input now
        step = 0;
        return 1; // Trigger for a strike
    }
}

bool KTOME_Simon::hasButtonBeenPushed(byte button_number) {
    if (switches[button_number].hasChanged()) {
        if (switches[button_number].isPressed()) {
            DEBUG_PRINT(button_number);
            DEBUG_PRINTLN(" button pressed!");
            user_interacted = true;
            if (!isSolved()) {
                outbox_waiting = true;
                outbox_msg = "u";
                outbox_msg.concat(button_number);
                outbox_msg.concat("0");
            }
            user_interrupt = true;
            // lightsOut();
            leds[button_number].setPulse(255, 0, 1, 100, 300, 100, false, 1);
            return true;
        }
    }
    return false;
}

void KTOME_Simon::lightsOut() {
    for (byte ii = 0; ii < 4; ii++) {
        leds[ii].write(false);
    }
}

bool KTOME_Simon::isSolved() {
    return module_solved;
}

byte KTOME_Simon::update() {

    byte return_result = 0;
    for (byte ii = 0; ii < 4; ii++) {
        leds[ii].update();
    }
    uint32_t this_time = millis();
    // DEBUG_PRINT("user_interrupt: ");
    // DEBUG_PRINTLN(user_interrupt);
    if (game_running && !module_solved) {
        if (button_timing <= this_time && user_interrupt) { // User has interruped demo but has now taken too long to input next button press...
            // DEBUG_PRINTLN("Switching back to demo...");
            user_interrupt = false;
            step = 0; // User needs to restart at the beginning of the sequence
            disp_step = 0;
        }
        if (light_timing <= this_time && !user_interrupt) { // We are due the next light in the demo and the user isn't inputting anything...
            // DEBUG_PRINTLN("Next light triggered...");
            leds[sequence_lights[disp_step]].setPulse(255, 0, 1, 100, 300, 100, false, 1);
            // return_result = (1 << sequence_lights[disp_step]);
            if (user_interacted) {
                outbox_waiting = true;
                outbox_msg = "u";
                outbox_msg.concat(sequence_lights[disp_step]);
                outbox_msg.concat("1");
            }
            light_timing = this_time + led_on_time + led_off_time;
            button_timing = this_time;
            disp_step++;
            if (disp_step > stage) { // Sequence ended, restart
                // DEBUG_PRINTLN("which was the last in the seq! Back to first light...");
                disp_step = 0;
                light_timing = this_time + reset_time;
            }
        }
    }
    return return_result;
}

void KTOME_Simon::explode() {
    // lightsOut();
    for (byte ii = 0; ii < 4; ii++) {
        leds[ii].blink(false, 100, 1);
    }
}

void KTOME_Simon::defuse() {
    game_running = false;
}

String KTOME_Simon::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Simon::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Simon::needsIsr() {
    return false;
}

void KTOME_Simon::isrHandler() {
    
}