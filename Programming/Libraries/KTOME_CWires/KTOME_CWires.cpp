#include <Arduino.h>
#include <KTOME_CWires.h>

KTOME_CWires::KTOME_CWires() {

}

void KTOME_CWires::start() {
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].init(pin_array[ii]);
        leds[ii].init(pin_array[ii+6]);
    }
    reset();
}

void KTOME_CWires::reset() {
    module_defused = false;
    for (byte ii = 0; ii < 6; ii++) {
        leds[ii].write(false);
    }
}

String KTOME_CWires::getManual() {
    String manual_string = "c";
    for (byte ii = 0; ii < 6; ii++) {
        manual_string = manual_string + char(wire_properties[ii]+'0');
    }
    return manual_string;
}

void KTOME_CWires::generate() {

    Serial.println("Wire properties:");

    int8_t temp_i;
    temp_i = random(4);
    int8_t wires_to_remove = constrain(temp_i, 1, 3) - 1;
    // Serial.printf("Random = %d, Remove = %d\n", temp_i, wires_to_remove);

    // int8_t wires_to_remove = constrain(random(4), 1, 3) - 1;
    Serial.printf("Remove %d wires...\n", wires_to_remove);

    for (byte ii = 0; ii < 6; ii++) {
        wire_properties[ii] = random(2) + random(2)*2 + random(2)*4 + random(2)*8 + random(2)*16;
        if (wire_properties[ii] % 8 == 0) { // No colours set
            wire_properties[ii] += 1; // Add white
        } else if (wire_properties[ii] % 8 == 7) { // Three colours set
            wire_properties[ii] -= 1; // Remove white
        }
        wire_connected[ii] = true;
    }

    Serial.println("Initial wire set complete");

    wire_numbers = 6;
    while (wire_numbers > (6 - wires_to_remove)) {
        Serial.printf("wire_numbers (%d) > (6 - wires_to_remove (%d))\n", wire_numbers, wires_to_remove);
        byte temp_selection = random(6);
        if (wire_connected[temp_selection]) {
            wire_connected[temp_selection] = false;
            wire_properties[temp_selection]+= 32;
            wire_numbers--;
        }
        delay(200);
    }
    Serial.println("Removal of wires complete");

    // Make sure one wire needs to be cut
    byte wire_to_ensure = random(wire_numbers);
    byte wire_counter = 0;
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_connected[ii]) {
            if (wire_to_ensure == wire_counter) { // Replace this wire
                byte correct_lottery[4] = {1, 17, 18, 19};
                byte swapper = random(4);
                Serial.printf("Ensuring one wire needs cut: %d from %d -> %d\n", ii+1, wire_properties[ii], correct_lottery[swapper]);
                wire_properties[ii] = correct_lottery[swapper];

            }
            wire_counter++;
        }
    }

    String serial_msg = "";
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_properties[ii] < 10) {
            serial_msg = serial_msg + '0';
        }
        serial_msg = serial_msg + String(wire_properties[ii]);

        if (ii < 5) {
            serial_msg = serial_msg + " | ";
        }
    }
    Serial.println(serial_msg);
    serial_msg = "";
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_properties[ii] % 16 >= 8) {
            serial_msg = serial_msg + "O ";
        } else {
            serial_msg = serial_msg + ". ";
        }
        if (ii < 5) {
            serial_msg = serial_msg + " | ";
        }
    }
    Serial.println(serial_msg);
    serial_msg = "";
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_properties[ii] >= 32) {
            serial_msg = serial_msg + "- ";
        } else {
            if (wire_properties[ii] % 2 >= 1) {
                serial_msg = serial_msg + "W";
            }
            if (wire_properties[ii] % 4 >= 2) {
                serial_msg = serial_msg + "R";
            }
            if (wire_properties[ii] % 8 >= 4) {
                serial_msg = serial_msg + "B";
            }
            if (wire_properties[ii] % 8 == 1 || wire_properties[ii] % 8 == 2 || wire_properties[ii] % 8 == 4) {
                serial_msg = serial_msg + " ";
            }
        }
        if (ii < 5) {
            serial_msg = serial_msg + " | ";
        }
    }
    Serial.println(serial_msg);
    serial_msg = "";
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_properties[ii] % 32 >= 16) {
            serial_msg = serial_msg + "* ";
        } else {
            serial_msg = serial_msg + "  ";
        }
        if (ii < 5) {
            serial_msg = serial_msg + " | ";
        }
    }
    Serial.println(serial_msg);
    Serial.println("Game generated");

}

