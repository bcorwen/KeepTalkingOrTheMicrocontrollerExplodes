#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <Keypad.h>
#include <KTOME_Password.h>

KTOME_Password::KTOME_Password():kpd(makeKeymap(keyMatrix), rowPins, colPins, ROWS, COLS) {
    // Keypad kpd(makeKeymap(keyMatrix), rowPins, colPins, ROWS, COLS);
}

void KTOME_Password::start() {
    // for (byte ii = 0; ii < 11; ii++) {
    //     switches[ii].init(pin_array[ii]);
    // }
    leds.init(pin_array[11]);
    u8g2.begin();
    for (byte ii = 0; ii < 26; ii++) {
        alphabet[ii] = char('A' + ii);
    }
    // kpd = Keypad(makeKeymap(keyMatrix), rowPins, colPins, ROWS, COLS);
    kpd.setDebounceTime(50);
    reset();
}

void KTOME_Password::reset() {
    leds.write(false);
    clearScreen();
    module_defused = false;
}

void KTOME_Password::setBacklight(bool led_state) {
    leds.write(led_state);
}

void KTOME_Password::generate() {
    // Reset the character pools
    for (byte ii = 0; ii < 5; ii++) {
        for (byte jj = 0; jj < 26; jj++) {
            letter_pool[ii][jj] = true;
        }
        letter_left[ii] = 26;
    }
    selected_word = random(35);
    Serial.print("The password is: ");
    Serial.println(word_list[selected_word]);

    String this_word;
    for (byte ii = 0; ii < 35; ii++) {
        if (ii != selected_word) {
            bool word_possible = true;
            for (byte jj = 0; jj < 5; jj++) {
                word_possible &= letter_pool[jj][word_list[ii].charAt(jj)-'A'];
            }
            if (word_possible) {
                Serial.print("Word """);
                Serial.print(word_list[ii]);
                Serial.println(""" is possible! Time to prune letters...");
                removeOtherWords(ii);
            } else {
                Serial.print("Word """);
                Serial.print(word_list[ii]);
                Serial.println(""" is not possible!");
            }
        }
    }

    allocateSlots();
    // drawScreen(); // Don't draw screen before game starts!
}

void KTOME_Password::removeOtherWords(byte word_to_remove) {
    byte temp_holder;
    byte removalOrder[5] = {0, 1, 2, 3, 4};
    for (byte ii = 0; ii < 4; ii++) {
        byte swap_out = random(5-ii);
        temp_holder = removalOrder[4-ii];
        removalOrder[4-ii] = removalOrder[swap_out];
        removalOrder[swap_out] = temp_holder;
    }
    for (byte ii = 0; ii < 5; ii++) {
        if (word_list[word_to_remove].charAt(removalOrder[ii]) != word_list[selected_word].charAt(removalOrder[ii])) { // If this letter isn't the same as the selected letter...
            if (letter_pool[removalOrder[ii]][(word_list[word_to_remove].charAt(removalOrder[ii])-'A')]) { // and if this letter hasn't been removed from the pool...
                if (letter_left[removalOrder[ii]] > 6) { // and if there are letters left that can be removed from this slot...
                    letter_pool[removalOrder[ii]][(word_list[word_to_remove].charAt(removalOrder[ii])-'A')] = false;
                    letter_left[removalOrder[ii]]--;
                    Serial.print("Letter ");
                    Serial.print(word_list[word_to_remove].charAt(removalOrder[ii]));
                    Serial.print(" removed from column ");
                    Serial.println(removalOrder[ii]);
                    ii = 5;
                }
            }
        } else {
            Serial.print("Could not remove letter from column ");
            Serial.println(removalOrder[ii]);
        }
    }   
}

void KTOME_Password::allocateSlots() {
    byte letter_finder[26];
    byte letter_pointer;
    Serial.println("After checking other words:");
    for (byte ii = 0; ii < 5; ii++) { // For each slot / column...
        Serial.print("Slot ");
        Serial.print(ii);
        Serial.print(": ");
        Serial.print(letter_left[ii]);
        Serial.println(" letters to pick from");
        // Insert the correct letter for the chosen word and remove these letters from the list
        letter_reel[ii][0] = word_list[selected_word].charAt(ii);
        letter_pool[ii][(word_list[selected_word].charAt(ii)-'A')] = false;
        letter_left[ii]--;
        letter_pointer = 0;
        // Find every letter left to pick from and remember
        for (byte jj = 0; jj < 26; jj++) {
            if (letter_pool[ii][jj]) {
                letter_finder[letter_pointer] = jj;
                letter_pointer++;
                // Serial.print(alphabet[jj]);
                // Serial.print(" stored in element ");
                // Serial.println(jj);
            }
        }
        // Shuffle this list of available letters
        for (byte jj = 0; jj < letter_left[ii]-1; jj++) {
            byte swap_out = random(letter_left[ii]-jj);
            byte temp_holder = letter_finder[letter_left[ii]-(1+jj)];
            letter_finder[letter_left[ii]-(1+jj)] = letter_finder[swap_out];
            letter_finder[swap_out] = temp_holder;
            // Serial.print("Swapping ");
            // Serial.print(swap_out);
            // Serial.print(" with ");
            // Serial.println(letter_left[ii]-(1+jj));
        }
        for (byte jj = 0; jj < letter_left[ii]; jj++) {
            Serial.print(alphabet[letter_finder[jj]]);
            Serial.print(" ");
        }
        Serial.println();

        // Take the first 5 letters and fill the slots
        for (byte jj = 0; jj < 5; jj++) {
            letter_reel[ii][jj+1] = alphabet[letter_finder[jj]];
        }

        Serial.print(" - populated with: ");
        for (byte jj = 0; jj < 6; jj++) {
            Serial.print(letter_reel[ii][jj]);
            Serial.print(" ");
        }
        Serial.println();
        displayed_letters[ii] = random(6); // Pick a random letter to be first shown, which prevents the correct password being automatically shown!
    }

    // Test to see if password is displayed off the bat
    byte correct_letter_count = 0;
    for (byte ii = 0; ii < 5; ii++) {
        if (/*alphabet[*/letter_reel[ii][displayed_letters[ii]]/*]*/ == word_list[selected_word].charAt(ii)) {
            correct_letter_count++;
        }
    }
    if (correct_letter_count == 5) {
        byte shift_slot = random(5);
        displayed_letters[shift_slot] += (random(5) + 1);
        if (displayed_letters[shift_slot] > 6) {
            displayed_letters[shift_slot] - 6;
        }
    }

    Serial.print("Letters picked to show on screen at start: ");
    for (byte ii = 0; ii < 5; ii++) {
        Serial.print(letter_reel[ii][displayed_letters[ii]]);
        Serial.print(" ");
    }
    Serial.println();
}

void KTOME_Password::drawLetter(char letter_to_draw, byte position_on_screen) {
    byte px_left = 2 + position_on_screen * 26;
    switch (letter_to_draw) {
        case 'A':
            px_left += 2;
            drawBox(px_left, 0, 1, 1, 4);
            drawBox(px_left, 1, 0, 2, 1);
            drawBox(px_left, 1, 2, 2, 1);
            drawBox(px_left, 3, 1, 1, 4);
        break;
        case 'B':
            px_left += 2;
            drawBox(px_left, 0, 0, 3, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 1);
            drawBox(px_left, 1, 3, 2, 1);
            u8g2.setDrawColor(1);
            drawBox(px_left, 3, 1, 1, 1);
            drawBox(px_left, 3, 3, 1, 1);
        break;
        case 'C':
            px_left += 2;
            drawBox(px_left, 0, 1, 4, 3);
            drawBox(px_left, 1, 0, 2, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 3);
            drawBox(px_left, 3, 2, 1, 1);
            u8g2.setDrawColor(1);
        break;
        case 'D':
            px_left += 2;
            drawBox(px_left, 0, 0, 3, 5);
            drawBox(px_left, 3, 1, 1, 3);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 3);
            u8g2.setDrawColor(1);
        break;
        case 'E':
            px_left += 4;
            drawBox(px_left, 0, 0, 3, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 1);
            drawBox(px_left, 1, 3, 2, 1);
            u8g2.setDrawColor(1);
        break;
        case 'F':
            px_left += 4;
            drawBox(px_left, 0, 0, 3, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 1);
            drawBox(px_left, 1, 3, 2, 2);
            u8g2.setDrawColor(1);
        break;
        case 'G':
            px_left += 2;
            drawBox(px_left, 0, 1, 1, 3);
            drawBox(px_left, 1, 0, 3, 1);
            drawBox(px_left, 1, 4, 2, 1);
            drawBox(px_left, 2, 2, 2, 1);
            drawBox(px_left, 3, 3, 1, 1);
        break;
        case 'H':
            px_left += 2;
            drawBox(px_left, 0, 0, 4, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 0, 2, 2);
            drawBox(px_left, 1, 3, 2, 2);
            u8g2.setDrawColor(1);
        break;
        case 'I':
            px_left += 4;
            drawBox(px_left, 1, 0, 1, 5);
        break;
        case 'J':
            px_left += 2;
            drawBox(px_left, 0, 3, 1, 1);
            drawBox(px_left, 1, 4, 2, 1);
            drawBox(px_left, 3, 0, 1, 4);
        break;
        case 'K':
            px_left += 2;
            drawBox(px_left, 0, 0, 1, 5);
            drawBox(px_left, 3, 0, 1, 1);
            drawBox(px_left, 2, 1, 1, 1);
            drawBox(px_left, 1, 2, 1, 1);
            drawBox(px_left, 2, 3, 1, 1);
            drawBox(px_left, 3, 4, 1, 1);
        break;
        case 'L':
            px_left += 4;
            drawBox(px_left, 0, 0, 1, 5);
            drawBox(px_left, 1, 4, 2, 1);
        break;
        case 'M':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 5);
            drawBox(px_left, 4, 0, 1, 5);
            drawBox(px_left, 1, 1, 1, 1);
            drawBox(px_left, 2, 2, 1, 1);
            drawBox(px_left, 3, 1, 1, 1);
        break;
        case 'N':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 5);
            drawBox(px_left, 4, 0, 1, 5);
            drawBox(px_left, 1, 1, 1, 1);
            drawBox(px_left, 2, 2, 1, 1);
            drawBox(px_left, 3, 3, 1, 1);
        break;
        case 'O':
            px_left += 2;
            drawBox(px_left, 0, 1, 4, 3);
            drawBox(px_left, 1, 0, 2, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 3);
            u8g2.setDrawColor(1);
        break;
        case 'P':
            px_left += 2;
            drawBox(px_left, 0, 0, 3, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 1);
            drawBox(px_left, 1, 3, 2, 2);
            u8g2.setDrawColor(1);
            drawBox(px_left, 3, 1, 1, 1);
        break;
        case 'Q':
            px_left += 2;
            drawBox(px_left, 0, 1, 4, 3);
            drawBox(px_left, 1, 0, 2, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 3);
            u8g2.setDrawColor(1);
            drawBox(px_left, 3, 5, 1, 1);
        break;
        case 'R':
            px_left += 2;
            drawBox(px_left, 0, 0, 3, 5);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 1, 2, 1);
            drawBox(px_left, 1, 3, 2, 2);
            u8g2.setDrawColor(1);
            drawBox(px_left, 3, 1, 1, 1);
            drawBox(px_left, 2, 3, 1, 1);
            drawBox(px_left, 3, 4, 1, 1);
        break;
        case 'S':
            px_left += 2;
            drawBox(px_left, 1, 0, 3, 1);
            drawBox(px_left, 0, 1, 1, 1);
            drawBox(px_left, 1, 2, 2, 1);
            drawBox(px_left, 3, 3, 1, 1);
            drawBox(px_left, 0, 4, 3, 1);
        break;
        case 'T':
            px_left += 4;
            drawBox(px_left, 1, 0, 1, 5);
            drawBox(px_left, 0, 0, 3, 1);
        break;
        case 'U':
            px_left += 2;
            drawBox(px_left, 1, 4, 2, 1);
            drawBox(px_left, 0, 0, 4, 4);
            u8g2.setDrawColor(0);
            drawBox(px_left, 1, 0, 2, 4);
            u8g2.setDrawColor(1);
        break;
        case 'V':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 2);
            drawBox(px_left, 4, 0, 1, 2);
            drawBox(px_left, 1, 2, 1, 2);
            drawBox(px_left, 3, 2, 1, 2);
            drawBox(px_left, 2, 4, 1, 1);
        break;
        case 'W':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 4);
            drawBox(px_left, 4, 0, 1, 4);
            drawBox(px_left, 1, 4, 1, 1);
            drawBox(px_left, 3, 4, 1, 1);
            drawBox(px_left, 2, 1, 1, 3);
        break;
        case 'X':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 1);
            drawBox(px_left, 0, 4, 1, 1);
            drawBox(px_left, 1, 1, 1, 1);
            drawBox(px_left, 1, 3, 1, 1);
            drawBox(px_left, 2, 2, 1, 1);
            drawBox(px_left, 3, 1, 1, 1);
            drawBox(px_left, 3, 3, 1, 1);
            drawBox(px_left, 4, 0, 1, 1);
            drawBox(px_left, 4, 4, 1, 1);
        break;
        case 'Y':
            px_left += 0;
            drawBox(px_left, 0, 0, 1, 1);
            drawBox(px_left, 1, 1, 1, 1);
            drawBox(px_left, 2, 2, 1, 3);
            drawBox(px_left, 3, 1, 1, 1);
            drawBox(px_left, 4, 0, 1, 1);
        break;
        case 'Z':
            px_left += 4;
            drawBox(px_left, 0, 0, 3, 1);
            drawBox(px_left, 2, 1, 1, 1);
            drawBox(px_left, 1, 2, 1, 1);
            drawBox(px_left, 0, 3, 1, 1);
            drawBox(px_left, 0, 4, 3, 1);
        break;
        default:
            px_left += 2;
            drawBox(px_left, 0, 1, 1, 1);
            drawBox(px_left, 1, 0, 2, 1);
            drawBox(px_left, 3, 1, 1, 1);
            drawBox(px_left, 2, 2, 1, 2);
            drawBox(px_left, 2, 5, 1, 1);
        break;
    }
}

void KTOME_Password::drawBox(byte left_offset, byte left_edge, byte top_edge, byte px_width, byte px_height) {
    byte px_top = 22;
    byte px_size = 4;
    u8g2.drawBox(left_offset + left_edge * px_size, px_top + top_edge * px_size, px_width * px_size, px_height * px_size);
}

void KTOME_Password::drawScreen() {
    u8g2.firstPage();  
    do {
        for (byte ii = 0; ii < 5; ii++) {
            // Serial.print("Drawing...");
            // Serial.println(letter_reel[ii][displayed_letters[ii]]);
            // u8g2.prepare();
            // u8g2.drawBox(10,10,30,30);
            drawLetter(letter_reel[ii][displayed_letters[ii]], ii);
        }
    } while( u8g2.nextPage() );
}

void KTOME_Password::clearScreen() {
    u8g2.clearDisplay();
}

byte KTOME_Password::inputCheck() {
    byte result_byte = 0;
    byte ii = 12;
    char pressedKey = kpd.getKey();
    if (pressedKey){
        Serial.print("Key pressed: ");
        Serial.println(pressedKey);
        switch (pressedKey) {
            case 'A':
                ii = 0;
                break;
            case 'a':
                ii = 1;
                break;
            case 'B':
                ii = 2;
                break;
            case 'b':
                ii = 3;
                break;
            case 'C':
                ii = 4;
                break;
            case 'c':
                ii = 5;
                break;
            case 'D':
                ii = 6;
                break;
            case 'd':
                ii = 7;
                break;
            case 'E':
                ii = 8;
                break;
            case 'e':
                ii = 9;
                break;
            case '!':
                ii = 10;
                break;
            case '?':
                ii = 11;
                break;
        }
        if (ii < 10) {
            upDownButton(ii);
        } else if (ii == 10) {
            return submitButton();
        }
    }
    return 0;

    /*
    for (byte ii = 0; ii < 10; ii++) { // Change letter buttons, even is up arrow (next array element), odd is down arrow (prev array element)
        if (hasButtonBeenPushed(ii)) {
            result_byte = (ii / 2);
            if ((ii % 2 == 0) && (displayed_letters[result_byte] == 5)) { // Increasing but at max
                displayed_letters[result_byte] = 0;
            } else if ((ii % 2 == 1) && (displayed_letters[result_byte] == 0)) { // Decreasing but at min
                displayed_letters[result_byte] = 5;
            } else if ((ii % 2 == 0)) { // Increasing
                displayed_letters[result_byte]++;
            } else if ((ii % 2 == 1) ) { // Decreasing
                displayed_letters[result_byte]--;
            }
            drawScreen();
        }
    }
    if (hasButtonBeenPushed(10)) { // Submit button
        if (logicCheck()) {
            return 2;
        } else {
            return 1;
        }
    }
    return 0;
    */
}

void KTOME_Password::upDownButton(byte button_number) {
    byte result_byte = (button_number / 2);
    if ((button_number % 2 == 0) && (displayed_letters[result_byte] == 5)) { // Increasing but at max
        displayed_letters[result_byte] = 0;
    } else if ((button_number % 2 == 1) && (displayed_letters[result_byte] == 0)) { // Decreasing but at min
        displayed_letters[result_byte] = 5;
    } else if ((button_number % 2 == 0)) { // Increasing
        displayed_letters[result_byte]++;
    } else if ((button_number % 2 == 1) ) { // Decreasing
        displayed_letters[result_byte]--;
    }
    drawScreen();
}

byte KTOME_Password::submitButton() {
    if (logicCheck()) {
        return 2;
    } else {
        return 1;
    }
}

bool KTOME_Password::hasButtonBeenPushed(byte button_number) {
    /*
    if (switches[button_number].hasChanged()) {
        if (switches[button_number].isPressed()) {
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
    */
}

bool KTOME_Password::logicCheck(){
    bool logic_check = true;
    for (byte ii = 0; ii < 5; ii++) {
        logic_check &= (letter_reel[ii][displayed_letters[ii]] == word_list[selected_word].charAt(ii));
        Serial.print(letter_reel[ii][displayed_letters[ii]]);
        Serial.println(word_list[selected_word].charAt(ii));
    }
    if (logic_check) { // Correct word
        module_defused = true;
        return true;
    } else {
        return false;
    }
}

bool KTOME_Password::isDefused() {
	if (module_defused) {
		return true;
	} else {
		return false;
	}
}

void KTOME_Password::explode() {
    leds.write(false);
    clearScreen();
}

U8G2_ST7920_128X64_1_SW_SPI u8g2 = U8G2_ST7920_128X64_1_SW_SPI(U8G2_R0, /* clock=*/ PASSWORD_GLCD_C, /* data=*/ PASSWORD_GLCD_D, /* CS=*/ PASSWORD_GLCD_L, /* reset=*/ PASSWORD_GLCD_R);
// Keypad kpd = Keypad(makeKeymap(keyMatrix), rowPins, colPins, ROWS, COLS);