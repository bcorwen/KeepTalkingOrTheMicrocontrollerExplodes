//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 07/06/22
//======================================================================
//
//  Module: Wires (Slave, Solvable, Vanilla)
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
#include <KTOME_Wires.h>
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

KTOME_Wires::KTOME_Wires() {

}

void KTOME_Wires::start(FLedPWM *fleds) {
    fleds_addr = fleds; // Store pointer to leds
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].init(pin_array[ii]);
    }
    reset();
}

void KTOME_Wires::reset() {
    fleds_addr->write(1, CRGB::Black);
    strike_count = 0;
    game_running = false;
    module_solved = false;
    for (byte ii = 0; ii < 6; ii++) {
        wire_interacted[ii] = false;
    }
}

void KTOME_Wires::powerOn() {
    
}

void KTOME_Wires::generate() {

    byte wires_to_remove = random(3);

    for (byte ii = 0; ii < 6; ii++) {
        wire_colour[ii] = random(5)+1;
    }
    wire_numbers = 6;
    while (wire_numbers >= (6 - wires_to_remove)) {
        byte temp_selection = random(6);
        if (wire_colour[temp_selection] != 0) {
            wire_colour[temp_selection] = 0;
            wire_numbers--;
        }
    }

    String serial_msg = "Wire colours: ";
    String colour_initials[] = {"-", "B", "W", "Y", "R", "K"};
    for (byte ii = 0; ii < 6; ii++) {
        serial_msg = serial_msg + colour_initials[wire_colour[ii]] + " ";
    }
    DEBUG_PRINTLN(serial_msg);
}

bool KTOME_Wires::isManual() {
    return true;
}

String KTOME_Wires::getManual() {
    String manual_string = "c";
    for (byte ii = 0; ii < 6; ii++) {
        manual_string = manual_string + char(wire_colour[ii]+'0');
    }
    return manual_string;
}

byte KTOME_Wires::manualConfirm() {

    byte return_bool = 1;
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].hasChanged();
        DEBUG_PRINT(switches[ii].isPressed());
        DEBUG_PRINT(": ");

        if (switches[ii].isPressed()) { // Wire connected...
            wire_state[ii] = true;
            wire_interacted[ii] = false;
            if (wire_colour[ii]>0) { // Wire should exist
                DEBUG_PRINTLN("Wire correctly exists...");
            } else { // Wire shouldn't exist
                return_bool = 2;
                DEBUG_PRINTLN("Wire exists but shouldn't!");
            }
        } else { // No wire connected...
            wire_state[ii] = false;
            wire_interacted[ii] = true;
            if (wire_colour[ii]>0) { // Wire should exist
                return_bool = 2;
                DEBUG_PRINTLN("Wire doesn't exist but should!");
            } else { // Wire shouldn't exist
                DEBUG_PRINTLN("Wire correctly doesn't exist...");
            }
        }
    }
    return return_bool;
}

