//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 11/01/21
//======================================================================
//
//  Module: Widgets (SLAVE)
//
//  version 0.5.0
//
//  Goal for this version: Have e-ink paper display serial from timer
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <CAN.h>
#include <KTOME_CAN.h>
#include <Adafruit_GFX.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "SoundFile.h"
#include <XT_DAC_Audio.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
#define PIN_LED       GPIO_NUM_12
#define PIN_EINK_DIN  GPIO_NUM_23
#define PIN_EINK_CLK  GPIO_NUM_18
#define PIN_EINK_CS   GPIO_NUM_5
#define PIN_EINK_DC   GPIO_NUM_22
#define PIN_EINK_RST  GPIO_NUM_21
#define PIN_EINK_BUSY GPIO_NUM_4

// CAN
#define CAN_ID            CAN_WIDGETS
#define CAN_MASK          CAN_WIDGETS

GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT> display(GxEPD2_290c(PIN_EINK_CS, PIN_EINK_DC, PIN_EINK_RST, PIN_EINK_BUSY));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

uint16_t colorWhite = GxEPD_WHITE;
uint16_t colorBlack = GxEPD_BLACK;
uint16_t colorRed   = GxEPD_RED;

XT_DAC_Audio_Class DacAudio(25, 0);
XT_Wav_Class SoundTick(TickWav);
XT_Wav_Class SoundTock(TockWav);
XT_Wav_Class SoundStrike(StrikeWav);
XT_Wav_Class SoundExpl(ExplWav);

char serial_number[] = "XXXXXX";
bool serial_inbox = false;
byte strike_number;
long thismillis;
long tickmillis;
long tockdelays[3] = {667, 600, 1000};
bool tockdone = true;
bool tickdone = false;
long tickprotect;
bool game_running = false;

//**********************************************************************
// E-PAPER DISPLAY
//**********************************************************************

void draw_serial() {
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setFontDirection(1);

  display.firstPage();
  do
  {
    display.fillScreen(colorWhite);
    u8g2Fonts.setForegroundColor(colorBlack);
    u8g2Fonts.setBackgroundColor(colorWhite);
    //u8g2Fonts.setFont(u8g2_font_inb53_mf);
    u8g2Fonts.setFont(u8g2_font_AnonPro_tf);
    u8g2Fonts.setCursor(16, 148 - (u8g2Fonts.getUTF8Width(serial_number) / 2));
    u8g2Fonts.print(serial_number);
    display.fillRect(76, 0, 52, 296, colorRed);
    u8g2Fonts.setForegroundColor(colorWhite);
    u8g2Fonts.setBackgroundColor(colorRed);
    u8g2Fonts.setFont(u8g2_font_LibSans_tf);
    u8g2Fonts.setCursor(90, 148 - (u8g2Fonts.getUTF8Width("SERIAL #") / 2));
    u8g2Fonts.print("SERI");
    u8g2Fonts.setCursor(90, 154);
    u8g2Fonts.print("AL #");

  }

  while (display.nextPage());
  delay(1000);
}

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************

void setup() {
  // Start serial connection
  Serial.begin(115200);
  while (!Serial);
  Serial.println("== KTOME: Timer ==");

  // Start CAN bus
  ktomeCAN.id(CAN_ID, CAN_MASK);
  ktomeCAN.start();
  // start the CAN bus at 500 kbps
  if (!ktomeCAN.start()) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.print("My ID is:   0b");
  ktomeCAN.padZeros(ktomeCAN.can_id);
  Serial.println(ktomeCAN.can_id, BIN);
  Serial.print("My mask is: 0b");
  ktomeCAN.padZeros(ktomeCAN.can_mask);
  Serial.println(ktomeCAN.can_mask, BIN);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  display.init();
  u8g2Fonts.begin(display);
}

void loop() {
  thismillis = millis();
  CANInbox();
  if (serial_inbox) {
    draw_serial();
    serial_inbox = false;
  }
  if (game_running) {
    if (thismillis >= (tickmillis + tockdelays[strike_number]) && !tockdone) {
      DacAudio.Play(&SoundTock);
      tockdone = true;
    }
    if (thismillis >= (tickmillis + 50 + (1000 / (1 + strike_number * 0.25)))) { // Sound a tick on cue in case we have a delay in CAN heartbeat
      tickmillis = millis();
      DacAudio.Play(&SoundTick);
      tockdone = false;
      tickdone = true;
      tickprotect = tickmillis + (1000 / (1 + strike_number * 0.25)) - 100;
    }
    DacAudio.FillBuffer();
  }
}

//**********************************************************************
// FUNCTIONS: Communications
//**********************************************************************

void CANInbox() {
  if (ktomeCAN.messageWaiting()) { // Outstanding messages to handle
    ktomeCAN.receive();
    if (ktomeCAN.can_msg[0] == 'S') {
      Serial.println("Serial number received!");
      serial_number[0] = ktomeCAN.can_msg[1];
      serial_number[1] = ktomeCAN.can_msg[2];
      serial_number[2] = ktomeCAN.can_msg[3];
      serial_number[3] = ktomeCAN.can_msg[4];
      serial_number[4] = ktomeCAN.can_msg[5];
      serial_number[5] = ktomeCAN.can_msg[6];
      Serial.println(serial_number);
      serial_inbox = true;
    }
    else if (ktomeCAN.can_msg[0] == 'X') {
      // Play strike sound
      DacAudio.Play(&SoundStrike);
      strike_number = ktomeCAN.can_msg[1] - '0';
      Serial.println(strike_number, DEC);
    }
    else if (ktomeCAN.can_msg[0] == 'H') {
      // Play ticking sound
      tickmillis = millis();
      DacAudio.Play(&SoundTick);
      tockdone = false;
    }
    else if (ktomeCAN.can_msg[0] == 'Z') {
      // Stop all sounds
      game_running = false;
      tockdone = true;
      DacAudio.StopAllSounds();
      if (ktomeCAN.can_msg[1] == '1') {
        DacAudio.Play(&SoundExpl);
      }
    }
    else if (ktomeCAN.can_msg[0] == 'A') {
      game_running = true;
      strike_number = 0;
    }
  }
}
