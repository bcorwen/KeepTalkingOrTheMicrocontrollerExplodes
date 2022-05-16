#include <Arduino.h>
#include <KTOME_Button.h>

KTOME_Button::KTOME_Button() {

}

void KTOME_Button::start() {
  leds_button[0].init(pin_array[0],5000,0);
  leds_button[1].init(pin_array[1],5000,1);
  leds_button[2].init(pin_array[2],5000,2);
  switches.init(pin_array[3]);
  reset();
}

void KTOME_Button::reset() {
  for (byte ii = 0; ii < 3; ii++) {
    leds_button[ii].write(false);
  }
  module_defused = false;
  wait_for_timer = false;
  strip_on = false;
  stripOn(false);
}

String KTOME_Button::getManual() {
  String manual_string = "c";
  manual_string = manual_string + char(button_colour+'0') + char(button_label+'0');
  return manual_string;
}

void KTOME_Button::generate() {

  button_colour = random(4);
  button_label = random(4);

  Serial.print(F("Button colour: "));
  Serial.println(button_colours[button_colour]);
  Serial.print(F("Button label: "));
  Serial.println(button_labels[button_label]);

}

void KTOME_Button::findSolution(bool ind_frk, bool ind_car, byte bat_num) {

  if (button_colour == 0 && button_label == 0) {
    shortpushneeded = false;
  } else if (bat_num > 1 && button_label == 1) {
    shortpushneeded = true;
  } else if (ind_car == 1 && button_colour == 1) {
    shortpushneeded = false;
  } else if (bat_num > 2 && ind_frk == 1) {
    shortpushneeded = true;
  } else if (button_colour == 2) {
    shortpushneeded = false;
  } else if (button_colour == 3 && button_label == 2) {
    shortpushneeded = true;
  } else {
    shortpushneeded = false;
  }

  Serial.print("CAR: ");
  Serial.print(ind_car);
  Serial.print(" | FRK: ");
  Serial.print(ind_frk);
  Serial.print(" | Batts: ");
  Serial.println(bat_num);

  Serial.print("Button colour: ");
  Serial.print(button_colours[button_colour]);
  Serial.print(" | Button label: ");
  Serial.println(button_labels[button_label]);

  Serial.print(F("Short push?: "));
  Serial.println(shortpushneeded);

  strip_colour = random(4);
  Serial.print(F("Strip colour: "));
  Serial.println(button_colours[strip_colour]);

  Serial.print(F("Release on "));
  if (strip_colour == 0) {
    Serial.println("4.");
  } else if (strip_colour == 2) {
    Serial.println("5.");
  } else {
    Serial.println("1.");
  }
}

byte KTOME_Button::inputCheck() {
  byte return_byte;
  if (hasButtonBeenReleased()) {
    return_byte = logicCheck();
    if (return_byte > 0) {
      return return_byte;
    }
  }
  return 0;
}

bool KTOME_Button::hasButtonBeenReleased() {
  if (switches.hasChanged()) {
    if (switches.isPressed()) {
      wait_for_timer = false;
      isthisapress = true;
      Serial.println("Button pressed!");
      return false;
    } else {
      Serial.println("Button released!");
      strip_on = false;
      stripOn(false);
      return true;
    }
  } else {
    if ((switches.isPressed()) && ((switches.lastChange() + 1000) <= millis()) && (!strip_on)) {
      isthisapress = false;
      strip_on = true;
      stripOn(true);
    }
    return false;
  }
}

byte KTOME_Button::logicCheck(){
  if (isthisapress && shortpushneeded) { // Correct short press - win
		wait_for_timer = 0;
    return 2;
	} else if (!isthisapress && !shortpushneeded) { // Correct long hold - check time
    wait_for_timer = 1;
    return 1;
	} else { // Incorrect hold or press
    wait_for_timer = 0;
    return 3;
  }
}

void KTOME_Button::stripOn(bool strip_power){
  if (strip_power) {
    byte this_colour;
    if (shortpushneeded) {
      // Use random colour from button_colours
      this_colour = random(4);
    } else {
      // Use strip_colour
      this_colour = strip_colour;
    }
    Serial.print("Strip colour: ");
    Serial.println(this_colour);
    if (this_colour >= 1) { //Add red
      Serial.println("Adding red...");
      leds_button[0].setPulse(255,127,166,333,500,333);
    }
    if (this_colour == 1 || this_colour == 2) { //Add green
      Serial.println("Adding green...");
      leds_button[1].setPulse(153,76,166,333,500,333);
    }      
    if (this_colour == 0 || this_colour == 1) { //Add blue
      Serial.println("Adding blue...");
      leds_button[2].setPulse(153,76,166,333,500,333);
    }
  } else {
    // Turn lights off
    for (byte ii = 0; ii < 3; ii++){
      // leds_button[ii].setPulse(0,0,166,333,500,333);
      leds_button[ii].write(false);
    }
  }
}

byte KTOME_Button::timerDigits(String timer_digits) {
  if (wait_for_timer){
    if (strip_colour == 0) { // 4
      for (byte ii = 0; ii < 4; ii++){
        if (timer_digits.charAt(ii) == '4'){
          return 1;
        }
      }
    } else if (strip_colour == 2) { // 5
      for (byte ii = 0; ii < 4; ii++){
        if (timer_digits.charAt(ii) == '5'){
          return 1;
        }
      }
    } else { // 1
      for (byte ii = 0; ii < 4; ii++){
        if (timer_digits.charAt(ii) == '1'){
          return 1;
        }
      }
    }
    return 0;
  } else {
    // This should never be called!
    return 2;
  }
}

bool KTOME_Button::isDefused() {
  return module_defused;
}

void KTOME_Button::update() {
  for (byte ii = 0; ii < 3; ii++) {
    leds_button[ii].update();
  }
}

void KTOME_Button::explode() {
  leds_button[0].blink(true, 100, 1);
}