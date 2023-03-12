#include <Arduino.h>
#include <KTOME_Keypad.h>

KTOME_Keypad::KTOME_Keypad() {

}

void KTOME_Keypad::start() {
  for (byte ii = 0; ii < 4; ii++) {
    leds_red[ii].init(pin_array[ii]);
    leds_green[ii].init(pin_array[ii + 4]);
    switches[ii].init(pin_array[ii + 8]);
  }
  reset();
}

void KTOME_Keypad::reset() {
  for (byte ii = 0; ii < 4; ii++) {
    leds_red[ii].write(false);
    leds_green[ii].write(false);
  }
  stage = 0;
}

String KTOME_Keypad::getManual() {
  String manual_string = "c";
  for (byte ii = 0; ii < 4; ii++) {
    manual_string = manual_string + char(column_symbols[selected_symbols[ii]]+'0');
  }
  return manual_string;
}

void KTOME_Keypad::generate() {

  byte col_select = B00000000;
  byte symbols_picked = 0;
  byte this_pick;
  byte keypad_column_choice;

  keypad_column_choice = random(6);
  Serial.print(F("Keypad column: "));
  Serial.println(keypad_column_choice);

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
  Serial.print(selected_symbols[0]);
  Serial.print(" ");
  Serial.print(selected_symbols[1]);
  Serial.print(" ");
  Serial.print(selected_symbols[2]);
  Serial.print(" ");
  Serial.println(selected_symbols[3]);

  Serial.print(column_symbols[selected_symbols[0]]);
  Serial.print(" ");
  Serial.print(column_symbols[selected_symbols[1]]);
  Serial.print(" ");
  Serial.print(column_symbols[selected_symbols[2]]);
  Serial.print(" ");
  Serial.println(column_symbols[selected_symbols[3]]);

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

  // char CAN_message[2];
  // CAN_message[0] = 'i';
  // CAN_message[1] = '\0';
  // leds[0].write(true);
  // ktomeCAN.send(can_ids.Master | CAN_ID, CAN_message, 1);
  // leds[0].write(false);
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
  //  Serial.println(col_select, BIN);
  //  Serial.println(looking_at_symbol);
  return results;
}

byte KTOME_Keypad::inputCheck() {
  byte return_byte;
  byte result_byte = 0;
  for (byte ii = 0; ii < 4; ii++) {
    if (hasButtonBeenPushed(ii)) {

      // Serial.print("Current stage: ");
      // Serial.print(stage);
      // Serial.print(" | order_symbols[button number]: ");
      // Serial.println(order_symbols[ii]);

      return_byte = logicCheck(ii);
      if (return_byte > 0) {
        return return_byte;
      }
    }
  }
  return return_byte;
}

bool KTOME_Keypad::hasButtonBeenPushed(byte button_number) {
  if (switches[button_number].hasChanged()) {
    if (switches[button_number].isPressed()) {
      byte func_return;
      Serial.print(button_number);
      Serial.println(" button pressed!");
      return true;
    } else {
      Serial.print(button_number);
      Serial.println(" button released!");
      return false;
    }
  } else {
    return false;
  }
}

byte KTOME_Keypad::logicCheck(byte button_number){
  if (order_symbols[button_number] == stage) { // Correct key pressed
		stage++;
		leds_green[button_number].write(true);
    return 2;
	} else if (order_symbols[button_number] > stage) { // Wrong key pressed
    leds_red[button_number].blink(true, 500, 10, 1);
    return 1;
	} else {
    return 0;
  }
}

byte KTOME_Keypad::getStage() {
  return stage;
}

bool KTOME_Keypad::isDefused() {
	if (stage == 4) {
		return true;
	} else {
		return false;
	}
}

void KTOME_Keypad::update() {
  for (byte ii = 0; ii < 4; ii++) {
    leds_red[ii].update();
  }
}

void KTOME_Keypad::explode() {
  for (byte ii = 0; ii < 4; ii++) {
    leds_red[ii].blink(true, 100, 1);
    leds_green[ii].write(false);
  }
}