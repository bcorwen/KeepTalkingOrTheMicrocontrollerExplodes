#include <Arduino.h>
#include <KTOME_Wires.h>

KTOME_Wires::KTOME_Wires() {

}

void KTOME_Wires::start() {
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].init(pin_array[ii]);
    }
    reset();
}

void KTOME_Wires::reset() {
    module_defused = false;
    for (byte ii = 0; ii < 6; ii++) {
        wire_interacted[ii] = false;
    }
}

String KTOME_Wires::getManual() {
    String manual_string = "c";
    for (byte ii = 0; ii < 6; ii++) {
        manual_string = manual_string + char(wire_colour[ii]+'0');
    }
    return manual_string;
}

void KTOME_Wires::generate() {

    String serial_msg = "Wire colours: ";

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

    String colour_initials[] = {"-", "B", "W", "Y", "R", "K"};
    for (byte ii = 0; ii < 6; ii++) {
        serial_msg = serial_msg + colour_initials[wire_colour[ii]] + " ";
    }
    
    Serial.println(serial_msg);

}

void KTOME_Wires::findSolution(bool serial_odd) {

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

    Serial.print("Wire positions: ");
    for (byte ii = 0; ii < 6; ii++) {
        Serial.print(wire_positions[ii]);
    }
    Serial.println();
    Serial.print("Colour counts: ");
    for (byte ii = 0; ii < 6; ii++) {
        Serial.print(colour_counts[ii]);
    }
    Serial.println();
    Serial.print("Wire colours: ");
    for (byte ii = 0; ii < 6; ii++) {
        Serial.print(wire_colour[ii]);
    }
    Serial.println();

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

    Serial.println(serial_msg);
}

bool KTOME_Wires::manualConfirm() {

    bool return_bool = true;
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].hasChanged();
        Serial.print(switches[ii].isPressed());

        if (switches[ii].isPressed()) { // Wire connected...
            wire_state[ii] = true;
            wire_interacted[ii] = false;
            if (wire_colour[ii]>0) { // Wire should exist
                Serial.println("Wire correctly exists...");
            } else { // Wire shouldn't exist
                return_bool = false;
                Serial.println("Wire exists but doesn't!");
            }
        } else { // No wire connected...
            wire_state[ii] = false;
            wire_interacted[ii] = true;
            if (wire_colour[ii]>0) { // Wire should exist
                return_bool = false;
                Serial.println("Wire doesn't exist but should!");
            } else { // Wire shouldn't exist
                Serial.println("Wire correctly doesn't exist...");
            }
        }
    }
    return return_bool;
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
                Serial.println("Wire " + String(ii+1) + " cut!");
                wire_interacted[ii] = true;
                return ii;
            }
        }
    }
    return 6;
}

byte KTOME_Wires::logicCheck(byte wire_cut){
    if (wire_cut == correct_wire) { // Correct wire cut
        return 1;
	} else { // Incorrect wire cut
        return 2;
    }
}

bool KTOME_Wires::isDefused() {
    return module_defused;
}

// void KTOME_Wires::update() {
    
// }

void KTOME_Wires::explode() {
    
}