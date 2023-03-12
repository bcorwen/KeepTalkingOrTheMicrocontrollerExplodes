/* SevSegShift Counter Example
 
 Copyright 2020 Dean Reading,
 Copyright 2020 Jens Breidenstein
 
 This example demonstrates a very simple use of the SevSegShift library with a 4
 digit display. It displays a counter that counts up, showing deci-seconds.
 */

#include "SevSegShift.h"

#define SHIFT_PIN_SHCP 6
#define SHIFT_PIN_STCP 7
#define SHIFT_PIN_DS   8


/* Instantiate a seven segment controller object with:
  - segment pins controlled via 1 shift register and 
  - digit pins connected to the Arduino directly
  */
SevSegShift sevsegshift(
                  SHIFT_PIN_DS, 
                  SHIFT_PIN_SHCP, 
                  SHIFT_PIN_STCP, 
                  1, /* number of shift registers there is only 1 Shiftregister 
                        used for all Segments (digits are on Controller)
                        default value = 2 (see SevSegShift example)
                        */
                  true /* Digits are connected to Arduino directly 
                          default value = false (see SevSegShift example)
                        */
                );

void setup() {
  byte numDigits = 4;
  byte digitPins[] = {5, 4, 3, 2}; // These are the PINS of the ** Arduino **
  byte segmentPins[] = {0, 2, 4, 6, 7, 1, 3, 5}; // these are the PINs of the ** Shift register **
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);
}

void loop() {
  static unsigned long timer = millis();
  static int deciSeconds = 0;
  
  if (millis() - timer >= 100) {
    timer += 100;
    deciSeconds++; // 100 milliSeconds is equal to 1 deciSecond
    
    if (deciSeconds == 10000) { // Reset to 0 after counting for 1000 seconds.
      deciSeconds=0;
    }
    sevseg.setNumber(deciSeconds, 1);
  }

  sevseg.refreshDisplay(); // Must run repeatedly
}

/// END ///
