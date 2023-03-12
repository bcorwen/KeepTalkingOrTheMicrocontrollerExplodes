# SevSegShift

This library adds the functionality of a Shift Register to the original [SevSeg library  by Dean Reading][1]. Most functionality is the same as the original library. You can even use the basic class from this library. It only inherits the class and adds the Shift Register functionality where required.

### Shift Register usage on 7-Segment-Displays
Advantage of the use with Shift Registers is the limited usage of Arduino PINs. You will only require 3 Arduino PINs (and 5V & Gnd). Everything else will be done on the Shift Registers. 
An Implementation without Shift Registers requires i.e. on a 4-digit 7 Segment Display 12 PINs to be connected to the Arduino.
Additionally there is also the option to use only one Shift Register for the Segments. The digit PINs are connected to the Arduino directly. This way you require the 3 Arduino PINs (and 5V & Gnd) plus the digit pins of the 7-Segment Display. This results in 7 PINs instead of 12 for a 4-digit 7 Segment Display.

### SevSeg support
In addition to the Shift Registers this library supports everything the original library supports. 
It supports common cathode and common anode displays, and the use of switching transistors. Numbers can be displayed in decimal or hexadecimal representation, with decimal places. Characters can be displayed (as accurately as possible). It also supports multiple displays, of varying dimensions. 

[Download it from GitHub][5].

[original library here][1].

