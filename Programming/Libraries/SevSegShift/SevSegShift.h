/* SevSegShift Library
 *
 * Copyright 2020 Jens Breidenstein
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * This library enhances the functionality of SevSeg with the possibility 
 * to use it with Shift Registers. 
 * All original functionality of SevSeg is transparent and could be used further
 *
 * See the included readme for instructions.
 * https://github.com/bridystone/SevSegShift
 */
#pragma once

#include <SevSeg.h>

class SevSegShift: public SevSeg {
public:
  /* Constructor
    pinDS => Data Select PIN
    pinSHCP => pin for ShiftClock
    pinSTCP => pin for StoreClock
    registerCount => number of used shift registers (currently set to 2 - later maybe differently)*/
  SevSegShift(
    byte pinDS, 
    byte pinSHCP, 
    byte pinSTCP,
    const byte numberOfShiftRegisters = 2, // currently const value (not changeable) - maybe in future
    bool isDigitsOnController=false // only Segments are on ShiftRegister - DigitPins are connected to controller
  );

  /*
    prepare the SevenSeg-Shift-Register lib
    hardwareConfig -> ANODE or CATHODE SevensSegment
    numDigits -> number of digitSegments
    byte shiftRegisterMapDigits[], -> pins are here the OUTPUT PINs of the Shift Register
    byte shiftRegisterMapSegments[], -> pins are here the OUTPUT PINs of the Shift Register
    resOnSegments -> Resistors on Segments instead of on LED (as it should be)
    upDataWithDelays -> ?
    leadingZeros -> use leading zeros
    disableDecPoint -> is the Decimal point *not available? */
  void begin(byte hardwareConfig, 
    byte numDigitsIn,
    byte shiftRegisterMapDigits[],
    byte shiftRegisterMapSegments[],
    bool resOnSegmentsIn=false, 
    bool updateWithDelaysIn=false, 
    bool leadingZerosIn=false,
    bool disableDecPoint=false
  );
protected:
  virtual void segmentOn(byte segmentNum);
  virtual void segmentOff(byte segmentNum);
  virtual void digitOn(byte digitNum);
  virtual void digitOff(byte digitNum);

  /*
    pushing the data in _shiftRegisterMap to all the Shift Registers
    */
  void pushData2ShiftRegister();

private:
  byte _pinDS;
  byte _pinSHCP;
  byte _pinSTCP;
  bool _isDigitsOnController;

  // which ports of the Shift Registers are used for which purpose
  byte *_shiftRegisterMapDigits;
  byte *_shiftRegisterMapSegments;

  // store number of ShiftRegisters
  byte _numberOfShiftRegisters;
  // current shift register map (to be used to cycle through when pushing data to the register)
  byte *_shiftRegisterMap;
};