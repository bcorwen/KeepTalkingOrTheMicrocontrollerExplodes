#include <Arduino.h>
#include <KTOME_common.h>
#include <KTOME_Memory.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <xbm_1.h>
#include <xbm_2.h>
#include <xbm_3.h>
#include <xbm_4.h>
#include <Mem_1a.h>
#include <Mem_2a.h>
#include <Mem_3a.h>
#include <Mem_4a.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
// #include <GxEPD2_display_selection_new_style.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Wire.h>

KTOME_Memory::KTOME_Memory() {
    
}

void KTOME_Memory::start() {
    
    for (byte ii = 0; ii < 5; ii++) {
        leds[ii].init(pin_array[ii]);
    }

    display.init();

    Wire.begin(21, 22, 400000);
    digitalWrite(TP_RST_PIN, 1);
	delay(100);
	digitalWrite(TP_RST_PIN, 0);
	delay(100);
	digitalWrite(TP_RST_PIN, 1);
	delay(100);

    pinMode(TFT_POWER, OUTPUT);
    tft.init();
    tft.setRotation(1);

    reset();
}

void KTOME_Memory::reset() {
    stage = 0;
    display_stage = 0;
    updateStageLEDs();
    module_defused = false;
    touch_status = 4;
    drawBottomScreen(true);
    powerTopScreen(false);
    drawTopScreen(6);
    isbusy = true;
}

void KTOME_Memory::generate() {

    for (byte ii = 0; ii < 5; ii++) { // Shuffle button labels
        for (byte jj = 0; jj < 3; jj++) {
            byte swap_out = random(4);
            byte temp_holder = button_labels[ii][jj];
            button_labels[ii][jj] = button_labels[ii][swap_out];
            button_labels[ii][swap_out] = temp_holder;
        }
        display_labels[ii] = random(4) + 1; // Generate random display number prompt
    }
    for (byte ii = 0; ii < 5; ii++) { // Store where labels are for reference
        for (byte jj = 0; jj < 4; jj++) {
            button_positions[ii][button_labels[ii][jj]-1] = jj+1;
        }
    }

    // Store correct values
    switch (display_labels[0]) {
        case 1:
            correct_inputs[0] = 1;
            break;
        case 2:
            correct_inputs[0] = 1;
            break;
        case 3:
            correct_inputs[0] = 2;
            break;
        case 4:
            correct_inputs[0] = 3;
            break;
    }
    switch (display_labels[1]) {
        case 1:
            correct_inputs[1] = button_positions[1][3]-1;
            break;
        case 2:
            correct_inputs[1] = correct_inputs[0];
            break;
        case 3:
            correct_inputs[1] = 0;
            break;
        case 4:
            correct_inputs[1] = correct_inputs[0];
            break;
    }
    switch (display_labels[2]) {
        case 1:
            correct_inputs[2] = button_positions[2][button_labels[1][correct_inputs[1]]-1]-1;
            break;
        case 2:
            correct_inputs[2] = button_positions[2][button_labels[0][correct_inputs[0]]-1]-1;
            break;
        case 3:
            correct_inputs[2] = 2;
            break;
        case 4:
            correct_inputs[2] = button_positions[2][3]-1;
            break;
    }
    switch (display_labels[3]) {
        case 1:
            correct_inputs[3] = correct_inputs[0];
            break;
        case 2:
            correct_inputs[3] = 0;
            break;
        case 3:
            correct_inputs[3] = correct_inputs[1];
            break;
        case 4:
            correct_inputs[3] = correct_inputs[1];
            break;
    }
    switch (display_labels[4]) {
        case 1:
            correct_inputs[4] = button_positions[4][button_labels[0][correct_inputs[0]]-1]-1;
            break;
        case 2:
            correct_inputs[4] = button_positions[4][button_labels[1][correct_inputs[1]]-1]-1;
            break;
        case 3:
            correct_inputs[4] = button_positions[4][button_labels[3][correct_inputs[3]]-1]-1;
            break;
        case 4:
            correct_inputs[4] = button_positions[4][button_labels[2][correct_inputs[2]]-1]-1;
            break;
    }    
}

void KTOME_Memory::begin() {
    Serial.println("Enabling game!");
    display_stage = 3;
    drawTopScreen(display_labels[stage]);
    powerTopScreen(true);
}

void KTOME_Memory::powerTopScreen(bool power_state) {
    if (power_state) {
        digitalWrite(TFT_POWER, HIGH);
        Serial.println("TFT on!");
    } else {
        digitalWrite(TFT_POWER, LOW);
        Serial.println("TFT off!");
    }
}

