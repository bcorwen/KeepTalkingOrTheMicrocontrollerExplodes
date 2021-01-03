#include <Wire.h>
#include <CAN.h>
#include <KTOME_CAN.h>

bool KTOME_CAN::start() {
	bool success;
	pin_rx = 27;
	pin_tx = 26;
	success = init();

	return success;
}

bool KTOME_CAN::start(uint8_t rx, uint8_t tx) {
	bool success;
	pin_rx = rx;
	pin_tx = tx;
	success = init();

	return success;
}

void KTOME_CAN::id(uint32_t id, uint32_t mask) {
	can_id = id;
	can_mask = mask;
}

bool KTOME_CAN::init() {
	CAN.setPins(pin_rx, pin_tx);
	if (!CAN.begin(500E3)) {
		// CAN failed to start
		return false;
	}
	CAN.onReceive(onReceive);
	return true;
}

void KTOME_CAN::send(uint32_t id, char msg_data[], byte msg_len) {
	
	CAN.beginExtendedPacket(id);
	for (int i = 0; i < msg_len; i++) {
		CAN.write(msg_data[i]);
	}
	CAN.endPacket();
  
	Serial.print("Sending packet ... ");
	Serial.print(" with id: 0b");
	padZeros(id);
	Serial.print(id, BIN);
	Serial.print(" - \"");
	Serial.print(msg_data);
	Serial.println("\"");
  
}

void onReceive(int packet_size) {
	ktomeCAN.handleISR(packet_size);
}

void KTOME_CAN::handleISR(int packet_size) {
  if ((CAN.packetId() & can_mask) == can_id) {
    for (byte i = 0; i < packet_size; i++) {
      buffer_msg[i][buffer_w] = CAN.read();
    }
    buffer_msg[packet_size][buffer_w] = '\0';
    buffer_msg_id[buffer_w] = CAN.packetId();
    if (buffer_w == 15) {
      buffer_w = 0;
    } else {
      buffer_w++;
    }
  }
}

void KTOME_CAN::receive() {
		for (byte ii = 0; ii < 9; ii++) {
			can_msg[ii] = buffer_msg[ii][buffer_r];
		}
		can_msg_id = buffer_msg_id[buffer_r];
		if (buffer_r == 15) {
			buffer_r = 0;
		} else {
			buffer_r++;
		}
		
		Serial.print("Received ");
		Serial.print("packet with id 0b");
		padZeros(can_msg_id);
		Serial.print(can_msg_id, BIN);
		Serial.print(" - \"");
		Serial.print(can_msg);
		Serial.println("\"");

}

bool KTOME_CAN::messageWaiting() {
	if (buffer_r != buffer_w) {
		return true;
	} else {
		return false;
	}
}

byte KTOME_CAN::peek_buffer_r() {
	return buffer_r;
}

byte KTOME_CAN::peek_buffer_w() {
	return buffer_w;
}

void KTOME_CAN::padZeros(uint32_t id) {
  for (byte ii = 28; ii > 0; ii--) {
    if (id < (1 << ii)) {
      Serial.print("0");
    }
  }
}

KTOME_CAN ktomeCAN = KTOME_CAN();