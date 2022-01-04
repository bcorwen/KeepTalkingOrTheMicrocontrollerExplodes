#ifndef KTOME_CAN_H
#define KTOME_CAN_H

#include <Arduino.h>
#include <Wire.h>
#include <CAN.h>

class CAN_IDS {
	public:
		int32_t Master 			= 0b10000000000000000000000000000;
		int32_t Widgets 		= 0b00000000000000000000000000000;
		int32_t Wires 			= 0b01000000000000000000000000000;
		int32_t Button 			= 0b00100000000000000000000000000;
		int32_t Keypad 			= 0b00010000000000000000000000000;
		int32_t Simon 			= 0b00001000000000000000000000000;
		int32_t Whos 			= 0b00000100000000000000000000000;
		int32_t Memory 			= 0b00000010000000000000000000000;
		int32_t Morse 			= 0b00000001000000000000000000000;
		int32_t CWires 			= 0b00000000100000000000000000000;
		int32_t WireSeq 		= 0b00000000010000000000000000000;
		int32_t Maze 			= 0b00000000001000000000000000000;
		int32_t Password 		= 0b00000000000100000000000000000;
		int32_t Vent 			= 0b00000000000010000000000000000;
		int32_t Capacitor 		= 0b00000000000001000000000000000;
		int32_t Knob 			= 0b00000000000000100000000000000;
		int32_t All_standards 	= 0b01111111111100000000000000000;
		int32_t All_needys 		= 0b00000000000011100000000000000;
		int32_t All_modules 	= 0b01111111111111100000000000000;
		int32_t All_manuals 	= 0b01110000110000000000000000000;
		int32_t MUID_01 		= 0b00000000000000000000000000001;
		int32_t MUID_02 		= 0b00000000000000000000000000010;
		int32_t MUID_03 		= 0b00000000000000000000000000011;
		int32_t MUID_04 		= 0b00000000000000000000000000100;
		int32_t MUID_05 		= 0b00000000000000000000000000101;
		int32_t MUID_06 		= 0b00000000000000000000000000110;
		int32_t MUID_07 		= 0b00000000000000000000000000111;
		int32_t MUID_08 		= 0b00000000000000000000000001000;
		int32_t MUID_09 		= 0b00000000000000000000000001001;
		int32_t MUID_10 		= 0b00000000000000000000000001010;
		int32_t MUID_11 		= 0b00000000000000000000000001011;
		int32_t MUID_12 		= 0b00000000000000000000000001100;
		int32_t MUID_13 		= 0b00000000000000000000000001101;
		int32_t MUID_14 		= 0b00000000000000000000000001110;
		int32_t MUID_15 		= 0b00000000000000000000000001111;
		
};

class KTOME_CAN {
	private:
		bool init();
		char buffer_msg[9][16];
		uint32_t  buffer_msg_id[16];
		byte buffer_r;
		byte buffer_w;
		uint8_t pin_tx;
		uint8_t pin_rx;
		uint32_t can_id;
		uint32_t can_mask;
		
	public:
		bool start();
		bool start(uint8_t rx, uint8_t tx);
		void setId(uint32_t id, uint32_t mask);
		void setId(uint32_t id);
		void send(uint32_t id, char msg_data[], byte msg_len);
		void receive();
		void handleISR(int packet_size);
		bool isMessageWaiting();
		void padZeros(uint32_t id);
		byte peek_buffer_r();
		byte peek_buffer_w();
		char can_msg[9];
		uint32_t can_msg_id;
		KTOME_CAN();
};

extern CAN_IDS can_ids;
extern KTOME_CAN ktomeCAN;
void onReceive(int packet_size);

#endif