#ifndef KTOME_MEMORY_H
#define KTOME_MEMORY_H

// Defines for TFT - pre-library import
#define USER_SETUP_LOADED // The following is taken from the user_setup.h file which is disabled with this define, allowing the following to be placed here:
#define ILI9341_DRIVER
#define TFT_MISO  19 //19
#define TFT_MOSI  23 //23
#define TFT_SCLK  18 //18
#define TFT_CS    5 // 5
#define TFT_DC    17 //32
#define TFT_RST   16 //33
#define LOAD_GLCD
#define LOAD_FONT2 // Remove?
#define LOAD_FONT4 // Remove?
#define LOAD_FONT6 // Remove?
#define LOAD_FONT7 // Remove?
#define LOAD_FONT8 // Remove?
#define LOAD_GFXFF
#define SMOOTH_FONT // Remove?
#define SPI_FREQUENCY  20000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000 // Remove?

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
#define TP_SCL_PIN          GPIO_NUM_22  // GPIO_NUM_22
#define TP_SDA_PIN          GPIO_NUM_21  // GPIO_NUM_21
#define TP_RST_PIN          GPIO_NUM_33  // GPIO_NUM_17
#define TP_INT_PIN          GPIO_NUM_32  // GPIO_NUM_16
#define PIN_EINK_DIN        GPIO_NUM_23  // GPIO_NUM_23
#define PIN_EINK_CLK        GPIO_NUM_18  // GPIO_NUM_18
#define PIN_EINK_CS         GPIO_NUM_15  // GPIO_NUM_25
#define PIN_EINK_DC         GPIO_NUM_17  // GPIO_NUM_32
#define PIN_EINK_RST        GPIO_NUM_16  // GPIO_NUM_33
#define PIN_EINK_BUSY       GPIO_NUM_4  // GPIO_NUM_4

// Defines for TFT display
#define ILI9341_BACKGROUND  0x0283
#define TFT_POWER           27  //12

class KTOME_Memory {
	private:
        // Standard
        byte strike_count;
        bool outbox_waiting;
        String outbox_msg;
        bool serial_vowels;
        bool serial_odd;
        byte battery_number;
        bool ind_car;
        bool ind_frk;
        bool port_s;
        bool port_p;
        byte logicCheck();

        // Specific
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
        byte stage = 0;
        bool drawingScreens;
        int32_t busy_timer;
        byte display_stage;
        uint16_t colorWhite = GxEPD_WHITE;
        uint16_t colorBlack = GxEPD_BLACK;
        uint8_t buffer[100];
        volatile bool touch_flag;
        byte touch_status = 4;
        byte top_display_number = 0;
        FLedPWM *fleds_addr;
        bool isAcceptingInputs();
        byte hasButtonBeenPushed();
        void updateStageLEDs();
        void drawTopScreen(byte displaynumber);
        void drawBottomScreen(bool full_update);
        void writeI2C(int8_t addr, int16_t reg, int8_t msg);
        void readI2C(int8_t addr, int16_t reg, int8_t len);
        void powerTopScreen(bool power_state);

	public:
        // Standard
        bool game_running;
        bool module_solved;
        byte intPin = TP_INT_PIN;
        void start(FLedPWM *fleds); // Initialise Button object
        void powerOn();
        void generate(); // Generate a game
        void reset();
        bool isManual();
        byte manualConfirm();
        byte timerUpdate(String timer_digits);
        void widgetUpdate(bool vowels, bool odds, byte batts, bool cars, bool frks, bool serials, bool parallels);
        void strikeUpdate(byte strikes);
        String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
        void gameStart();
        byte inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
        bool isSolved();
        void explode();
        void defuse();
        String outbox();
        bool isOutbox();
        byte update();
        bool needsIsr();
        void isrHandler();

        // Specific
        KTOME_Memory();
		
};

extern TFT_eSPI tft;
extern TwoWire einktouch;
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> einkdisplay;

#endif