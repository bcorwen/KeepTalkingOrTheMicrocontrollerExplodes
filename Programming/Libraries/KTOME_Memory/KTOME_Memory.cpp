//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 17/06/22
//======================================================================
//
//  Module: Memory (Slave, Solvable, Vanilla)
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

KTOME_Memory::KTOME_Memory() {
    
}

void KTOME_Memory::start(FLedPWM *fleds) {
    
    fleds_addr = fleds; // Store pointer to leds

    pinMode(TFT_POWER, OUTPUT);
    pinMode(TP_RST_PIN, OUTPUT);
    pinMode(TP_INT_PIN, INPUT);

    einkdisplay.init();

    Wire.begin(TP_SDA_PIN, TP_SCL_PIN, 400000);
    digitalWrite(TP_RST_PIN, 1);
	delay(100);
	digitalWrite(TP_RST_PIN, 0);
	delay(100);
	digitalWrite(TP_RST_PIN, 1);
	delay(100);

    tft.init();
    tft.setRotation(1);

    DEBUG_PRINTLN("Preparing top screen powerOn...");
    display_stage = 8;
    drawingScreens = true;
    // busy_timer = millis();
    // DEBUG_PRINT("Splash 0 @ ");
    // DEBUG_PRINTLN(busy_timer);

    DEBUG_PRINT("Splash 1 @ ");
    // DEBUG_PRINTLN(millis(););
    drawTopScreen(8);
    powerTopScreen(true);
    busy_timer = millis(); + 6000;

    reset();
}

void KTOME_Memory::reset() {
    for (byte ii = 0; ii < 5; ii++) {
        fleds_addr->write(ii+1, CRGB::Black);
    }
    strike_count = 0;
    game_running = false;
    module_solved = false;
    stage = 0;
    // display_stage = 0;
    updateStageLEDs();
    touch_status = 4;
    drawBottomScreen(true);
    if (!drawingScreens) {
        powerTopScreen(false);
    }
    // drawTopScreen(6);
}

void KTOME_Memory::powerOn() {
    // MOVED TO START TO JUMP AHEAD OF EINK REFRESH
    // DEBUG_PRINTLN("Preparing top screen powerOn...");
    // display_stage = 7;
    // drawingScreens = true;
    // busy_timer = millis();
    // DEBUG_PRINT("Splash 0 @ ");
    // DEBUG_PRINTLN(busy_timer);
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
            correct_inputs[0] = 1; break;
        case 2:
            correct_inputs[0] = 1; break;
        case 3:
            correct_inputs[0] = 2; break;
        case 4:
            correct_inputs[0] = 3; break;
    }
    switch (display_labels[1]) {
        case 1:
            correct_inputs[1] = button_positions[1][3]-1; break;
        case 2:
            correct_inputs[1] = correct_inputs[0]; break;
        case 3:
            correct_inputs[1] = 0; break;
        case 4:
            correct_inputs[1] = correct_inputs[0]; break;
    }
    switch (display_labels[2]) {
        case 1:
            correct_inputs[2] = button_positions[2][button_labels[1][correct_inputs[1]]-1]-1; break;
        case 2:
            correct_inputs[2] = button_positions[2][button_labels[0][correct_inputs[0]]-1]-1; break;
        case 3:
            correct_inputs[2] = 2; break;
        case 4:
            correct_inputs[2] = button_positions[2][3]-1; break;
    }
    switch (display_labels[3]) {
        case 1:
            correct_inputs[3] = correct_inputs[0]; break;
        case 2:
            correct_inputs[3] = 0; break;
        case 3:
            correct_inputs[3] = correct_inputs[1]; break;
        case 4:
            correct_inputs[3] = correct_inputs[1]; break;
    }
    switch (display_labels[4]) {
        case 1:
            correct_inputs[4] = button_positions[4][button_labels[0][correct_inputs[0]]-1]-1; break;
        case 2:
            correct_inputs[4] = button_positions[4][button_labels[1][correct_inputs[1]]-1]-1; break;
        case 3:
            correct_inputs[4] = button_positions[4][button_labels[3][correct_inputs[3]]-1]-1; break;
        case 4:
            correct_inputs[4] = button_positions[4][button_labels[2][correct_inputs[2]]-1]-1; break;
    }

    DEBUG_PRINT("Correct responses are: ");
    DEBUG_PRINT(correct_inputs[0]+1);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(correct_inputs[1]+1);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(correct_inputs[2]+1);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(correct_inputs[3]+1);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(correct_inputs[4]+1);
}

