//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 19/06/22
//======================================================================
//
//  Module: Morse (Slave, Solvable, Vanilla)
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
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <AccelStepper.h>
#include <KTOME_Morse.h>
#include <Munro_31.h>

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

KTOME_Morse::KTOME_Morse() {
    // u8g2 = new U8G2_ST7920_128X64_1_SW_SPI(U8G2_R0, /* clock=*/ 19, /* data=*/ 21, /* CS=*/ 22, /* reset=*/ 8);
    // U8G2_ST7920_128X64_1_SW_SPI u8g2 (U8G2_R0, /* clock=*/ 19, /* data=*/ 21, /* CS=*/ 22, /* reset=*/ 8);
}

void KTOME_Morse::start(FLedPWM *fleds) {
    fleds_addr = fleds; // Store pointer to leds
    switches[0].init(MORSE_ENS);
    switches[1].init(STEPPER_CALIBR);
    ESP32Encoder::useInternalWeakPullResistors=UP;
    encoder.attachHalfQuad(MORSE_ENA, MORSE_ENB);
    encoder.setCount(0);
    encoder_tracker = 0;
    u8g2.begin();
    u8g2.setFont(Munro_31);
    reset();
}

void KTOME_Morse::reset() {
    fleds_addr->write(1, CRGB::Black);
    strike_count = 0;
    game_running = false;
    module_solved = false;
    clearScreen();
}

void KTOME_Morse::powerOn() {
    fleds_addr->blink(1, CRGB::Orange, CRGB::Black, 150, 100, 10);
    calibrateStepper();
}

void KTOME_Morse::calibrateStepper() {
    stepper.setMaxSpeed(stepperSpeedMax);
    stepper.moveTo(-9000);
    stepper.setSpeed(stepperSpeed);
    DEBUG_PRINTLN("Finding calibration point...");
    do {
        if (!stepper_calibrated && switches[1].hasChanged()) {
            if (switches[1].isPressed()) {
                // This calibration button has just been triggered
                DEBUG_PRINTLN("Calibration point found!");
                stepper.setCurrentPosition(-100);
                stepper.moveTo(150);
                stepper.setSpeed(stepperSpeed);
                stepper_calibrated = true;
            }
        }
        stepper.runSpeedToPosition();
    } while (!stepper_calibrated);
    DEBUG_PRINTLN("Calibration ending");
}

void KTOME_Morse::generate() {
    selected_word = random(16);
    DEBUG_PRINT("The morse code is: ");
    DEBUG_PRINTLN(word_list[selected_word]);
}

bool KTOME_Morse::isManual() { // Required manual setup
    return false;
}

String KTOME_Morse::getManual() {
    DEBUG_PRINTLN("ERROR: Should not have been asked to fetch manual setup!");
    return "";
}

byte KTOME_Morse::manualConfirm() { // No further confirm needed for this module, return 0
    return 0;
}

void KTOME_Morse::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    
}

void KTOME_Morse::strikeUpdate(byte strikes) {

}

byte KTOME_Morse::timerUpdate(String timer_digits) {
    
}

void KTOME_Morse::gameStart() {
    DEBUG_PRINTLN("Turn on the screen...");
    game_running = true;
    encoder_tracker = encoder.getCount();
    drawScreen();
}

