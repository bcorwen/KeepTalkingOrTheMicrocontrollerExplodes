//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 14/06/22
//======================================================================
//
//  Module: Keypads (Slave, Solvable, Vanilla)
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
#include <KTOME_Keypad.h>

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

KTOME_Keypad::KTOME_Keypad() {

}

void KTOME_Keypad::start(FLedPWM *fleds) {
    fleds_addr = fleds; // Store pointer to leds
    for (byte ii = 0; ii < 4; ii++) {
        switches[ii].init(pin_array[ii]);
    }
    reset();
}

void KTOME_Keypad::reset() {
    for (byte ii = 0; ii < 4; ii++) {
        fleds_addr->write(ii + 1, CRGB::Black);
    }
    game_running = false;
    module_solved = false;
    stage = 0;
}

void KTOME_Keypad::powerOn() {
    fleds_addr->fade(1, CRGB::Green, CRGB::Black, 100, 0, 100, 100, 0, 1, 0);
    fleds_addr->fade(2, CRGB::Green, CRGB::Black, 100, 150, 100, 100, 0, 1, 0);
    fleds_addr->fade(3, CRGB::Green, CRGB::Black, 100, 300, 100, 100, 0, 1, 0);
    fleds_addr->fade(4, CRGB::Green, CRGB::Black, 100, 450, 100, 100, 0, 1, 0);
}

void KTOME_Keypad::generate() {

    byte col_select = B00000000;
    byte symbols_picked = 0;
    byte this_pick;
    byte keypad_column_choice;

    keypad_column_choice = random(6);
    DEBUG_PRINT(F("Keypad column: "));
    DEBUG_PRINTLN(keypad_column_choice);

    switch (keypad_column_choice) {
        case 0:
            column_symbols[0] = 25;
            column_symbols[1] = 12;
            column_symbols[2] = 26;
            column_symbols[3] = 11;
            column_symbols[4] = 7;
            column_symbols[5] = 9;
            column_symbols[6] = 21;
            break;
        case 1:
            column_symbols[0] = 15;
            column_symbols[1] = 25;
            column_symbols[2] = 21;
            column_symbols[3] = 23;
            column_symbols[4] = 3;
            column_symbols[5] = 9;
            column_symbols[6] = 18;
            break;
        case 2:
            column_symbols[0] = 1;
            column_symbols[1] = 8;
            column_symbols[2] = 23;
            column_symbols[3] = 5;
            column_symbols[4] = 14;
            column_symbols[5] = 26;
            column_symbols[6] = 3;
            break;
        case 3:
            column_symbols[0] = 10;
            column_symbols[1] = 19;
            column_symbols[2] = 27;
            column_symbols[3] = 7;
            column_symbols[4] = 5;
            column_symbols[5] = 18;
            column_symbols[6] = 4;
            break;
        case 4:
            column_symbols[0] = 22;
            column_symbols[1] = 4;
            column_symbols[2] = 27;
            column_symbols[3] = 21;
            column_symbols[4] = 19;
            column_symbols[5] = 17;
            column_symbols[6] = 2;
            break;
        case 5:
            column_symbols[0] = 10;
            column_symbols[1] = 15;
            column_symbols[2] = 24;
            column_symbols[3] = 13;
            column_symbols[4] = 22;
            column_symbols[5] = 16;
            column_symbols[6] = 6;
            break;
    }

    while (symbols_picked < 4) {
        this_pick = keypadPicker(col_select);
        if (this_pick < 7) {
            selected_symbols[symbols_picked] = this_pick;
            bitSet(col_select, this_pick);
            symbols_picked++;
        }
    }
    DEBUG_PRINT(selected_symbols[0]);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(selected_symbols[1]);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(selected_symbols[2]);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(selected_symbols[3]);

    DEBUG_PRINT(column_symbols[selected_symbols[0]]);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(column_symbols[selected_symbols[1]]);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(column_symbols[selected_symbols[2]]);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(column_symbols[selected_symbols[3]]);

    byte greater_number;
    for (byte i = 0; i < 4; i++) { // i is the element we are looking at
        greater_number = 0;
        for (byte j = 0; j < 4 ; j++) { // j is the other element we are comparing against
            if (selected_symbols[i] > selected_symbols[j]) {
                greater_number++;
            }
        }
        order_symbols[i] = greater_number;
    }
}

