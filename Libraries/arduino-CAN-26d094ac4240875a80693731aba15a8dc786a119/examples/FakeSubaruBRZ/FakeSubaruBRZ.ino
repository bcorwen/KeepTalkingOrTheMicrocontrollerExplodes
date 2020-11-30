#include <CAN.h>

// This is a demo program that sends messages over the CAN bus in
// a way that resembles real messages you can receive if you listen
// to messages on the CAN bus of a Subaru BRZ.
//
// It was tested on Arduino Uno, Arduino Micro, Adafruit Feather nRF52832 and
// Adafruit ItsyBitsy nRF52840 Express, and should be trivial to tweak to
// support pretty much any other board with SPI.
//
// Connections:
//  BOARD | MCP2515
//     5V   VCC
//    GND   GND
//  Pin 7   CS
//  Pin 9   INT  // Unconnected is fine too, we don't use interrupts here.
//     MO   SI
//     MI   SO
//    SCK   SCK

const int CS_PIN = 7;
const int IRQ_PIN = 9;
const int QUARTZ_MHZ = 16;  // Some MCP2515 boards have 8 MHz quartz.
const int SPI_MHZ = 8;

void setup() {
  Serial.begin(115200);

  uint32_t startTimeMs = millis();
  while (!Serial && millis() - startTimeMs < 1000);
  if (!Serial) {
    Serial.println("Started!");
  } else {
    // Whatever, noone's going to see anyways.
  }

  CAN.setClockFrequency(QUARTZ_MHZ * 1E6);
  CAN.setSPIFrequency(SPI_MHZ * 1E6);
  CAN.setPins(CS_PIN, IRQ_PIN);

  // Subaru BRZ uses a 500k baud rate.
  while (!CAN.begin(500000)) {
    Serial.println("Failed to connect to the CAN controller!");
    delay(1000);
  }

  Serial.println("CAN controller connected");
}

// Forward declaration for a helper.
void generate_payload(uint16_t pid, uint8_t payload);

void loop() {
  // Many PIDs are sent 50 times per second.
  uint16_t num_cycles_per_second = 50;

  // 0x140 and 0x141 are intentionally duplicated in this array as they are sent 100 times
  // per second (double that for other PIDs) in the real car.
  // TODO: 0x360 is actually sent 20 times per second.
  uint16_t pids[] = {0x140, 0x141, 0xD0, 0xD1, 0xD4, 0xD5, 0x140, 0x141, 0x360};
  uint16_t num_messages_per_cycle = sizeof(pids) / sizeof(pids[0]);
  uint16_t num_messages_per_second =
      num_cycles_per_second * num_messages_per_cycle;

  unsigned long first_message_sent_micros = micros();
  unsigned long num_messages_sent = 0;
  for (int i = 0; i < num_messages_per_cycle; i++, num_messages_sent++) {
    uint16_t pid = pids[i];
    if (!CAN.beginPacket(pid)) {
      Serial.println("beginPacket() failed.");
    }

    uint8_t payload[8];
    generate_payload(pid, payload);
    CAN.write(payload, 8);
    if (!CAN.endPacket()) {
      Serial.println("endPacket() failed.");
    }

    unsigned long next_message_time_micros =
        first_message_sent_micros
            + (num_messages_sent * 1000000) / num_messages_per_second;
    unsigned long time_till_next_message_micros =
        next_message_time_micros - micros();
    // Theoretically, the clock can be ticking faster than we can send data.
    // The overflow-safe math is tricky.
    if ((long)time_till_next_message_micros > 0) {
      delayMicroseconds(time_till_next_message_micros);
    }
  }
}

void generate_payload(uint16_t pid, uint8_t *payload) {
  memset(payload, /* value= */ 0, /* size= */ 8);

  switch (pid) {
    case 0xD0: {
      // 0xD0 contains the steering wheel angle and data from motion sensors.

      // Pretend that the steering wheel is turned by 123 degrees to the left
      int16_t steering_angle_degrees = -123;
      int16_t value = steering_angle_degrees * 10;
      payload[0] = value & 0xFF;
      payload[1] = (value >> 8) & 0xFF;

      // TODO: Verify the scale for this value. The current scale is suspicious,
      // but matches real-world testing so far. Need to go to a skid pad to
      // verify for sure.
      int16_t rotation_clockwise_degrees_per_second = -70;
      int16_t rotation_clockwise_radians_per_second_x180 =
          (int16_t)(-3.14159 * rotation_clockwise_degrees_per_second);
      payload[2] = rotation_clockwise_radians_per_second_x180 & 0xFF;
      payload[3] = (rotation_clockwise_radians_per_second_x180 >> 8) & 0xFF;

      // TODO: decode what's in payload[4] and payload[5].

      float lateral_acceleration_g = 0.3;
      // Looks to be encoded in a way that +1 increment is +0.2 m/s2.
      payload[6] = (int8_t)(9.80665 * lateral_acceleration_g / 0.2);

      float longitudinal_acceleration_g = 0.2;
      // Looks to be encoded in a way that +1 increment is -0.1 m/s2.
      // I know, it's strange that they use different scales for lat vs long.
      payload[7] = (int8_t)(-9.80665 * longitudinal_acceleration_g / 0.1);
      break;
    }

    case 0xD1: {
      // 0xD1 contains the speed, and the master brake cylinder pressure.
      uint16_t speed_m_s = 10;  // 36 km/h, ~22.4 mph.
      // The encoding seems to be roughly radians per second x100.
      // The coefficient was tuned by comparing the values against an external
      // GPS from a session // where I drove in a straight line on a highway at
      // constant speed on cruise control.
      uint16_t value = (uint16_t)(10 * 63.72);
      payload[0] = value & 0xFF;
      payload[1] = (value >> 8) & 0xFF;

      // The units used for the master brake cylinder pressure are believed to
      // be bars.
      uint8_t brake_pressure_bar = 50;
      payload[2] = brake_pressure_bar;
      break;
    }

    case 0xD4: {
      // Wheel speed sensors / ABS sensors.
      // TODO: fill out similar to speed in 0xD1.
      // They are in the FL, FR, RL, RR order.
    }

    case 0x140: {
      uint8_t accelerator_pedal_percent = 42;
      payload[0] = accelerator_pedal_percent * 255 / 100;

      // The clutch pedal has two sensors:
      // - 0% and >0% (used here)
      // - 100% and <100% (haven't found yet)
      // TODO: Find where data from the second sensor is.
      bool clutch_down = false;
      payload[1] = (clutch_down ? 0x80 : 0x00);

      // RPMs are believed to be encoded with just 14 bits.
      uint16_t rpm = 3456;
      payload[2] = rpm & 0xFF;
      payload[3] = (rpm >> 8) & 0x3F;
    }

    case 0x360: {
      uint8_t oil_temperature_celsius = 100;
      payload[2] = oil_temperature_celsius + 40;

      uint8_t coolant_temperature_celsius = 90;
      payload[3] = coolant_temperature_celsius + 40;
    }
  }
}
