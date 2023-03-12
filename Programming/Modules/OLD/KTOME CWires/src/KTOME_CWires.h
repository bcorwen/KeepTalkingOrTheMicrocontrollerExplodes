#ifndef KTOME_CWIRES_H
#define KTOME_CWIRES_H

#include <Arduino.h>
#include <KTOME_common.h>

#define CWires_1     GPIO_NUM_19
#define CWires_2     GPIO_NUM_18
#define CWires_3     GPIO_NUM_5
#define CWires_4     GPIO_NUM_17
#define CWires_5     GPIO_NUM_16
#define CWires_6     GPIO_NUM_4
#define CWires_LED1  GPIO_NUM_32
#define CWires_LED2  GPIO_NUM_33
#define CWires_LED3  GPIO_NUM_25
#define CWires_LED4  GPIO_NUM_14
#define CWires_LED5  GPIO_NUM_12
#define CWires_LED6  GPIO_NUM_13

class KTOME_CWires {
	private:
    bool module_defused;
    int8_t wire_properties[6]; // 1 = White, 2 = Red, 4 = Blue, 8 = LED on, 16 = *, 32 = no wire
    bool wire_connected[6];
    int8_t wire_numbers;
    bool correct_wires[6];
    Switch switches[6];
    Led leds[6];
    int8_t pin_array[12] = {CWires_1, CWires_2, CWires_3, CWires_4, CWires_5, CWires_6, CWires_LED1, CWires_LED2, CWires_LED3, CWires_LED4, CWires_LED5, CWires_LED6};
    int8_t hasWireBeenCut();
    int8_t logicCheck(byte wire_cut);
    int8_t battery_number;
    bool parallel_port;
    bool serial_even;
		
	public:
    void start(); // Initialise Keypad object
    void generate(); // Generate a game (i.e. keypad_setup)
    void reset();
    void findSolution(); // Call after receiving widget info to be able to determine the correct input
	String getManual(); // Sends CAN of keypad symbols, flashes module light before manual check
    bool manualConfirm();
    void widgetSolution(bool parallel_port, byte battery_number, bool serial_even);
    void begin();
    int8_t inputCheck(); // Will check input button to see if right / wrong - Feed in button number, will give a "correct", "wrong" or "no action" answer.
    bool isDefused();
    void explode();
    // void update();
    KTOME_CWires();
		
};

#endif