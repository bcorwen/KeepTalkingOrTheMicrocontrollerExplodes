#ifndef KTOME_CAN_H
#define KTOME_CAN_H

#include <Wire.h>
#include <CAN.h>

#define CAN_MASTER        0b10000000000000000000000000000
#define CAN_WIDGETS		  0b00000000000000000000000000000
#define CAN_WIRES         0b01000000000000000000000000000
#define CAN_BUTTON        0b00100000000000000000000000000
#define CAN_KEYPAD        0b00010000000000000000000000000
#define CAN_SIMON         0b00001000000000000000000000000
#define CAN_WHOS          0b00000100000000000000000000000
#define CAN_MEMORY        0b00000010000000000000000000000
#define CAN_MORSE         0b00000001000000000000000000000
#define CAN_CWIRES        0b00000000100000000000000000000
#define CAN_WIRESQ        0b00000000010000000000000000000
#define CAN_MAZE          0b00000000001000000000000000000
#define CAN_PASSWORD      0b00000000000100000000000000000
#define CAN_VENT          0b00000000000010000000000000000
#define CAN_CAPACITOR     0b00000000000001000000000000000
#define CAN_KNOB          0b00000000000000100000000000000
#define CAN_STD_MOD       0b01111111111100000000000000000
#define CAN_NEEDY_MOD     0b00000000000011100000000000000
#define CAN_ALL_MOD       0b01111111111111100000000000000
#define CAN_MUID_1        0b00000000000000010000000000000
#define CAN_MUID_2        0b00000000000000001000000000000
#define CAN_MUID_3        0b00000000000000000100000000000
#define CAN_MUID_4        0b00000000000000000010000000000
#define CAN_MUID_5        0b00000000000000000001000000000
#define CAN_MUID_6        0b00000000000000000000100000000
#define CAN_MUID_7        0b00000000000000000000010000000
#define CAN_MUID_8        0b00000000000000000000001000000
#define CAN_MUID_9        0b00000000000000000000000100000
#define CAN_MUID_10       0b00000000000000000000000010000
#define CAN_MUID_11       0b00000000000000000000000001000
#define CAN_MUID_ALL      0b00000000000000011111111111000
#define CAN_MANUALSETUP   0b01110000110000000000000000000


class KTOME_CAN {
	private:
		bool init();
		
		char buffer_msg[9][16];
		uint32_t  buffer_msg_id[16];
		byte buffer_r;
		byte buffer_w;
		uint8_t pin_tx;
		uint8_t pin_rx;
		
	public:
		bool start();
		bool start(uint8_t rx, uint8_t tx);
		void id(uint32_t id, uint32_t mask);
		void send(uint32_t id, char msg_data[], byte msg_len);
		void receive();
		void handleISR(int packet_size);
		bool messageWaiting();
		void padZeros(uint32_t id);
		
		byte peek_buffer_r();
		byte peek_buffer_w();
		uint32_t can_id;
		uint32_t can_mask;
		char can_msg[9];
		uint32_t can_msg_id;		
};

extern KTOME_CAN ktomeCAN;

void onReceive(int packet_size);
void CANInbox();

#endif