#include <Wire.h>
#include <CAN.h>

#define PIN_CAN_TX  GPIO_NUM_26
#define PIN_CAN_RX  GPIO_NUM_27

// CAN                      [ Module type ][ID][ unused ]
#define CAN_ID            0b10000000000000000000000000000 // ID for Master (timer)
#define CAN_MASK          0b10000000000000000000000000000 // Filter for the Master (timer)

#define CAN_TO_MASTER     0b10000000000000000000000000000
#define CAN_TO_WIRES      0b01000000000000000000000000000
#define CAN_TO_BUTTON     0b00100000000000000000000000000
#define CAN_TO_KEYPAD     0b00010000000000000000000000000
#define CAN_TO_SIMON      0b00001000000000000000000000000
#define CAN_TO_WHOS       0b00000100000000000000000000000
#define CAN_TO_MEMORY     0b00000010000000000000000000000
#define CAN_TO_MORSE      0b00000001000000000000000000000
#define CAN_TO_CWIRES     0b00000000100000000000000000000
#define CAN_TO_WIRESQ     0b00000000010000000000000000000
#define CAN_TO_MAZE       0b00000000001000000000000000000
#define CAN_TO_PASSWORD   0b00000000000100000000000000000
#define CAN_TO_VENT       0b00000000000010000000000000000
#define CAN_TO_CAPACITOR  0b00000000000001000000000000000
#define CAN_TO_KNOB       0b00000000000000100000000000000
#define CAN_TO_STD_MOD    0b01111111111100000000000000000
#define CAN_TO_NEEDY_MOD  0b00000000000011100000000000000
#define CAN_TO_ALL_MOD    0b01111111111111100000000000000
#define CAN_MUID_1        0b00000000000000010000000000000
#define CAN_MUID_2        0b00000000000000001000000000000
#define CAN_MUID_3        0b00000000000000000100000000000
#define CAN_MUID_4        0b00000000000000000010000000000
#define CAN_MUID_ALL      0b00000000000000011110000000000

#define CAN_MANUALSETUP   0b01110000110000000000000000000

char buffer_msg[9][16];
int buffer_can_id[16];
byte buffer_pointer_r;
byte buffer_pointer_w;
char msg_received[9];
int msg_rec_id;

void setup() {
  
  // Start CAN bus
  CAN.setPins(PIN_CAN_RX, PIN_CAN_TX);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  //  CAN.filterExtended(CAN_ID, CAN_MASK);
  CAN.onReceive(onReceive); // Set-up interrupt
  
}

void loop() {

  char CAN_message[7];
  CAN_message[0] = 'H';
  CAN_message[1] = 'e';
  CAN_message[2] = 'l';
  CAN_message[3] = 'l';
  CAN_message[4] = 'o';
  CAN_message[5] = '!';
  CAN_message[6] = '\0';
  CANSend((CAN_TO_ALL_MOD | CAN_MUID_ALL), CAN_message, 6); // Send a 7-byte message, but we don't need to send the string terminator byte so message length variable is one less than message length.

  delay(1000);
   
}

void CANSend(int id, char msg_data[], byte msg_len) {
  Serial.print("Sending packet ... ");
  digitalWrite(PIN_LED, HIGH);

  CAN.beginExtendedPacket(id);
  for (int i = 0; i < msg_len; i++) {
    CAN.write(msg_data[i]);
  }
  //  CAN.write(msg_data, msg_len);
  CAN.endPacket();

  Serial.print(" with id: 0b");
  padZerosCAN(id);
  Serial.print(id, BIN);
  Serial.print(" - \"");
  Serial.print(msg_data);
  Serial.println("\"");
  digitalWrite(PIN_LED, LOW);
}

void onReceive(int packet_size) {
  if ((CAN.packetId() & CAN_MASK) == CAN_ID) { // Manual filter for messages - the CAN library has a filter function but this appears not to work for extended messages, so this is a manual work-around.
    for (byte i = 0; i < packet_size; i++) {
      //    msg_received[i] = CAN.read();
      buffer_msg[i][buffer_pointer_w] = CAN.read();
    }
    //  msg_received[packet_size] = '\0';
    buffer_msg[packet_size][buffer_pointer_w] = '\0';

    buffer_can_id[buffer_pointer_w] = CAN.packetId();

    if (buffer_pointer_w == 15) {
      buffer_pointer_w = 0;
    } else {
      buffer_pointer_w++;
    }
  }
}

void CANReceive() {
  Serial.print("Read pointer: ");
  Serial.println(buffer_pointer_r);
  Serial.print("Write pointer: ");
  Serial.println(buffer_pointer_w);
  if (buffer_pointer_r != buffer_pointer_w) { // Outstanding messages to handle
    for (byte ii = 0; ii < 9; ii++) {
      msg_received[ii] = buffer_msg[ii][buffer_pointer_r];
    }
    msg_rec_id = buffer_can_id[buffer_pointer_r];

    if (buffer_pointer_r == 15) {
      buffer_pointer_r = 0;
    } else {
      buffer_pointer_r++;
    }

    Serial.print("Received ");
    Serial.print("packet with id 0b");
    padZerosCAN(msg_rec_id);
    Serial.print(msg_rec_id, BIN);

    Serial.print(" - \"");
    Serial.print(msg_received);
    Serial.println("\"");

    if (msg_received[0] == 'p' && gamemode == 1) {
      module_detected = true;
      Serial.println("Module detected!");
    }
    else if (msg_received[0] == 'i' && gamemode == 2) {
      module_inited = true;
      Serial.println("Module has declare it is setup!");
    }
    else if (msg_received[0] == 'c' && gamemode == 2) {
      Serial.println("Module is transmitting it's manual setup needs!");
    }
  }
}

void padZerosCAN(int id) {
  for (byte ii = 28; ii > 0; ii--) {
    if (id < (1 << ii)) {
      Serial.print("0");
    }
  }
}