void KTOME_Morse::blinkLetter() {
    uint32_t this_time = millis();
    byte letter_code[5] = {0,0,0,0,0};
    byte letter_length;
    if (this_time >= morse_timer) {
        switch (word_list[selected_word].charAt(morse_letter)) {
            case 'a':
                // byte letter_code[] = {0,1};
                letter_code[1] = 1;
                letter_length = 2;
            break;
            case 'b':
                // byte letter_code[] = {1,0,0,0};
                letter_code[0] = 1;
                letter_length = 4;
            break;
            case 'c':
                // byte letter_code[] = {1,0,1,0};
                letter_code[0] = 1;
                letter_code[2] = 1;
                letter_length = 4;
            break;
            case 'd':
                // byte letter_code[] = {1,0,0};
                letter_code[0] = 1;
                letter_length = 3;
            break;
            case 'e':
                // byte letter_code[] = {0};
                letter_length = 1;
            break;
            case 'f':
                // byte letter_code[] = {0,0,1,0};
                letter_code[2] = 1;
                letter_length = 4;
            break;
            case 'g':
                // byte letter_code[] = {1,1,0};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_length = 3;
            break;
            case 'h':
                // byte letter_code[] = {0,0,0,0};
                letter_length = 4;
            break;
            case 'i':
                // byte letter_code[] = {0,0};
                letter_length = 2;
            break;
            case 'j':
                // byte letter_code[] = {0,1,1,1};
                letter_code[1] = 1;
                letter_code[2] = 1;
                letter_code[3] = 1;
                letter_length = 4;
            break;
            case 'k':
                // byte letter_code[] = {1,0,1};
                letter_code[0] = 1;
                letter_code[2] = 1;
                letter_length = 3;
            break;
            case 'l':
                // byte letter_code[] = {0,1,0,0};
                letter_code[1] = 1;
                letter_length = 4;
            break;
            case 'm':
                // byte letter_code[] = {1,1};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_length = 2;
            break;
            case 'n':
                // byte letter_code[] = {1,0};
                letter_code[0] = 1;
                letter_length = 2;
            break;
            case 'o':
                // byte letter_code[] = {1,1,1};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_code[2] = 1;
                letter_length = 3;
            break;
            case 'p':
                // byte letter_code[] = {0,1,1,0};
                letter_code[1] = 1;
                letter_code[2] = 1;
                letter_length = 4;
            break;
            case 'q':
                // byte letter_code[] = {1,1,0,1};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_code[3] = 1;
                letter_length = 4;
            break;
            case 'r':
                // byte letter_code[] = {0,1,0};
                letter_code[1] = 1;
                letter_length = 3;
            break;
            case 's':
                // byte letter_code[] = {0,0,0};
                letter_length = 3;
            break;
            case 't':
                // byte letter_code[] = {1};
                letter_code[0] = 1;
                letter_length = 1;
            break;
            case 'u':
                // byte letter_code[] = {0,0,1};
                letter_code[2] = 1;
                letter_length = 3;
            break;
            case 'v':
                // byte letter_code[] = {0,0,0,1};
                letter_code[3] = 1;
                letter_length = 4;
            break;
            case 'w':
                // byte letter_code[] = {0,1,1};
                letter_code[1] = 1;
                letter_code[2] = 1;
                letter_length = 3;
            break;
            case 'x':
                // byte letter_code[] = {1,0,0,1};
                letter_code[0] = 1;
                letter_code[3] = 1;
                letter_length = 4;
            break;
            case 'y':
                // byte letter_code[] = {1,0,1,1};
                letter_code[0] = 1;
                letter_code[2] = 1;
                letter_code[3] = 1;
                letter_length = 4;
            break;
            case 'z':
                // byte letter_code[] = {1,1,0,0};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_length = 4;
            break;
            default:
                // byte letter_code[] = {1,1,1,1,1};
                letter_code[0] = 1;
                letter_code[1] = 1;
                letter_code[2] = 1;
                letter_code[3] = 1;
                letter_code[4] = 1;
                letter_length = 5;
            break;
        }

        // leds.blink(true, morse_timings[letter_code[morse_dot_dah]], 1);
        fleds_addr->blink(1, CRGB::Orange, CRGB::Black, morse_timings[letter_code[morse_dot_dah]], 2);
        morse_timer = this_time + morse_timings[letter_code[morse_dot_dah]] + morse_timings[0];
        DEBUG_PRINT("Bilnk of ");
        DEBUG_PRINT(morse_timings[letter_code[morse_dot_dah]]);        
        DEBUG_PRINT(" | Timer set to ");
        DEBUG_PRINT(morse_timer);
        morse_dot_dah ++;

        if (morse_dot_dah >= letter_length) {
            morse_dot_dah = 0;
            morse_letter ++;
            morse_timer += morse_timings[2];
        }
        if (morse_letter > (word_list[selected_word].length()-1)) {
            morse_letter = 0;
            morse_timer += morse_timings[3];
        }

        DEBUG_PRINT(" | Letter to ");
        DEBUG_PRINT(morse_letter);
        
        DEBUG_PRINT(" | Stage to ");
        DEBUG_PRINTLN(morse_dot_dah);

    }
}

void KTOME_Morse::drawScreen() {
    u8g2.firstPage();  
    do {
        for (byte ii = 0; ii < 5; ii++) {
            // DEBUG_PRINT("Drawing...");
            u8g2.setCursor(8, 40);
            u8g2.print("3.");
            
            u8g2.print(freq_list[freq_choice]+500);
            u8g2.setCursor(62, 40);
            u8g2.print(" MHz");
        }
    } while( u8g2.nextPage() );
}

void KTOME_Morse::clearScreen() {
    u8g2.clearDisplay();
}

byte KTOME_Morse::inputCheck() {
    byte result_byte = 0;
    // Rotary encoder check
    if (encoder_tracker != encoder.getCount()) {
        DEBUG_PRINT("Encoder changed!");
        DEBUG_PRINTLN(encoder.getCount());
        freq_choice = constrain(freq_choice + (encoder.getCount() - encoder_tracker),0,15);
        encoder_tracker = encoder.getCount();
        drawScreen();
        stepper.moveTo(freq_list[freq_choice]*30);
        stepper.setSpeed(stepperSpeed);
    }

    // Update displayed frequency

    if (hasButtonBeenPushed()) { // Submit button
       return logicCheck();
    }
    return 0;
}

bool KTOME_Morse::hasButtonBeenPushed() {
    if (switches[0].hasChanged()) {
        if (switches[0].isPressed()) {
            DEBUG_PRINTLN("Button pressed!");
            return true;
        } else {
            DEBUG_PRINTLN("Button released!");
            return false;
        }
    } else {
        return false;
    }
}

byte KTOME_Morse::logicCheck(){
    if (freq_choice == selected_word) { // Correct word
        module_solved = true;
        // leds.write(false);
        fleds_addr->write(1, CRGB::Black);
        return 2;
    } else {
        return 1;
    }
}

bool KTOME_Morse::isSolved() {
	if (module_solved) {
		return true;
	} else {
		return false;
	}
}

byte KTOME_Morse::update() {
    //Stepper
    stepper.runSpeedToPosition();
    
    //Morse light
    if (game_running && !module_solved) {
        blinkLetter();
    }

    return 0;
}

void KTOME_Morse::explode() {
    game_running = false;
    fleds_addr->blink(1, CRGB::Red, CRGB::Black, 100, 2);
    clearScreen();
}

void KTOME_Morse::defuse() {
    game_running = false;
}

String KTOME_Morse::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Morse::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Morse::needsIsr() {
    return false;
}

void KTOME_Morse::isrHandler() {
    
}

U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, MORSE_OLED_CLK, MORSE_OLED_DAT, MORSE_OLED_CS, MORSE_OLED_DC, MORSE_OLED_RST);
AccelStepper stepper(4, STEPPER_A, STEPPER_a, STEPPER_B, STEPPER_b);