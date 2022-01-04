// #define DEBUG_ON // Comment this to disable Serial monitor

#include <Arduino.h>
#include <KTOME_common.h>
#include <U8g2lib.h>
#include <KTOME_Vent.h>
#include <Cabin_16.h>
#include <ShiftDisplay.h>

#ifdef DEBUG_ON
#define DebugSerial(...) Serial.printf(__VA_ARGS__)
#else 
#define DebugSerial(...)
#endif

KTOME_Vent::KTOME_Vent() {
    
}

void KTOME_Vent::start() {
    switches[0].init(pin_array[0]);
    switches[1].init(pin_array[1]);
    u8g2.begin();
    u8g2.setFont(Cabin_16);
    reset();
}

void KTOME_Vent::reset() {
    clearScreen();
    // module_defused = false;
    msg_stage = 0;
    needy_active = 0;
    accept_input = false;
}

// void KTOME_Vent::gameStart() {
    // #ifdef DEBUG
    // Serial.println("Turn on the screen...");
    // #endif
    // drawScreen();
// }

void KTOME_Vent::generate() {
    DebugSerial("Module awakes...\n");
    needy_active = 2;
    sfx_alert = 1;
    sfx_wake = 1;
    if (random(100) > 90) {
        prompt_type = 1;
    } else {
        prompt_type = 0;
    }
    display_line[0] = word_list[prompt_type];
    display_line[1] = word_list[2];
    display_line[2] = "";
    drawScreen();
    needy_timer = millis() + 40000; // 40 seconds
    accept_input = true;
    DebugSerial("Module awoken.\n");
}

void KTOME_Vent::sleep() {
    DebugSerial("Module sleeps...\n");
    needy_active = 1;
    sfx_alert = 0;
    sfx_wake = 0;
    shift_timer.set("  ");
    shift_timer.show();
    clearScreen();
    needy_timer = millis() + 10000 + random(30000); // Between 10 and 40 seconds sleep
}

byte KTOME_Vent::update() {
    byte return_byte;
    int32_t this_millis = millis();
    if (needy_active == 2) {
        display_timer = (999 + needy_timer - this_millis)/1000;
        DebugSerial("%d\n",display_timer);
        shift_timer.set(display_timer);
        if (display_timer < 10) {
            // shift_timer.setAt(0, '0');
            // shift_timer.setAt(1, '0');
            String disp_low = "0" + String(display_timer);
            shift_timer.set(disp_low);
        }
        shift_timer.show();
        if (display_timer < 11 && sfx_alert == 1) {
            sfx_alert = 2;
        }
    } else {
        shift_timer.set("  ");
        shift_timer.show();
    }

    if (msg_stage > 0 && this_millis > msg_timer) {
        DebugSerial("Stage time reached...\n");
        return_byte = stageChange();
        if (return_byte > 0) {
            return return_byte;
        }
    }

    if (this_millis > needy_timer) {
        DebugSerial("Needy timer runs out...\n");
        if (needy_active == 2) { // Needy is active so this indicates needy timer has run out...
            clearScreen();
            sleep();
            return 1;
        } else if (needy_active == 1) { // Needy is dormant, so wake this module up...
            generate();
        }
    }
    return 0;
}

byte KTOME_Vent::stageChange() {
    switch (msg_stage) {
        case 1:
            display_line[2] = "Y";
            msg_timer = millis() + 666;
            msg_stage ++;
            drawScreen();
            break;
        case 2:
            display_line[2] = "YE";
            msg_timer += 666;
            msg_stage ++;
            drawScreen();
            break;
        case 3:
            display_line[2] = "YES";
            msg_timer += 666;
            msg_stage = 6;
            drawScreen();
            break;
        case 4:
            display_line[2] = "N";
            msg_timer = millis() + 1000;
            msg_stage ++;
            drawScreen();
            break;
        case 5:
            display_line[2] = "NO";
            msg_timer += 1000;
            msg_stage = 6;
            drawScreen();
            break;
        case 6:
            clearScreen();
            byte return_byte;
            return_byte = logicCheck();
            msg_stage = 0;
            if (return_byte == 1) {
                sleep();
                return 2; // Correct
            } else if (return_byte == 2) {
                sleep();
                return 1; // Wrong and strike
            } else { 
                msg_timer += 500;
                msg_stage = 7;
                display_line[0] = word_list[5];
                display_line[1] = word_list[6];
                display_line[2] = word_list[7];
                drawScreen();
                return 0; // Wrong but no strike
            }
            break;
        case 7:
            clearScreen();
            msg_timer += 500;
            msg_stage = 8;
            break;
        case 8:
            msg_timer += 500;
            msg_stage = 9;
            display_line[0] = word_list[5];
            display_line[1] = word_list[6];
            display_line[2] = word_list[7];
            drawScreen();
            break;
        case 9:
            clearScreen();
            msg_timer += 500;
            msg_stage = 10;
            break;
        case 10:
            msg_timer += 1000;
            msg_stage = 11;
            display_line[0] = word_list[5];
            display_line[1] = word_list[6];
            display_line[2] = word_list[7];
            drawScreen();
            break;
        case 11:
            display_line[0] = word_list[prompt_type];
            display_line[1] = word_list[2];
            display_line[2] = "";
            msg_stage = 0;
            drawScreen();
            break;
    }
    return 0;
}

