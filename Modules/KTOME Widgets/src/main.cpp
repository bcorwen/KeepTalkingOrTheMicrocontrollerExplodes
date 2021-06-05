//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 04/05/21
//======================================================================
//
//  Module: Widgets (SLAVE)
//
//  version 0.6.0
//
//  Goal for this version: Add in Simon sound handling
//
//======================================================================

//**********************************************************************
// LIBRARIES
//**********************************************************************
#include <Arduino.h>
#include <CAN.h>
#include <KTOME_CAN.h>
#include <Adafruit_GFX.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <SoundFile.h>
#include <XT_DAC_Audio.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
#define PIN_LED GPIO_NUM_12
#define PIN_EINK_DIN GPIO_NUM_23
#define PIN_EINK_CLK GPIO_NUM_18
#define PIN_EINK_CS GPIO_NUM_5
#define PIN_EINK_DC GPIO_NUM_22
#define PIN_EINK_RST GPIO_NUM_21
#define PIN_EINK_BUSY GPIO_NUM_4

// CAN
//#define CAN_ID            CAN_WIDGETS
//#define CAN_MASK          CAN_WIDGETS
int CAN_ID;

GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT>
    display(GxEPD2_290c(PIN_EINK_CS, PIN_EINK_DC, PIN_EINK_RST, PIN_EINK_BUSY));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

uint16_t colorWhite = GxEPD_WHITE;
uint16_t colorBlack = GxEPD_BLACK;
uint16_t colorRed = GxEPD_RED;