bool KTOME_Memory::isManual() { // Required manual setup
    return false;
}

String KTOME_Memory::getManual() {
    DEBUG_PRINTLN("ERROR: Should not have been asked to fetch manual setup!");
    return "";
}

byte KTOME_Memory::manualConfirm() { // No further confirm needed for this module, return 0
    return 0;
}

void KTOME_Memory::widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels) {
    
}

void KTOME_Memory::strikeUpdate(byte strikes) {

}

byte KTOME_Memory::timerUpdate(String timer_digits) {
    
}

void KTOME_Memory::gameStart() {
    game_running = true;
    display_stage = 3;
    drawingScreens = false;
    drawTopScreen(display_labels[stage]);
    drawBottomScreen(false);
    powerTopScreen(true);
}

void KTOME_Memory::powerTopScreen(bool power_state) {
    if (power_state) {
        digitalWrite(TFT_POWER, HIGH);
        DEBUG_PRINTLN("TFT on!");
    } else {
        digitalWrite(TFT_POWER, LOW);
        DEBUG_PRINTLN("TFT off!");
    }
}

void KTOME_Memory::drawTopScreen(byte displaynumber) {
    DEBUG_PRINT("Drawing TFT... displaying ");
    DEBUG_PRINTLN(displaynumber);
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
    if (displaynumber == 8) {
        tft.fillScreen(ILI9341_BLUE);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(1);
        tft.setTextFont(2);
        // tft.setFreeFont(FreeMono9pt7b);
        tft.drawString("rst:0x1 (POWERON_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)", 2, 2);
        tft.drawString("configsip: 0, SPIWP:0xee", 2, 14);
        tft.drawString("clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00", 2, 26);
        tft.drawString("mode:DIO, clock div:2", 2, 38);
        tft.drawString("load:0x3fff0018,len:4", 2, 50);
        tft.drawString("load:0x3fff001c,len:1044", 2, 62);
        tft.drawString("load:0x40078000,len:10124", 2, 74);
        tft.drawString("load:0x40080400,len:5828", 2, 86);
        tft.drawString("entry 0x400806a8", 2, 98);
        tft.drawString("Loading module: Memory...", 2, 110);
    }
    if (displaynumber == 9) {
        tft.drawString("Loading module: Memory... OK!", 2, 110);
        tft.drawString("Initialised module logic - preparing ""src/main.cpp""", 2, 122);
    }

    top_display_number = displaynumber;
}

void KTOME_Memory::drawBottomScreen(bool full_update) {
    DEBUG_PRINTLN("Drawing e-ink...");
    einkdisplay.setRotation(1);
    if (full_update) {

        einkdisplay.setFullWindow();
        einkdisplay.firstPage();
        do {
            einkdisplay.fillScreen(colorBlack);
        } while (einkdisplay.nextPage());
        DEBUG_PRINTLN("Clearing the e-ink display to black...");

    } else {

        einkdisplay.setPartialWindow(0,0,einkdisplay.width(),einkdisplay.height());
        einkdisplay.firstPage();
        do {

            if (display_stage == 2) {
                einkdisplay.fillScreen(colorBlack);
                DEBUG_PRINTLN("Setting the e-ink display to black...");
            }
            if (display_stage == 3) {
                einkdisplay.fillScreen(colorBlack);
                DEBUG_PRINTLN("Drawing buttons to the e-ink...");
                for (byte ii = 0; ii < 4; ii++) {
                    einkdisplay.fillRoundRect(2+ii*74,2,70,124-2,9,colorWhite);
                    switch (button_labels[stage][ii]) {
                        case 1:
                        einkdisplay.drawXBitmap((ii*74 + 34 - mem1aWidth/2), 18, no_1a, mem1aWidth, mem1aHeight, colorBlack);
                        break;
                        case 2:
                        einkdisplay.drawXBitmap((ii*74 + 37 - mem2aWidth/2), 18, no_2a, mem2aWidth, mem2aHeight, colorBlack);
                        break;
                        case 3:
                        einkdisplay.drawXBitmap((ii*74 + 37 - mem3aWidth/2), 18, no_3a, mem3aWidth, mem3aHeight, colorBlack);
                        break;
                        case 4:
                        einkdisplay.drawXBitmap((ii*74 + 37 - mem4aWidth/2), 18, no_4a, mem4aWidth, mem4aHeight, colorBlack);
                        break;
                    }
                }

            }
        } while (einkdisplay.nextPage());
    }
}

