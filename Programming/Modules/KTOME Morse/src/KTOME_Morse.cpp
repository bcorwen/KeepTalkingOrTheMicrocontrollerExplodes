#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <KTOME_Morse.h>
// #include <Munro_35.h>
#include <Munro_31.h>

KTOME_Morse::KTOME_Morse() {
    // u8g2 = new U8G2_ST7920_128X64_1_SW_SPI(U8G2_R0, /* clock=*/ 19, /* data=*/ 21, /* CS=*/ 22, /* reset=*/ 8);
    // U8G2_ST7920_128X64_1_SW_SPI u8g2 (U8G2_R0, /* clock=*/ 19, /* data=*/ 21, /* CS=*/ 22, /* reset=*/ 8);
}

void KTOME_Morse::start() {
    switches.init(pin_array[3]);
    leds.init(pin_array[0]);
    ESP32Encoder::useInternalWeakPullResistors=UP;
    encoder.attachHalfQuad(pin_array[1], pin_array[2]);
    encoder.setCount(0);
    encoder_tracker = 0;
    u8g2.begin();
    u8g2.setFont(Munro_31);
    reset();
}

void KTOME_Morse::reset() {
    leds.write(false);
    clearScreen();
    module_defused = false;
}

void KTOME_Morse::gameStart() {
    Serial.println("Turn on the screen...");
    encoder_tracker = encoder.getCount();
    drawScreen();
}

void KTOME_Morse::generate() {
    selected_word = random(16);
    Serial.print("The morse code is: ");
    Serial.println(word_list[selected_word]);
}

void KTOME_Morse::update() {
    blinkLetter();
    leds.update();
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

        leds.blink(true, morse_timings[letter_code[morse_dot_dah]], 1);
        morse_timer = this_time + morse_timings[letter_code[morse_dot_dah]] + morse_timings[0];
        Serial.print("Bilnk of ");
        Serial.print(morse_timings[letter_code[morse_dot_dah]]);        
        Serial.print(" | Timer set to ");
        Serial.print(morse_timer);
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

        Serial.print(" | Letter to ");
        Serial.print(morse_letter);
        
        Serial.print(" | Stage to ");
        Serial.println(morse_dot_dah);

    }
}

void KTOME_Morse::drawScreen() {
    u8g2.firstPage();  
    do {
        for (byte ii = 0; ii < 5; ii++) {
            // Serial.print("Drawing...");
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
        Serial.print("Encoder changed!");
        Serial.println(encoder.getCount());
        freq_choice = constrain(freq_choice + (encoder.getCount() - encoder_tracker),0,15);
        encoder_tracker = encoder.getCount();
        drawScreen();
    }

    // Update displayed frequency

    if (hasButtonBeenPushed()) { // Submit button
        if (logicCheck()) {
            return 2;
        } else {
            return 1;
        }
    }
    return 0;
}

bool KTOME_Morse::hasButtonBeenPushed() {
    if (switches.hasChanged()) {
        if (switches.isPressed()) {
            Serial.println("Button pressed!");
            return true;
        } else {
            Serial.println("Button released!");
            return false;
        }
    } else {
        return false;
    }
}

bool KTOME_Morse::logicCheck(){
    if (freq_choice == selected_word) { // Correct word
        module_defused = true;
        leds.write(false);
        return true;
    } else {
        return false;
    }
}

bool KTOME_Morse::isDefused() {
	if (module_defused) {
		return true;
	} else {
		return false;
	}
}

void KTOME_Morse::defused() {
    Serial.println("Defused method");
    leds.write(false);
}

void KTOME_Morse::explode() {
    Serial.println("Explode method");
    leds.write(false);
    clearScreen();
}

// U8G2_ST7920_128X64_1_SW_SPI u8g2 = U8G2_ST7920_128X64_1_SW_SPI(U8G2_R0, /* clock=*/ MORSE_GLCD_C, /* data=*/ MORSE_GLCD_D, /* CS=*/ MORSE_GLCD_L, /* reset=*/ MORSE_GLCD_R);
U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, MORSE_OLED_CLK, MORSE_OLED_DAT, MORSE_OLED_CS, MORSE_OLED_DC, MORSE_OLED_RST);  