void KTOME_Memory::drawTopScreen(byte displaynumber) {
    Serial.printf("Drawing TFT... displaying %d\n", displaynumber);
    if (displaynumber == 6) {
        tft.fillScreen(ILI9341_BACKGROUND);
        for (int ii = 1; ii < 4; ii++) {
            tft.drawLine(0, 30*ii, 320, 30*ii, ILI9341_DARKGREY);
        }
        for (int ii = 5; ii < 8; ii++) {
            tft.drawLine(0, 30*ii+1, 320, 30*ii+1, ILI9341_DARKGREY);
        }
        for (int ii = 1; ii < 4; ii++) {
            tft.drawLine(40*ii, 0, 40*ii, 240, ILI9341_DARKGREY);
        }
        for (int ii = 5; ii < 8; ii++) {
            tft.drawLine(40*ii+1, 0, 40*ii+1, 240, ILI9341_DARKGREY);
        }
        tft.fillRect(0, 119, 320, 4, ILI9341_DARKCYAN);
        tft.fillRect(159, 0, 4, 240, ILI9341_MAGENTA);
    } else if (displaynumber == 0) {
        switch (top_display_number) {
            case 1:
                tft.drawXBitmap(121,30,no_1,mem1Width,mem1Height,ILI9341_BACKGROUND);
                break;
            case 2:
                tft.drawXBitmap(85,29,no_2,mem2Width,mem2Height,ILI9341_BACKGROUND);
                break;
            case 3:
                tft.drawXBitmap(81,30,no_3,mem3Width,mem3Height,ILI9341_BACKGROUND);
                break;
            case 4:
                tft.drawXBitmap(69,29,no_4,mem4Width,mem4Height,ILI9341_BACKGROUND);
                break;
        }
        for (int ii = 1; ii < 4; ii++) {
            tft.drawLine(0, 30*ii, 320, 30*ii, ILI9341_DARKGREY);
        }
        for (int ii = 5; ii < 8; ii++) {
            tft.drawLine(0, 30*ii+1, 320, 30*ii+1, ILI9341_DARKGREY);
        }
        for (int ii = 1; ii < 4; ii++) {
            tft.drawLine(40*ii, 0, 40*ii, 240, ILI9341_DARKGREY);
        }
        for (int ii = 5; ii < 8; ii++) {
            tft.drawLine(40*ii+1, 0, 40*ii+1, 240, ILI9341_DARKGREY);
        }
        tft.fillRect(0, 119, 320, 4, ILI9341_DARKCYAN);
        tft.fillRect(159, 0, 4, 240, ILI9341_MAGENTA);
    }

    switch (displaynumber) {
        case 1:
            tft.drawXBitmap(121,30,no_1,mem1Width,mem1Height,ILI9341_WHITE);
            break;
        case 2:
            tft.drawXBitmap(85,29,no_2,mem2Width,mem2Height,ILI9341_WHITE);
            break;
        case 3:
            tft.drawXBitmap(81,30,no_3,mem3Width,mem3Height,ILI9341_WHITE);
            break;
        case 4:
            tft.drawXBitmap(69,29,no_4,mem4Width,mem4Height,ILI9341_WHITE);
            break;
    }

    if (displaynumber == 7) {
        tft.fillScreen(ILI9341_WHITE);
    }

    top_display_number = displaynumber;
}

void KTOME_Memory::drawBottomScreen(bool full_update) {
    Serial.println("Drawing e-ink...");
    display.setRotation(1);
    if (full_update) {

        display.setFullWindow();
        display.firstPage();
        do {
            display.fillScreen(colorBlack);
        } while (display.nextPage());
        Serial.println("Clearing the e-ink display to black...");

    } else {

        display.setPartialWindow(0,0,display.width(),display.height());
        display.firstPage();
        do {

            if (display_stage == 2) {
                display.fillScreen(colorBlack);
                Serial.println("Setting the e-ink display to black...");
            }
            if (display_stage == 3) {
                display.fillScreen(colorBlack);
                Serial.println("Drawing buttons to the e-ink...");
                for (byte ii = 0; ii < 4; ii++) {
                    display.fillRoundRect(2+ii*74,2,70,124-2,9,colorWhite);
                    switch (button_labels[stage][ii]) {
                        case 1:
                        display.drawXBitmap((ii*74 + 34 - mem1aWidth/2), 18, no_1a, mem1aWidth, mem1aHeight, colorBlack);
                        break;
                        case 2:
                        display.drawXBitmap((ii*74 + 37 - mem2aWidth/2), 18, no_2a, mem2aWidth, mem2aHeight, colorBlack);
                        break;
                        case 3:
                        display.drawXBitmap((ii*74 + 37 - mem3aWidth/2), 18, no_3a, mem3aWidth, mem3aHeight, colorBlack);
                        break;
                        case 4:
                        display.drawXBitmap((ii*74 + 37 - mem4aWidth/2), 18, no_4a, mem4aWidth, mem4aHeight, colorBlack);
                        break;
                    }
                }

            }
        } while (display.nextPage());
    }
}

bool KTOME_Memory::isAcceptingInputs() {
    // Serial.printf("isbusy %d\n", isbusy);
    return !isbusy;
}