bool KTOME_Memory::isAcceptingInputs() {
    return !drawingScreens;
}

byte KTOME_Memory::inputCheck() {
    if (isAcceptingInputs()) {
        byte result_byte = hasButtonBeenPushed();

        if (result_byte > 0 && stage < 5) {
            DEBUG_PRINT("Correct responses are: ");
            DEBUG_PRINT(correct_inputs[0]+1);
            DEBUG_PRINT(" ");
            DEBUG_PRINT(correct_inputs[1]+1);
            DEBUG_PRINT(" ");
            DEBUG_PRINT(correct_inputs[2]+1);
            DEBUG_PRINT(" ");
            DEBUG_PRINT(correct_inputs[3]+1);
            DEBUG_PRINT(" ");
            DEBUG_PRINTLN(correct_inputs[4]+1);
            DEBUG_PRINT("input checking... result = ");
            DEBUG_PRINTLN(result_byte);

            drawingScreens = true;
            busy_timer = millis();
            display_stage = 1;
            if (result_byte == correct_inputs[stage]+1) { // Correct input...
                stage++;
                updateStageLEDs();
                DEBUG_PRINT("Correct! Move to stage ");
                DEBUG_PRINTLN(stage);
                if (stage == 5) {
                    module_solved = true;
                    drawingScreens = false;
                    return 2; // Correct and solved
                }
            } else {
                stage = 0;
                DEBUG_PRINT("Incorrect! Move back to stage ");
                DEBUG_PRINTLN(stage + 1);
                updateStageLEDs();
                generate();
                return 1; // Incorrect input
            }
            
        }
    }
    return 0;
}

byte KTOME_Memory::hasButtonBeenPushed() {
    if (touch_flag) {

        DEBUG_PRINTLN("Start fectching touch data...");
        readI2C(IIC_Address, IIC_Reg_2, 35);
        writeI2C(IIC_Address, IIC_Reg_1, 0x00);
        
        uint16_t touch_x = (buffer[2] << 8) + buffer[1];
        uint16_t touch_y = (buffer[4] << 8) + buffer[3];
        // uint16_t touch_p = buffer[5];
        uint16_t touch_e = buffer[6];
        // uint16_t touch_o = buffer[0];
        DEBUG_PRINT("X: ");
        DEBUG_PRINT(touch_x);
        DEBUG_PRINT(", Y: ");
        DEBUG_PRINT(touch_y);
        DEBUG_PRINT(", E: ");
        DEBUG_PRINTLN(touch_e);

        touch_flag = false;

        if ((touch_status == 1 || touch_status == 2) && touch_e == 4) { // No longer touching
            touch_status = touch_e;
        } else if ((touch_status == 4 || touch_status == 0) && (touch_e == 1 || touch_e == 2)) { // Started touching
            touch_status = touch_e;
            DEBUG_PRINT("New touch @ X=");
            DEBUG_PRINT(touch_x);
            DEBUG_PRINT("Y=");
            DEBUG_PRINT(touch_y);
            DEBUG_PRINT(" : ");
            if (touch_x < 296/4 - 1) {
                DEBUG_PRINTLN("Pressed first position!");
                return 1;
            } else if (touch_x < 2*296/4 - 1) {
                DEBUG_PRINTLN("Pressed second position!");
                return 2;
            } else if (touch_x < 3*296/4 - 1) {
                DEBUG_PRINTLN("Pressed third position!");
                return 3;
            } else if (touch_x < 4*296/4 - 1) {
                DEBUG_PRINTLN("Pressed fourth position!");
                return 4;
            }
        }
    }
    return 0;
}