void KTOME_Vent::drawScreen() {
    //clearScreen();
    u8g2.firstPage();  
    do {
        // for (byte ii = 0; ii < 5; ii++) {
            DebugSerial("Drawing...\n");
            u8g2.setCursor((64 - (u8g2.getStrWidth(display_line[0].c_str())/ 2)), 22);
            u8g2.print(display_line[0]);
            u8g2.setCursor((64 - (u8g2.getStrWidth(display_line[1].c_str())/ 2)), 40);
            u8g2.print(display_line[1]);
            u8g2.setCursor((64 - (u8g2.getStrWidth(display_line[2].c_str())/ 2)), 58);
            u8g2.print(display_line[2]);
        // }
    } while( u8g2.nextPage() );
}

void KTOME_Vent::clearScreen() {
    u8g2.clear();
    u8g2.firstPage();  
    do {
        
    } while( u8g2.nextPage() );
}

void KTOME_Vent::inputCheck() {
    DebugSerial("Inputs checked...\n");
    // byte result_byte = 0;
    for (byte ii = 0; ii < 2; ii++)
    {
        if (hasButtonBeenPushed(ii)) { // Submit button
        button_pressed = ii;
        if (ii == 0) {
            msg_stage = 1;
        } else {
            msg_stage = 4;
        }
        // result_byte = logicCheck(ii);
        //     if (result_byte == 1) {
        //         return 2; // Correct
        //     } else if (result_byte == 2) {
        //         return 1; // Wrong and strike
        //     } else { 
                
        //         return 0; // Wrong but no strike
        //     }
        }
    }
    // return 0;
}

bool KTOME_Vent::hasButtonBeenPushed(byte button_number) {
    if (switches[button_number].hasChanged()) {
        if (switches[button_number].isPressed()) {
            DebugSerial("%d button pressed!\n", button_number);
            return true;
        } else {
            DebugSerial("%d button released!\n", button_number);
            return false;
        }
    } else {
        return false;
    }
}

byte KTOME_Vent::logicCheck(){
    DebugSerial("Logic checked...\n");
    if (button_pressed == prompt_type) { // Correct response
        return 1;
    } else if (button_pressed == 0) {
        return 2; // Wrong and strike
    } else {
        return 0; // Wrong, no strike
    }
}

bool KTOME_Vent::isActive() {
	if (needy_active == 2) {
		return true;
	} else {
		return false;
	}
}

bool KTOME_Vent::isAwake() {
	if (needy_active > 0) {
		return true;
	} else {
		return false;
	}
}

void KTOME_Vent::activate() {
    generate();
}

void KTOME_Vent::defused() {
    DebugSerial("Defused method\n");
    // leds.write(false);
}

void KTOME_Vent::explode() {
    DebugSerial("Explode!\n");
    clearScreen();
}

byte KTOME_Vent::sendSound() {
    if (sfx_alert == 2) {
        sfx_alert = 0;
        return 1;
    } else if (sfx_wake == 1) {
        sfx_wake = 0;
        return 2;
    }
    return 0;
}

// U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, VENT_OLED_CLK, VENT_OLED_DAT, VENT_OLED_CS, VENT_OLED_DC, VENT_OLED_RST);  
U8G2_SSD1309_128X64_NONAME0_1_4W_HW_SPI u8g2(U8G2_R0, VENT_OLED_CS, VENT_OLED_DC, VENT_OLED_RST);  
ShiftDisplay shift_timer(VENT_SR_L, VENT_SR_C, VENT_SR_D, COMMON_CATHODE, 2);