void KTOME_CWires::begin() {
    for (byte ii = 0; ii < 6; ii++) {
        if (wire_properties[ii] % 16 >= 8) {
            leds[ii].write(true);
        } else {
            leds[ii].write(false);
        }
    }
}

void KTOME_CWires::widgetSolution(bool parallel_port, byte battery_number, bool serial_even) {
    this->parallel_port = parallel_port;
    this->battery_number = battery_number;
    this->serial_even = serial_even;
    findSolution();
}

void KTOME_CWires::findSolution() {

    byte action_to_take[5] = {1, 0, 0, 0, 0};
    if (serial_even){
        action_to_take[2] = 1;
    }
    if (parallel_port){
        action_to_take[3] = 1;
    }
    if (battery_number > 1){
        action_to_take[4] = 1;
    }

    //                       W, R, B, RB,
    byte solution_lut[16] = {0, 2, 2, 2,   /* X . */
                             1, 4, 3, 2,   /* O . */
                             0, 0, 3, 3,   /* X * */
                             4, 4, 3, 1};   /* O * */

    Serial.print("Correct wires to cut: ");
    for (byte ii = 0; ii < 6; ii++) {
        correct_wires[ii] = action_to_take[solution_lut[(wire_properties[ii]%32)/2]];
        if (correct_wires[ii] == 1) {
            Serial.print("X ");
        } else {
            Serial.print("| ");
        }
    }
    Serial.println("");

}

bool KTOME_CWires::manualConfirm() {

    bool return_bool = true;
    for (byte ii = 0; ii < 6; ii++) {
        switches[ii].hasChanged();
        Serial.print(switches[ii].isPressed());

        if (switches[ii].isPressed()) { // Wire connected...
            if (wire_connected[ii] == 1) { // Wire should exist
                Serial.println("Wire correctly exists...");
            } else { // Wire shouldn't exist
                return_bool = false;
                Serial.println("Wire exists but doesn't!");
            }
        } else { // No wire connected...
            if (wire_connected[ii] == 1) { // Wire should exist
                return_bool = false;
                Serial.println("Wire doesn't exist but should!");
            } else { // Wire shouldn't exist
                Serial.println("Wire correctly doesn't exist...");
            }
        }
    }
    return return_bool;
}

int8_t KTOME_CWires::inputCheck() { // Return 1 for solve, 2 for strike
    byte return_byte;
    return_byte = hasWireBeenCut();
    if (return_byte < 6) { // A wire has been cut...
        return_byte = logicCheck(return_byte);
        if (return_byte == 1) {

            return return_byte;
        } else if (return_byte == 2) {
            return return_byte;
        }
    }
    return 0;
}

int8_t KTOME_CWires::hasWireBeenCut() {
    for (byte ii = 0; ii < 6; ii++) {
        if (switches[ii].hasChanged())
        {
            if (!switches[ii].isPressed() && wire_connected[ii]) {
                Serial.printf("Wire %d cut!\n", ii+1);
                wire_connected[ii] = false;
                return ii;
            }
        }
    }
    return 6;
}

int8_t KTOME_CWires::logicCheck(byte wire_cut){
    if (correct_wires[wire_cut] == 1) { // Correct wire cut
        bool all_done = true;
        for (byte ii = 0; ii < 6; ii++){
            if (correct_wires[ii] && (wire_connected[ii] == correct_wires[ii])) {
                all_done = false;
                Serial.print("N");
            } else {
                Serial.print("Y");
            }
        }
        Serial.println("");
        if (all_done) {
            return 1;
        }
	} else { // Incorrect wire cut
        return 2;
    }
}

bool KTOME_CWires::isDefused() {
    return module_defused;
}

// void KTOME_CWires::update() {
    
// }

void KTOME_CWires::explode() {
    for (byte ii = 0; ii < 6; ii++) {
        leds[ii].write(false);
    }
}