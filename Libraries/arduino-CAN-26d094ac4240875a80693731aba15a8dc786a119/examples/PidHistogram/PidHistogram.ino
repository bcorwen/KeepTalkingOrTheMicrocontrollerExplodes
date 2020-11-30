#include <CAN.h>

// This is a demo program that listens to messages on the CAN bus and
// periodically prints a list of PIDs observed, with counts how many times they
// were observed.
//
// It was tested on Arduino Uno, Arduino Micro, Adafruit Feather nRF52832 and
// Adafruit ItsyBitsy nRF52840 Express, and should be trivial to tweak to
// support pretty much any other board with SPI.
//
// Connections:
//  BOARD | MCP2515
//    USB   VCC
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

// Defines the maximum number of unique CAN PIDs to keep track of between
// printing histrogram reports. Too many will make the output unreadable, too
// few will not give you the full picture.
const uint16_t MAX_NUM_CAN_PIDS = 32;

// Interval in seconds between printing reports.
const uint32_t REPORT_INTERVAL_SECONDS = 1;

struct pid_entry {
  uint32_t pid;
  uint32_t num_received_since_last_report;
};
pid_entry observed_pids[MAX_NUM_CAN_PIDS]; // Keep sorted by PID.
uint16_t num_unique_observed_pids = 0;

uint32_t last_report_printed_ms;

void setup() {
  Serial.begin(115200);

  uint32_t startTimeMs = millis();
  while (!Serial);

  Serial.println("Started!");

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

// Forward declarations for helper functions.
void handle_message(uint32_t pid);
void print_report();

void loop() {
  if (millis() - last_report_printed_ms > REPORT_INTERVAL_SECONDS * 1000) {
    print_report();
    last_report_printed_ms = millis();
  }

  int packet_size = CAN.parsePacket();
  if (packet_size <= 0) {
    return;
  }

  if (CAN.packetRtr()) {
    // Ignore RTRs for now.
    return;
  }

  handle_message(CAN.packetId());
}

void handle_message(uint32_t pid) {
  pid_entry *after_last = observed_pids + num_unique_observed_pids;

  // Use binary search to find the location where this PID should go in the
  // sorted 'observed_pids' list.
  // Unfortunately, it's not simple to use std::lower_bound() on Ardiuno AVR
  // boards, so I had to recall some CS classes. Hopefully, no bugs :)
  pid_entry *insertion_location = nullptr;
  {
    pid_entry *start_ptr = observed_pids;
    pid_entry *end_ptr = after_last;

    while (start_ptr != end_ptr) {
      if (start_ptr->pid >= pid) {
        insertion_location = start_ptr;
        break;
      }

      // We can now assume start_ptr->pid < pid.

      if (start_ptr + 1 == end_ptr) {
        // Narrowed the range down to one element that's smaller than pid.
        insertion_location = start_ptr + 1;
        break;
      }

      // Otherwise, keep narrowing down. This is guaranteed to make progress
      // as start_pid + 1 < end_ptr, and so (end_ptr - start_ptr) >= 2.
      pid_entry *middle = start_ptr + (end_ptr - start_ptr) / 2;
      if (middle->pid <= pid) {
        start_ptr = middle;
      } else {
        end_ptr = middle;
      }
    }

    // Handle the special case of an empty list.
    if (insertion_location == nullptr) {
      insertion_location = start_ptr;
    }
  }

  if (insertion_location < after_last && insertion_location->pid == pid) {
    // Found a match!
    insertion_location->num_received_since_last_report++;
    return;
  } else if (num_unique_observed_pids == MAX_NUM_CAN_PIDS) {
    // The list is already full, ignoring this PID.
    return;
  }

  // Move elements after insert_to_index by one element. Need to use
  // memmove() instead of memcpy() as src and dst overlap.
  memmove(
      /* destination= */ insertion_location + 1,
      /* source= */ insertion_location,
      /* num_bytes= */ (after_last - insertion_location) * sizeof(pid_entry));

  // Finally, insert the new entry for this PID.
  num_unique_observed_pids++;
  insertion_location->pid = pid;
  insertion_location->num_received_since_last_report = 1;
}

void print_report() {
  if (num_unique_observed_pids == 0) {
    Serial.println("No messages received!");
    return;
  }

  Serial.print("Number of unique observed PIDs since last report: ");
  Serial.print(num_unique_observed_pids);
  Serial.println(".");

  for (uint16_t i = 0; i < num_unique_observed_pids; i++) {
    pid_entry *entry = &observed_pids[i];
    Serial.print("  PID: ");
    Serial.print(entry->pid);
    Serial.print(" (0x");
    Serial.print(entry->pid, HEX);
    Serial.print(") received ");
    Serial.print(entry->num_received_since_last_report);
    Serial.println(" times.");

    // Reset the count for the next report.
    entry->num_received_since_last_report = 0;
  }
  Serial.print("Interval between reports: ");
  Serial.print(REPORT_INTERVAL_SECONDS);
  Serial.print(" second");
  if (REPORT_INTERVAL_SECONDS != 1) {
    Serial.print("s");
  }
  Serial.println(".");
  Serial.println("");

  num_unique_observed_pids = 0;
}
