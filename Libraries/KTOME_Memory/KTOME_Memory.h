#ifndef KTOME_MEMORY_H
#define KTOME_MEMORY_H

#include <Arduino.h>
#include <KTOME_common.h>
#include <SPI.h>
#include <TFT_eSPI.h>
// #include <xbm_1.h>
// #include <xbm_2.h>
// #include <xbm_3.h>
// #include <xbm_4.h>
// #include <Mem_1a.h>
// #include <Mem_2a.h>
// #include <Mem_3a.h>
// #include <Mem_4a.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
// #include <GxEPD2_display_selection_new_style.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Wire.h>

// Defines for e-ink display
#define ENABLE_GxEPD2_GFX   0
#define IIC_Address		    0x48
#define IIC_Reg_1           0x1001
#define IIC_Reg_2           0x1002
#define TP_SCL_PIN          22
#define TP_SDA_PIN          21
#define TP_RST_PIN          17
#define TP_INT_PIN          16
#define PIN_EINK_DIN  GPIO_NUM_23
#define PIN_EINK_CLK  GPIO_NUM_18
#define PIN_EINK_CS   GPIO_NUM_25
#define PIN_EINK_DC   GPIO_NUM_32
#define PIN_EINK_RST  GPIO_NUM_33
#define PIN_EINK_BUSY GPIO_NUM_4

// Defines for TFT display
#define ILI9341_BACKGROUND  0x0283
#define TFT_POWER           13

// Other Defines
#define MEMORY_LED1   12
#define MEMORY_LED2   12
#define MEMORY_LED3   12
#define MEMORY_LED4   12
#define MEMORY_LED5   12

class KTOME_Memory {
	private:
        Led leds[5];
        byte pin_array[5] = {MEMORY_LED1, MEMORY_LED2, MEMORY_LED3, MEMORY_LED4, MEMORY_LED5};
        byte button_labels[5][4] = {
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
        };
        byte button_positions[5][4] = {
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
        };
        byte display_labels[5] = {1,1,1,1,1};
        byte correct_inputs[5] = {1,1,1,1,1};
        bool module_defused;
        byte stage = 0;
        bool isbusy;
        int32_t busy_timer;
        byte display_stage;
        byte hasButtonBeenPushed();
        bool logicCheck();
        void updateStageLEDs();
        void drawTopScreen(byte displaynumber);
        void drawBottomScreen(bool full_update);
        void writeI2C(int8_t addr, int16_t reg, int8_t msg);
        void readI2C(int8_t addr, int16_t reg, int8_t len);
        void powerTopScreen(bool power_state);
        uint16_t colorWhite = GxEPD_WHITE;
        uint16_t colorBlack = GxEPD_BLACK;
        uint8_t buffer[100];
        volatile bool touch_flag;
        byte touch_status = 4;
        byte top_display_number = 0;

	public:
        void start(); // Initialise Keypad object
        void generate(); // Generate a game (i.e. keypad_setup)
        void reset();
        void begin();
        void isrhandler();
        byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
        void updateDisplays();
        bool isAcceptingInputs();
        bool isDefused();
        void explode();
        KTOME_Memory();
		
};

extern TFT_eSPI tft;
extern TwoWire einktouch;
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

#endif