void KTOME_Memory::writeI2C(int8_t addr, int16_t reg, int8_t msg) {
    DEBUG_PRINTLN("Writing to I2C...");
    Wire.beginTransmission(addr);  
    Wire.write(reg >> 8);
    Wire.write(reg & 0xff);
    Wire.write(msg);
    Wire.endTransmission();
}

void KTOME_Memory::readI2C(int8_t addr, int16_t reg, int8_t len) {
    DEBUG_PRINTLN("Reading from I2C...");
    Wire.beginTransmission(addr);  
    Wire.write(reg >> 8);
    Wire.write(reg & 0xff);
    Wire.endTransmission(false);
    Wire.requestFrom(IIC_Address, len);
    Wire.readBytes(buffer, len);
}

bool KTOME_Memory::isSolved() {
	return module_solved;
}

byte KTOME_Memory::update() {
    if (drawingScreens) {
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
                case 4: // Display new prompt on main display
                    DEBUG_PRINTLN("Display stage = 4");
                    drawTopScreen(display_labels[stage]);
                    display_stage = 0;
                    drawingScreens = false;
                    // During display updates, touch is no longer read. If released, this release isn't captured so double tap required. Add extra touch read in here to check if touch still happening...
                    touch_flag = true;
                    break;
                case 5: // Flash on explode
                    drawTopScreen(7);
                    display_stage++;
                    busy_timer += 100;
                    break;
                case 6: // Turn off flash
                    powerTopScreen(false);
                    drawTopScreen(6);
                    display_stage = 0;
                    drawBottomScreen(false);
                    break;
                case 7: // Splash screen start
                    DEBUG_PRINT("Splash 1 @ ");
                    DEBUG_PRINTLN(thistime);
                    drawTopScreen(8);
                    powerTopScreen(true);
                    display_stage++;
                    busy_timer = thistime + 3000;
                    break;
                case 8: // Splash update
                    DEBUG_PRINT("Splash 2 @ ");
                    DEBUG_PRINTLN(thistime);
                    drawTopScreen(9);
                    display_stage++;
                    busy_timer = thistime + 1000;
                    break;
                case 9: // Splash off
                    DEBUG_PRINT("Splash 3 @ ");
                    DEBUG_PRINTLN(thistime);
                    powerTopScreen(false);
                    drawTopScreen(6);
                    display_stage = 0;
                    break;
            }
            // DEBUG_PRINT("Display stage = ");
            // DEBUG_PRINTLN(display_stage);
        }
    }
    return 0;
}

void KTOME_Memory::updateStageLEDs() {
    for (byte ii = 0; ii < 5; ii++) {
        if (ii < stage) {
            fleds_addr->write(ii+1, CRGB::Green);
            // leds[ii].write(true);
        } else {
            fleds_addr->write(ii+1, CRGB::Black);
            // leds[ii].write(false);
        }
    }
}

void KTOME_Memory::explode() {
    for (byte ii = 0; ii < 5; ii++) {
        fleds_addr->blink(ii+1, CRGB::Red, CRGB::Black, 100, 2);
    }
    // powerTopScreen(false);
    // drawTopScreen(6);
    // drawBottomScreen(true);
    display_stage = 5;
    drawingScreens = true;
    busy_timer = millis();
}

void KTOME_Memory::defuse() {
    game_running = false;
}

String KTOME_Memory::outbox() {
    outbox_waiting = false;
    return outbox_msg;
}

bool KTOME_Memory::isOutbox() {
    return outbox_waiting;
}

bool KTOME_Memory::needsIsr() {
    return true;
}

void KTOME_Memory::isrHandler() {
    if (isAcceptingInputs() && !module_solved && game_running) {
        touch_flag = true;
    }
}

TFT_eSPI tft = TFT_eSPI();
TwoWire einktouch = TwoWire(0);
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> einkdisplay = GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)>(GxEPD2_DRIVER_CLASS(/*CS=5*/ PIN_EINK_CS /*25*/, /*DC=*/ PIN_EINK_DC /*26*/, /*RST=*/ PIN_EINK_RST /*27*/, /*BUSY=*/ PIN_EINK_BUSY));