#### Note on shift registers
It's often preferred to drive seven segment displays through shift register chips, as that only uses ~3 micrcontroller pins instead of ~12 pins. This library does not support shift registers. However, there's a mostly-compatible branch that does support shift registers.
See [bridystone's SevSegShift][5].

## Hardware

### Seven segment display pins

Your display should have:
*   **Digit Pins** \- One for each digit. These are the 'common pins'. They will be cathodes (negative pins) for common cathode displays, or anodes (positive pins) for common anode displays.
*   **8 Segment Pins** \- One for each of the seven segments plus the decimal point.

### Shift Registers
For a 4-Digit-Display with 12 PINS, you should use:
*   **2 Shift Registers** \- to be used with the 12 necessary connections pins of the 7 Segment display
or (if digit PINs are connected to the Arduino directly)
*   **1 Shift Registers** \- to be used with the 8 necessary connections pins of the 7 Segment display


### Arduino connections

For the *Shift Register configuration* 5V, GND and 3 PINs of Arduino are connected to the Shift Registers. The PINs of the Shift Registers are connected to the 12 PINs of the Seven Segment Display.

see [The SevSegShift_Counter Example][6].

For the *Hybrid Shift Register configuration* 5V, GND and 3 PINs of Arduino are connected to the Shift Registers. The PINs of the Shift Registers are connected to the 8 Segment-PINs of the Seven Segment Display. The Digit-PINs are connected to the Arduino PINs.

see [The SevSegShiftOne_Counter Example][7].


For the *normal configuration* (SevSeg without Shift):
All digit pins and segment pins can be connected to any of the Arduino's digital pins, or analog pins with digital support; just make sure you take note of your connections! Analog pins on most Arduinos have digital support, but the Arduino Nano is an exception.


### Current-limiting resistors

Don't forget that the display uses LEDs, so you should use current-limiting resistors in series with the *digit pins*. 330 ohms is a safe value if you're unsure. If you use current-limiting resistors on the *segment pins* instead, then set resistorsOnSegments to true (see the example SevSeg_Counter.ino).


### Hardware configuration

You have to specify your hardware configuration to the library as the first argument in sevseg.begin. The options are detailed below.

#### Simple, low power displays
These displays are powered directly through the Arduino output pins.
*   **COMMON_CATHODE** \- For common cathode displays without switches. These displays require a low voltage at the digit pin to illuminate the digit.
*   **COMMON_ANODE** \- For common anode displays without switches. These displays require a high voltage at the digit pin to illuminate the digit.

#### Displays with switches
Some displays (mostly bigger ones) use switching transistors, but most people won't have to worry about the configurations below.
*   **N_TRANSISTORS** \- If you use N-type transistors to sink current (or any other active-high, low-side switches).
*   **P_TRANSISTORS** \- If you use P-type transistors to supply current (or any other active-low, high-side switches).
*   **NP_COMMMON_CATHODE** \- If your setup uses N-type AND P-type transistors with a common cathode display.
*   **NP_COMMMON_ANODE** \- If your setup uses N-type AND P-type transistors with a common anode display.
Note that use of active-high, high-side switches will have no impact on the configuration chosen. There are usually called high-side switches.


### Example pinout

In the below pinout, digits are numbered 1, 2, 3, 4.
Segments are numbered A through G plus Decimal Point (DP), according to [this picture][2].
Pins are ordered as looking at the front of the display.

[Cheap, 4-digit, 12-pin display from Ebay][3] (labelled HS410561k-32 on bottom edge):
4-digit common anode display, with 2 rows of 6 pins.
```
Top Row:    1 A F  2 3 B
Bottom Row: E D DP C G 4
```

#### Configuration layout with shift registers
Example diagram for 2 Shift Register usage see [The SevSegShift_Counter Example][6].

Example diagram for 1 Shift Register usage (Hybrid confihuration) see [The SevSegShiftOne_Counter Example][7].

## Software

To install, copy the SevSegShift folder into your arduino sketchbook\-libraries folder. More detailed instructions are [here][4].
The Library Manager can be used from arduino version 1.6.2.


### Setting up (2 Shift Registers)

```c++
#include "SevSegShift.h"

#define SHIFT_PIN_DS   13 /* Data input PIN */
#define SHIFT_PIN_STCP 12 /* Shift Register Storage PIN */
#define SHIFT_PIN_SHCP 11 /* Shift Register Shift PIN */

SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP); //Instantiate a seven segment controller object (with Shift Register functionality)

/* instance without Shift registers would be:
SevSeg sevseg;
*/

void setup() {
  byte numDigits = 4;
  byte digitPins[] = {8+2, 8+5, 8+6, 2}; // of ShiftRegister(s) | 8+x (2nd Register)
  byte segmentPins[] = {8+3, 8+7, 4, 6, 7, 8+4, 3,  5}; // of Shiftregister(s) | 8+x (2nd Register)
  /* configuration without ShiftRegister - Direct arduino connection
  byte digitPins[] = {2, 3, 4, 5}; // PINs of Arduino
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13}; // PINs of Arduino
  */
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);
  ...
}
```
* `SHIFT_PIN_DS` defines the PIN the DS-PIN of the 1st Shift Register is connected to
* `SHIFT_PIN_SHCP` defines the PIN the SH_CP PIN of both Shift Registers are conncted to
* `SHIFT_PIN_STCP` defines the PIN the ST_CP PIN of both Shift Registers are conncted to
* `digitPins` is an array that stores the Shift Register pin numbers that the digits (D1, D2, D3, D4) are connected to. Order them from left to right.
* `segmentPins` is an array that stores the Shift Register pin numbers that the segments (A, B, C, D, E, F, G, DP) are connected to. Order them from segment a to g, then the decimal place (if it's connected).

### Setting up Hybird (1 Shift Registers)

```c++
#include "SevSegShift.h"

#define SHIFT_PIN_DS   8 /* Data input PIN */
#define SHIFT_PIN_STCP 7 /* Shift Register Storage PIN */
#define SHIFT_PIN_SHCP 6 /* Shift Register Shift PIN */

//Instantiate a seven segment controller object (with Shift Register functionality)
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
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);
  ...
}
```
* `SHIFT_PIN_DS` defines the PIN the DS-PIN of the 1st Shift Register is connected to
* `SHIFT_PIN_SHCP` defines the PIN the SH_CP PIN of both Shift Registers are conncted to
* `SHIFT_PIN_STCP` defines the PIN the ST_CP PIN of both Shift Registers are conncted to
* `digitPins` is an array that stores the Arduino pin numbers that the digits (D1, D2, D3, D4) are connected to. Order them from left to right.
* `segmentPins` is an array that stores the Shift Register pin numbers that the segments (A, B, C, D, E, F, G, DP) are connected to. Order them from segment a to g, then the decimal place (if it's connected).

If you wish to use more than 8 digits, increase MAXNUMDIGITS in SevSeg.h.


### Setting a number
#### Integer
```c++
sevseg.setNumber(3141,3); // Displays '3.141'
```
The first argument is the number to display. The second argument indicates where the decimal place should be, counted from the least significant digit. E.g. to display an integer, the second argument is 0.

#### Floating point
```c++
sevseg.setNumberF(3.14159f,3); //Displays '3.141'
```
Floats are supported. In this case, the second argument indicates how many decimal places of precision you want to display.

Note that:

 - Out of range numbers are shown as '----'. 
 - If the second argument is -1 or omitted, there will be no decimal place. 
 - Enter 'true' as the third argument to display the number in hexadecimal representation (instead of decimal)

### Setting a character string

```c++
sevseg.setChars("abcd");
```

Character arrays can be displayed - as accurately as possible on a seven segment display. See SevSeg.cpp digitCodeMap[] to notes on each character. Only alphanumeric characters, plus ' ', '-' and '.' are supported. The character array should be NULL terminated.


### Refreshing the display

```c++
sevseg.refreshDisplay();
```

Your program must run the refreshDisplay() function repeatedly to display the number. Note that any delays introduced by other functions will produce undesirable effects on the display.

To blank the display, call:

```c++
sevseg.blank();
```


### Setting the brightness

```c++
sevseg.setBrightness(90);
```

The brightness can be adjusted using a value between -200 and 200. 0 to 100 is the standard range.
Numbers greater than 100 and less than -100 may cause noticeable flickering.
Note that a 0 does not correspond to no brightness - nor does -200. If your display has noticeable flickering, modifying the brightness towards 0 may correct it.
Results will vary for each implementation. The brightness seen depends on the display characteristics, the arduino model driving it, the resistors used, and the amount of time spent doing other things in the program.

## License

MIT License

Copyright (c) 2020 Dean Reading, 
Copyright (c) 2020 Jens Breidenstein (SevSegShift part)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

[1]: https://github.com/DeanIsMe/SevSeg
[2]: https://en.wikipedia.org/wiki/File:7_segment_display_labeled.svg
[3]: http://www.ebay.com/sch/i.html?LH_BIN=1&_from=R40&_sacat=0&_nkw=7+segment+display+4+digit+2+pcs&_sop=15
[4]: http://arduino.cc/en/Guide/Libraries
[5]: https://github.com/bridystone/SevSegShift
[6]: https://github.com/bridystone/SevSegShift/blob/ShiftRegister/examples/SevSegShift_Counter/SevSegShift.png
[7]: https://github.com/bridystone/SevSegShift/blob/ShiftRegister/examples/SevSegShiftOne_Counter/SevSegShiftOne.png