byte KTOME_Keypad::keypadPicker(byte col_select) {
    byte looking_at_symbol;
    byte results;
    looking_at_symbol = random(7); // pick a number from 0-6 to choose from the symbol column.
    if (bitRead(col_select, looking_at_symbol) == 0) { //if this symbol hasn't been picked yet
        //    bitSet(col_select, looking_at_symbol);
        results = looking_at_symbol;
    } else {
        results = 7;
    }
    //  DEBUG_PRINTLN(col_select, BIN);
    //  DEBUG_PRINTLN(looking_at_symbol);
    return results;
}

bool KTOME_Keypad::isManual() {
    return true;
}

String KTOME_Keypad::getManual() {
    String manual_string = "c";
    for (byte ii = 0; ii < 4; ii++) {
        manual_string = manual_string + char(column_symbols[selected_symbols[ii]]+'0');
    }
    return manual_string;
}

byte KTOME_Keypad::manualConfirm() {
    return 0;
}

void KTOME_Keypad::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    this->serial_vowels = vowels;
    this->serial_odd = odds;
    this->battery_number = batts;
    this->ind_car = cars;
    this->ind_frk = frks;
    this->port_s = serials;
    this->port_p = parallels;
}

void KTOME_Keypad::strikeUpdate(byte strikes) {

}

byte KTOME_Keypad::timerUpdate(String timer_digits) {
    return 0;
}

void KTOME_Keypad::gameStart() {
    game_running = true;
}

byte KTOME_Keypad::inputCheck() { // 0 no action, 1 = strike, 2 = solve
    byte return_byte = 0;
    for (byte ii = 0; ii < 4; ii++) {
        if (hasButtonBeenPushed(ii)) {
            DEBUG_PRINT("Current stage: ");
            DEBUG_PRINT(stage);
            DEBUG_PRINT(" | order_symbols[button number]: ");
            DEBUG_PRINTLN(order_symbols[ii]);
            if (game_running && !module_solved) {
                return_byte = logicCheck(ii);
            }
        }
    }
    return return_byte;
}

byte KTOME_Keypad::logicCheck(byte button_number){
    if (order_symbols[button_number] == stage) { // Correct key pressed
        stage++;
        fleds_addr->write(button_number+1, CRGB::Green);
        if (stage == 4) {
            module_solved = true;
            return 2;
        }
        return 0;
    } else if (order_symbols[button_number] > stage) { // Wrong key pressed
        fleds_addr->blink(button_number+1, CRGB::Red, CRGB::Black, 500, 2);
        return 1;
    } else {
        return 0;
    }
}

bool KTOME_Keypad::hasButtonBeenPushed(byte button_number) {
    if (switches[button_number].hasChanged()) {
        if (switches[button_number].isPressed()) {
            byte func_return;
            DEBUG_PRINT(button_number);
            DEBUG_PRINTLN(" button pressed!");
            return true;
        } else {
            DEBUG_PRINT(button_number);
            DEBUG_PRINTLN(" button released!");
            return false;
        }
    } else {
        return false;
    }
}

byte KTOME_Keypad::getStage() {
    return stage;
}

bool KTOME_Keypad::isSolved() {
    return module_solved;
}

byte KTOME_Keypad::update() {
    return 0;
}

void KTOME_Keypad::explode() {
    game_running = false;
    for (byte ii = 0; ii < 4; ii++) {
        fleds_addr->blink(ii + 1, CRGB::Red, CRGB::Black, 100, 2);
    }
}

void KTOME_Keypad::defuse() {
    game_running = false;
}

String KTOME_Keypad::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Keypad::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Keypad::needsIsr() {
    return false;
}

void KTOME_Keypad::isrHandler() {
    
}