byte KTOME_Memory::inputCheck() {
    byte result_byte = hasButtonBeenPushed();

    if (result_byte > 0 && stage < 5) {
        Serial.printf("Correct responses are: %d %d %d %d %d\n", correct_inputs[0]+1, correct_inputs[1]+1, correct_inputs[2]+1, correct_inputs[3]+1, correct_inputs[4]+1);
        Serial.printf("input checking... result = %d\n", result_byte);

        isbusy = true;
        busy_timer = millis();
        display_stage = 1;
        if (result_byte == correct_inputs[stage]+1) { // Correct input...
            stage++;
            updateStageLEDs();
            Serial.printf("Correct! Move to stage %d \n", stage);
            if (stage == 5) {
                module_defused = true;
                return 2; // Correct and solved
            }
        } else {
            stage = 0;
            Serial.printf("Incorrect! Move back to stage %d \n", stage);
            updateStageLEDs();
            generate();
            return 1; // Incorrect input
        }
        
    }
    return 0;
}

byte KTOME_Memory::hasButtonBeenPushed() {
    if (touch_flag) {

        Serial.println("Start fectching touch data...");
        readI2C(IIC_Address, IIC_Reg_2, 35);
        writeI2C(IIC_Address, IIC_Reg_1, 0x00);
        
        uint16_t touch_x = (buffer[2] << 8) + buffer[1];
        uint16_t touch_y = (buffer[4] << 8) + buffer[3];
        // uint16_t touch_p = buffer[5];
        uint16_t touch_e = buffer[6];
        // uint16_t touch_o = buffer[0];
        Serial.printf("X: %d, Y: %d, E: %d\n", touch_x, touch_y, touch_e);

        touch_flag = false;

        if ((touch_status == 1 || touch_status == 2) && touch_e == 4) { // No longer touching
            touch_status = touch_e;
        } else if ((touch_status == 4 || touch_status == 0) && (touch_e == 1 || touch_e == 2)) { // Started touching
            touch_status = touch_e;
            Serial.printf("New touch @ X=%d Y=%d : ",touch_x, touch_y);
            if (touch_x < 296/4 - 1) {
                Serial.println("Pressed first position!");
                return 1;
            } else if (touch_x < 2*296/4 - 1) {
                Serial.println("Pressed second position!");
                return 2;
            } else if (touch_x < 3*296/4 - 1) {
                Serial.println("Pressed third position!");
                return 3;
            } else if (touch_x < 4*296/4 - 1) {
                Serial.println("Pressed fourth position!");
                return 4;
            }
        }
    }
    return 0;
}

void KTOME_Memory::isrhandler() {
    touch_flag = true;
}

void KTOME_Memory::writeI2C(int8_t addr, int16_t reg, int8_t msg) {
    Serial.println("Writing to I2C...");
  Wire.beginTransmission(addr);  
  Wire.write(reg >> 8);
  Wire.write(reg & 0xff);
  Wire.write(msg);
  Wire.endTransmission();
}

void KTOME_Memory::readI2C(int8_t addr, int16_t reg, int8_t len) {
    Serial.println("Reading from I2C...");
  Wire.beginTransmission(addr);  
  Wire.write(reg >> 8);
  Wire.write(reg & 0xff);
  Wire.endTransmission(false);
  Wire.requestFrom(IIC_Address, len);
  Wire.readBytes(buffer, len);
}

void KTOME_Memory::updateDisplays() {
    if (isbusy) {
        int32_t thistime = millis();
        if (thistime > busy_timer) {
            switch (display_stage) {
                case 1: // Turn off main display
                    drawTopScreen(0);
                    display_stage++;
                    busy_timer += 1000;
                    break;
                case 2: // Remove buttons from e-ink display
                    drawBottomScreen(false);
                    display_stage++;
                    busy_timer += 1000;
                    break;
                case 3: // Display new e-ink buttons
                    drawBottomScreen(false);
                    display_stage++;
                    busy_timer += 1000;
                    break;
                case 4: // Display new pormpt on main display
                    drawTopScreen(display_labels[stage]);
                    display_stage = 0;
                    isbusy = false;
                    break;
                case 5: // Flash on explode
                    drawTopScreen(7);
                    display_stage++;
                    busy_timer += 100;
                    break;
                case 6: // Turn off flash
                    powerTopScreen(false);
                    drawTopScreen(0);
                    display_stage = 0;
                    drawBottomScreen(true);
                    break;
            }
            // Serial.printf("Display stage = %d\n", display_stage);
        }
    }
}

void KTOME_Memory::updateStageLEDs() {
    for (byte ii = 0; ii < 5; ii++) {
        if (ii < stage) {
            leds[ii].write(true);
        } else {
            leds[ii].write(false);
        }
    }
}

bool KTOME_Memory::isDefused() {
	if (module_defused) {
		return true;
	} else {
		return false;
	}
}

void KTOME_Memory::explode() {
    for (byte ii = 0; ii < 5; ii++) {
        leds[ii].write(false);
    }
    // powerTopScreen(false);
    // drawTopScreen(6);
    // drawBottomScreen(true);
    display_stage = 5;
    isbusy = true;
    busy_timer = millis();
}

TFT_eSPI tft = TFT_eSPI();
TwoWire einktouch = TwoWire(0);
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display = GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)>(GxEPD2_DRIVER_CLASS(/*CS=5*/ 25 /*25*/, /*DC=*/ 32 /*26*/, /*RST=*/ 33 /*27*/, /*BUSY=*/ 4));