XT_DAC_Audio_Class DacAudio(25, 0);
XT_Wav_Class SoundTick(TickWav);
XT_Wav_Class SoundTock(TockWav);
XT_Wav_Class SoundStrike(StrikeWav);
XT_Wav_Class SoundExpl(ExplWav);
XT_Wav_Class SoundWin(WinWav);
XT_Instrument_Class SimonSays;
int8_t PROGMEM ToneRed[] = {NOTE_CS5, SCORE_END};
int8_t PROGMEM ToneBlue[] = {NOTE_E5, SCORE_END};
int8_t PROGMEM ToneGreen[] = {NOTE_G5, SCORE_END};
int8_t PROGMEM ToneYellow[] = {NOTE_B5, SCORE_END};
XT_MusicScore_Class SimonRed(ToneRed, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonBlue(ToneBlue, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonGreen(ToneGreen, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonYellow(ToneYellow, TEMPO_MODERATO, &SimonSays);
bool simon_delay;
byte simon_sound;
bool simon_cued;
int32_t simon_time;

char serial_number[] = "XXXXXX";
bool serial_inbox = false;
byte strike_number;
long thismillis;
long tickmillis;
long tockdelays[3] = {667, 600, 1000};
bool tockdone = true;
bool tickdone = false;
bool tickstarted = false;
long tickprotect;
bool game_running = false;

//**********************************************************************
// E-PAPER DISPLAY
//**********************************************************************

void draw_serial() {
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(1);

    display.firstPage();
    do {
        display.fillScreen(colorWhite);
        u8g2Fonts.setForegroundColor(colorBlack);
        u8g2Fonts.setBackgroundColor(colorWhite);
        // u8g2Fonts.setFont(u8g2_font_inb53_mf);
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
// FUNCTIONS: Communications
//**********************************************************************


void soundEnding(byte bomb_exploded) {
    if (bomb_exploded == 1){
        DacAudio.StopAllSounds();
        DacAudio.Play(&SoundExpl);
    } else if (bomb_exploded == 0) {
        DacAudio.RemoveFromPlayList(&SoundTick);
        DacAudio.RemoveFromPlayList(&SoundTock);
        DacAudio.Play(&SoundWin);
    } else {
        DacAudio.StopAllSounds();
    }
}

void CANInbox() {
    if (ktomeCAN.isMessageWaiting()) { // Outstanding messages to handle
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
        } else if (ktomeCAN.can_msg[0] == 'X') {
            // Play strike sound
            DacAudio.Play(&SoundStrike);
            strike_number = ktomeCAN.can_msg[1] - '0';
            Serial.println(strike_number, DEC);
        } else if (ktomeCAN.can_msg[0] == 'H') {
            // Play ticking sound
            tickmillis = millis();
            DacAudio.Play(&SoundTick);
            tockdone = false;
            tickstarted = true;
        } else if (ktomeCAN.can_msg[0] == 'Z') {
            // Stop all sounds
            game_running = false;
            tockdone = true;
            tickstarted = false;
            byte bomb_exploded;
            if (ktomeCAN.can_msg[1] == '1'){
                bomb_exploded = 1;
            } else if (ktomeCAN.can_msg[1] == '0') {
                bomb_exploded = 0;
            } else {
                bomb_exploded = 2;
            }
            soundEnding(bomb_exploded);
            // DacAudio.StopAllSounds();
            // if (ktomeCAN.can_msg[1] == '1') {
            //     DacAudio.Play(&SoundExpl);
            // }
        } else if (ktomeCAN.can_msg[0] == 'A') {
            game_running = true;
            strike_number = 0;
        } else if (ktomeCAN.can_msg[0] == 'u') {
            simon_sound = ktomeCAN.can_msg[1] - '0';
            simon_delay = ktomeCAN.can_msg[2] - '0';
            simon_cued = true;
            if (simon_delay) {
                simon_time = millis() + 100;
            } else {
                simon_time = millis();
            }
        }
    }
}

void playSimon() {
    if (simon_cued && (millis() >= simon_time)) {
        simon_cued = false;
        switch (simon_sound) {
        case 0:
            DacAudio.RemoveFromPlayList(&SimonBlue);
            DacAudio.RemoveFromPlayList(&SimonGreen);
            DacAudio.RemoveFromPlayList(&SimonYellow);
            DacAudio.Play(&SimonRed);
            break;
        case 1:
            DacAudio.RemoveFromPlayList(&SimonRed);
            DacAudio.RemoveFromPlayList(&SimonGreen);
            DacAudio.RemoveFromPlayList(&SimonYellow);
            DacAudio.Play(&SimonBlue);
            break;
        case 2:
            DacAudio.RemoveFromPlayList(&SimonRed);
            DacAudio.RemoveFromPlayList(&SimonBlue);
            DacAudio.RemoveFromPlayList(&SimonYellow);
            DacAudio.Play(&SimonGreen);
            break;
        case 3:
            DacAudio.RemoveFromPlayList(&SimonRed);
            DacAudio.RemoveFromPlayList(&SimonBlue);
            DacAudio.RemoveFromPlayList(&SimonGreen);
            DacAudio.Play(&SimonYellow);
            break;
        }
    }
}

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************

void setup() {
    // Start serial connection
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("== KTOME: Widget ==");

    // Start CAN bus
    CAN_ID = can_ids.Widgets;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start();
    // start the CAN bus at 500 kbps
    if (!ktomeCAN.start()) {
        Serial.println("Starting CAN failed!");
        while (1)
        ;
    }
    Serial.print("My ID is:   0b");
    ktomeCAN.padZeros(CAN_ID);
    Serial.println(CAN_ID, BIN);
    //  Serial.print("My mask is: 0b");
    //  ktomeCAN.padZeros(ktomeCAN.can_mask);
    //  Serial.println(ktomeCAN.can_mask, BIN);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    display.init();
    u8g2Fonts.begin(display);

    SimonSays.SetInstrument(3);
    SimonSays.SetWaveForm(WAVE_SINE);
}

void loop() {
    thismillis = millis();
    CANInbox();
    if (serial_inbox) {
        draw_serial();
        serial_inbox = false;
    }
    if (game_running) {
        if (tickstarted) {
            if (thismillis >= (tickmillis + tockdelays[strike_number]) && !tockdone) {
                DacAudio.Play(&SoundTock);
                tockdone = true;
            }
            if (thismillis >=
                (tickmillis + 50 +
                (1000 / (1 + strike_number * 0.25)))) { // Sound a tick on cue in
                                                        // case we have a delay in
                                                        // CAN heartbeat
                tickmillis = millis();
                DacAudio.Play(&SoundTick);
                tockdone = false;
                tickdone = true;
                tickprotect = tickmillis + (1000 / (1 + strike_number * 0.25)) - 100;
            }
        }
    }
    playSimon();
    DacAudio.FillBuffer();
}