void KTOME_Wires::findSolution() {

    byte colour_counts[6] = {0, 0, 0, 0, 0, 0};
    byte wire_positions[6] = {0, 0, 0, 0, 0, 0};
    byte wire_pointer = 0;

    for (byte ii = 0; ii < 6; ii++) {
        colour_counts[wire_colour[ii]]++;
        if (wire_colour[ii]>0){
            wire_positions[wire_pointer] = ii;
            wire_pointer++;
        }
    }

    switch (wire_numbers) {
        case 3:
            if (colour_counts[4]==0) {
                correct_wire = wire_positions[1];
            } else if (wire_colour[wire_positions[2]]==2) {
                correct_wire = wire_positions[2];
            } else if (colour_counts[1]>1) {
                for (byte ii = 0; ii < 6; ii++) {
                    if (wire_colour[ii]==1) {
                        correct_wire = ii;
                    }
                }
            } else {
                correct_wire = wire_positions[2];
            }
            break;
        case 4:
            if (colour_counts[4]>1 && serial_odd) {
                for (byte ii = 0; ii < 6; ii++) {
                    if (wire_colour[ii]==4) {
                        correct_wire = ii;
                    }
                }
            } else if (wire_colour[wire_positions[3]]==3 && colour_counts[4]==0) {
                correct_wire = wire_positions[0];
            } else if (colour_counts[1]==1) {
                correct_wire = wire_positions[0];
            } else if (colour_counts[3]>1) {
                correct_wire = wire_positions[3];
            } else {
                correct_wire = wire_positions[1];
            }
            break;
        case 5:
            if (wire_colour[wire_positions[4]]==5 && serial_odd) {
                correct_wire = wire_positions[3];
            } else if (colour_counts[4]==1 && colour_counts[3]>1) {
                correct_wire = wire_positions[0];
            } else if (colour_counts[5]==0) {
                correct_wire = wire_positions[1];
            } else {
                correct_wire = wire_positions[0];
            }
            break;
        case 6:
            if (colour_counts[3]==0 && serial_odd) {
                correct_wire = wire_positions[2];
            } else if (colour_counts[3]==1 && colour_counts[2]>1) {
                correct_wire = wire_positions[3];
            } else if (colour_counts[4]==0) {
                correct_wire = wire_positions[5];
            } else {
                correct_wire = wire_positions[3];
            }
            break;
    }

    DEBUG_PRINT("Wire positions: ");
    for (byte ii = 0; ii < 6; ii++) {
        DEBUG_PRINT(wire_positions[ii]);
    }
    DEBUG_PRINTLN();
    DEBUG_PRINT("Colour counts: ");
    for (byte ii = 0; ii < 6; ii++) {
        DEBUG_PRINT(colour_counts[ii]);
    }
    DEBUG_PRINTLN();
    DEBUG_PRINT("Wire colours: ");
    for (byte ii = 0; ii < 6; ii++) {
        DEBUG_PRINT(wire_colour[ii]);
    }
    DEBUG_PRINTLN();

    String serial_msg = "Correct wire to cut: ";
    if (correct_wire == 0) {
        serial_msg = serial_msg + "1st";
    } else if (correct_wire == 1) {
        serial_msg = serial_msg + "2nd";
    } else if (correct_wire == 2) {
        serial_msg = serial_msg + "3rd";
    } else {
        serial_msg = serial_msg + char(correct_wire + 1 + '0') + "th";
    }
    serial_msg = serial_msg + " wire!";

    DEBUG_PRINTLN(serial_msg);
}

void KTOME_Wires::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    this->serial_vowels = vowels;
    this->serial_odd = odds;
    this->battery_number = batts;
    this->ind_car = cars;
    this->ind_frk = frks;
    this->port_s = serials;
    this->port_p = parallels;
    findSolution();
}

void KTOME_Wires::strikeUpdate(byte strikes) {

}

byte KTOME_Wires::timerUpdate(String timer_digits) {
    return 0;
}

void KTOME_Wires::gameStart() {
    game_running = true;
}

byte KTOME_Wires::inputCheck() { // Return 1 for solve, 2 for strike
    byte return_byte;
    return_byte = hasWireBeenCut();
    if (return_byte < 6) { // A wire has been cut...
        return_byte = logicCheck(return_byte);
        if (return_byte > 0) {
            return return_byte;
        }
    }
    return 0;
}

byte KTOME_Wires::hasWireBeenCut() {
    for (byte ii = 0; ii < 6; ii++) {
        if (switches[ii].hasChanged())
        {
            if (!switches[ii].isPressed() && !wire_interacted[ii]) {
                DEBUG_PRINTLN("Wire " + String(ii+1) + " cut!");
                wire_interacted[ii] = true;
                return ii;
            }
        }
    }
    return 6;
}

byte KTOME_Wires::logicCheck(byte wire_cut){
    if (wire_cut == correct_wire) { // Correct wire cut
        module_solved = true;
        return 2;
	} else { // Incorrect wire cut
        return 1;
    }
}

bool KTOME_Wires::isSolved() {
    return module_solved;
}

byte KTOME_Wires::update() {

}

void KTOME_Wires::explode() {
    game_running = false;
}

void KTOME_Wires::defuse() {
    game_running = false;
}

String KTOME_Wires::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Wires::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Wires::needsIsr() {
    return false;
}

void KTOME_Wires::isrHandler() {
    
}