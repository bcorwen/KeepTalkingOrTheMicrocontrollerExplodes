//======================================================================
//
//  Keep Talking Or the Microcontroller Explodes!
//
//    - bcorwen, 22/03/23
//======================================================================
//
//  Module: Widgets (SLAVE)
//
//  version 0.9.0
//
//  Goal for this version: Support for multiple audio-producing
//                          modules of same type
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

#define DEBUG 1
// #define TEST_SOUND 1
#define EINK_DISPLAY 1

#ifdef DEBUG
    #define DEBUG_SERIAL(x)     Serial.begin(x)
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTBIN(x)   Serial.print(x, BIN)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTLNBIN(x) Serial.println(x, BIN)
    #define DEBUG_PADZERO(x)    ktomeCAN.padZeros(x)
#else
    #define DEBUG_SERIAL(x)
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTBIN(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTLNBIN(x)
    #define DEBUG_PADZERO(x)
#endif

#ifdef EINK_DISPLAY
    #define EINK_DRAW(x)  draw_serial(x)
    #define EINK_INIT(x)  display.init(x);
    #define EINK_FONTS(x) u8g2Fonts.begin(x);
#else
    #define EINK_DRAW(x)
    #define EINK_INIT(x)
    #define EINK_FONTS(x)
#endif

// #include <WiFi.h>
// #include <BluetoothSerial.h>

//**********************************************************************
// GLOBAL VARIABLES
//**********************************************************************
// #define PIN_LED GPIO_NUM_12
#define PIN_EINK_DIN GPIO_NUM_23
#define PIN_EINK_CLK GPIO_NUM_18
#define PIN_EINK_CS GPIO_NUM_16     //GPIO_NUM_5
#define PIN_EINK_DC GPIO_NUM_17     //GPIO_NUM_22
#define PIN_EINK_RST GPIO_NUM_4     //GPIO_NUM_21
#define PIN_EINK_BUSY GPIO_NUM_5    //GPIO_NUM_4

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
XT_Wav_Class SoundAct(NeedyAct);
XT_Wav_Class SoundWarn(NeedyWarn);
XT_Wav_Class SoundCapPop(CapPopWav);

XT_Instrument_Class SimonSays;
int8_t PROGMEM ToneRed[] = {NOTE_CS5, SCORE_END};
int8_t PROGMEM ToneBlue[] = {NOTE_E5, SCORE_END};
int8_t PROGMEM ToneGreen[] = {NOTE_G5, SCORE_END};
int8_t PROGMEM ToneYellow[] = {NOTE_B5, SCORE_END};
XT_MusicScore_Class SimonRed(ToneRed, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonBlue(ToneBlue, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonGreen(ToneGreen, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonYellow(ToneYellow, TEMPO_MODERATO, &SimonSays);
struct SimonModule {
    XT_MusicScore_Class Red;
    XT_MusicScore_Class Blue;
    XT_MusicScore_Class Green;
    XT_MusicScore_Class Yellow;
    uint32_t id;
    bool delay;
    bool cued;
    byte sound;
    int32_t time;
};
// Support for up to 4 Simon modules connected - only plan to produce 3 so this is enough
SimonModule SimonMods[4] = {{SimonRed, SimonBlue, SimonGreen, SimonYellow, 0, false, false, 0, 0},
                            {SimonRed, SimonBlue, SimonGreen, SimonYellow, 0, false, false, 0, 0},
                            {SimonRed, SimonBlue, SimonGreen, SimonYellow, 0, false, false, 0, 0},
                            {SimonRed, SimonBlue, SimonGreen, SimonYellow, 0, false, false, 0, 0}};

// bool simon_delay;
// byte simon_sound;
// bool simon_cued;
// int32_t simon_time;

// Capacitor discharge:
// Tone sounds with 25 seconds left
// When discharging, will increase timer x5 normal speed
// Tone starts 300 Hz and ends 450 Hz
XT_Instrument_Class DisInstrument;

struct CapDisModule {
    uint32_t id;
    bool tone_playing;
    bool cued;
    byte sound;
    bool repeat;
    int32_t time;
    int32_t distimeleft;
    int8_t disdirection;
    int32_t disvol;
    int32_t disfreq;

};
// Support for up to 3 CapDis modules connected - only plan to produce 2 so this is enough
CapDisModule CapDisMods[3] = {{0, false, false, 0, false, 0},
                              {0, false, false, 0, false, 0},
                              {0, false, false, 0, false, 0}};

// bool cap_tone_playing = false;
bool needy_cued;
byte needy_sound;
bool needy_repeat;
int32_t needy_time;

// int32_t distimeleft = 45000;
// int8_t disdirection = -1; // -1 is decreasing time (increasing tone), 1 is increasing time (decreasing tone)
// int32_t disvol = 0; // 0 @25s left, 127 @0s left
// int32_t disfreq = 300;
int32_t disvolstepsec = 1000*127/25; // 5k steps per second, 5 steps per ms,
int32_t disfreqstepsec = 1000*150/25; // 6k steps per second, 6 steps per ms
int8_t disvollut[] = {0, 1, 1, 1, 1, 1, 1, 1,
                      1, 1, 1, 2, 2, 2, 2, 2,
                      2, 2, 2, 2, 2, 2, 2, 2,
                      2, 2, 2, 3, 3, 3, 3, 3,
                      3, 3, 3, 3, 3, 4, 4, 4,
                      4, 4, 4, 4, 5, 5, 5, 5,
                      5, 6, 6, 6, 6, 6, 7, 7,
                      7, 7, 8, 8, 8, 9, 9, 9,
                      10, 10, 11, 11, 12, 12, 12, 13,
                      13, 14, 15, 15, 16, 16, 17, 18,
                      19, 19, 20, 21, 22, 23, 24, 25,
                      26, 27, 28, 29, 30, 32, 33, 34,
                      36, 37, 39, 40, 42, 44, 46, 47,
                      49, 52, 54, 56, 58, 61, 63, 66,
                      69, 72, 75, 78, 81, 84, 88, 92,
                      96, 100, 104, 108, 113, 117, 122, 127};

int8_t sound_test_counter;

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

int32_t test_timer;
int32_t time_step;
int32_t lastmillis;

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
    // delay(1000);
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
            DEBUG_PRINTLN("Serial number received!");
            serial_number[0] = ktomeCAN.can_msg[1];
            serial_number[1] = ktomeCAN.can_msg[2];
            serial_number[2] = ktomeCAN.can_msg[3];
            serial_number[3] = ktomeCAN.can_msg[4];
            serial_number[4] = ktomeCAN.can_msg[5];
            serial_number[5] = ktomeCAN.can_msg[6];
            DEBUG_PRINTLN(serial_number);
            serial_inbox = true;
        } else if (ktomeCAN.can_msg[0] == 'X') {
            // Play strike sound
            DacAudio.Play(&SoundStrike);
            strike_number = ktomeCAN.can_msg[1] - '0';
            DEBUG_PRINTLN(strike_number);
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
            for (byte ii=0; ii<3; ii++){
                CapDisMods[ii].tone_playing = false;
            }
            // cap_tone_playing = false;
            byte bomb_exploded;
            if (ktomeCAN.can_msg[1] == '1'){
                bomb_exploded = 1;
            } else if (ktomeCAN.can_msg[1] == '0') {
                bomb_exploded = 0;
            } else {
                bomb_exploded = 2;
            }
            soundEnding(bomb_exploded);
            // SIMON & CAPDIS - unregister modules
            for (byte ii=0; ii<4; ii++){
                SimonMods[ii].id = 0; // Un-log each module from this slot
                SimonMods[ii].cued = false;
                SimonMods[ii].delay = false;
                SimonMods[ii].time = 0;
                SimonMods[ii].sound = 0;
            }
            for (byte ii=0; ii<3; ii++){
                CapDisMods[ii].id = 0; // Un-log each module from this slot
                CapDisMods[ii].cued = false;
                CapDisMods[ii].tone_playing = false;
                CapDisMods[ii].time = 0;
                CapDisMods[ii].sound = 0;
                CapDisMods[ii].repeat = false;
                CapDisMods[ii].disdirection = -1;
                CapDisMods[ii].disfreq = 300;
                CapDisMods[ii].distimeleft = 45000;
                CapDisMods[ii].disvol = 0;
            }
        } else if (ktomeCAN.can_msg[0] == 'A') {
            game_running = true;
            strike_number = 0;
        } else if (ktomeCAN.can_msg[0] == 'u') {
            if (ktomeCAN.can_msg[1] == '+') {
                // CAPDIS - handle sounds
                for (byte ii=0; ii<3; ii++){
                    if (CapDisMods[ii].id == ktomeCAN.can_msg_id){ // Check if this CapDis module slot is filled with the calling module
                        String temp = ktomeCAN.can_msg;
                        temp = temp.substring(2);
                        int32_t needy_time = temp.toDouble();
                        CapDisMods[ii].distimeleft = needy_time;
                        CapDisMods[ii].disdirection = +5;
                        break;
                    }
                }
                // String temp = ktomeCAN.can_msg;
                // temp = temp.substring(2);
                // int32_t needy_time = temp.toDouble();
                // distimeleft = needy_time;
                // disdirection = +5;
            } else if (ktomeCAN.can_msg[1] == '-') {
                // CAPDIS - handle sounds
                for (byte ii=0; ii<3; ii++){
                    if (CapDisMods[ii].id == ktomeCAN.can_msg_id){ // Check if this CapDis module slot is filled with the calling module
                        String temp = ktomeCAN.can_msg;
                        temp = temp.substring(2);
                        int32_t needy_time = temp.toDouble();
                        CapDisMods[ii].distimeleft = needy_time;
                        CapDisMods[ii].disdirection = -1;
                        if (!CapDisMods[ii].tone_playing){
                            CapDisMods[ii].tone_playing = true;
                            DacAudio.Play(&DisInstrument);
                        }
                        break;
                    }
                }
                // String temp = ktomeCAN.can_msg;
                // temp = temp.substring(2);
                // int32_t needy_time = temp.toDouble();
                // distimeleft = needy_time;
                // disdirection = -1;
                // if (!cap_tone_playing){
                //     cap_tone_playing = true;
                //     DacAudio.Play(&DisInstrument);
                    
                // }
            } else if (ktomeCAN.can_msg[1] < '5') {
                // SIMON - handle sounds
                for (byte ii=0; ii<4; ii++){
                    if (SimonMods[ii].id == ktomeCAN.can_msg_id){ // Check if this Simon module slot is filled with the calling module
                        SimonMods[ii].cued = true;
                        SimonMods[ii].delay = ktomeCAN.can_msg[2] - '0';
                        SimonMods[ii].sound = ktomeCAN.can_msg[1] - '0';
                        if (SimonMods[ii].delay) {
                            SimonMods[ii].time = millis() + 100;
                        } else {
                            SimonMods[ii].time = millis();
                        }
                        break;
                    }
                }
                // simon_sound = ktomeCAN.can_msg[1] - '0';
                // simon_delay = ktomeCAN.can_msg[2] - '0';
                // simon_cued = true;
                // if (simon_delay) {
                //     simon_time = millis() + 100;
                // } else {
                //     simon_time = millis();
                // }
            } else if (ktomeCAN.can_msg[1] < '7') {
                needy_sound = ktomeCAN.can_msg[1] - '5';
                // Serial.println(needy_sound);
                needy_cued = true;
                needy_time = millis();
                if (needy_sound == 1) {
                    needy_repeat = true;
                } else {
                    needy_repeat = false;
                }
            }
        } else if (ktomeCAN.can_msg[0] == 'x' && (ktomeCAN.can_msg_id & can_ids.CapDis)>0){ // Capacitor discharge has struck
            DacAudio.RemoveFromPlayList(&DisInstrument);
            DacAudio.Play(&SoundCapPop);
        } else if (ktomeCAN.can_msg[0] == 'i') { // module is registered
            if ((ktomeCAN.can_msg_id & can_ids.CapDis)>0){ // Capacitor discharge is registered
                // CAPDIS - register
                for (byte ii=0; ii<3; ii++){
                    if (CapDisMods[ii].id == 0){ // Check if this Simon module slot is filled with a registered module
                        CapDisMods[ii].id = ktomeCAN.can_msg_id; // Log this registering module to this slot
                        CapDisMods[ii].cued = false;
                        CapDisMods[ii].repeat = false;
                        CapDisMods[ii].sound = 0;
                        CapDisMods[ii].time = 0;
                        CapDisMods[ii].tone_playing = false;
                        CapDisMods[ii].disdirection = -1;
                        CapDisMods[ii].disfreq = 300;
                        CapDisMods[ii].distimeleft = 45000;
                        CapDisMods[ii].disvol = 0;
                        break;
                    }
                }
            } else if ((ktomeCAN.can_msg_id & can_ids.Simon)>0){ // Simon is registered
                // SIMON - register
                for (byte ii=0; ii<4; ii++){
                    if (SimonMods[ii].id == 0){ // Check if this Simon module slot is filled with a registered module
                        SimonMods[ii].id = ktomeCAN.can_msg_id; // Log this registering module to this slot
                        SimonMods[ii].cued = false;
                        SimonMods[ii].delay = false;
                        SimonMods[ii].time = 0;
                        SimonMods[ii].sound = 0;
                        break;
                    }
                }
            }
        }
    }
}

void playSimon() {
    for (byte ii=0; ii<4; ii++){
        if (SimonMods[ii].cued && (millis() >= SimonMods[ii].time)) {
            SimonMods[ii].cued = false;
            DacAudio.RemoveFromPlayList(&SimonMods[ii].Red);
            DacAudio.RemoveFromPlayList(&SimonMods[ii].Blue);
            DacAudio.RemoveFromPlayList(&SimonMods[ii].Green);
            DacAudio.RemoveFromPlayList(&SimonMods[ii].Yellow);
            switch (SimonMods[ii].sound) {
                case 0:
                    DacAudio.Play(&SimonMods[ii].Red);
                    break;
                case 1:
                    DacAudio.Play(&SimonMods[ii].Blue);
                    break;
                case 2:
                    DacAudio.Play(&SimonMods[ii].Green);
                    break;
                case 3:
                    DacAudio.Play(&SimonMods[ii].Yellow);
                    break;
            }
        }
    }
    // if (simon_cued && (millis() >= simon_time)) {
    //     simon_cued = false;
    //     switch (simon_sound) {
    //         case 0:
    //             DacAudio.RemoveFromPlayList(&SimonBlue);
    //             DacAudio.RemoveFromPlayList(&SimonGreen);
    //             DacAudio.RemoveFromPlayList(&SimonYellow);
    //             DacAudio.Play(&SimonRed);
    //             break;
    //         case 1:
    //             DacAudio.RemoveFromPlayList(&SimonRed);
    //             DacAudio.RemoveFromPlayList(&SimonGreen);
    //             DacAudio.RemoveFromPlayList(&SimonYellow);
    //             DacAudio.Play(&SimonBlue);
    //             break;
    //         case 2:
    //             DacAudio.RemoveFromPlayList(&SimonRed);
    //             DacAudio.RemoveFromPlayList(&SimonBlue);
    //             DacAudio.RemoveFromPlayList(&SimonYellow);
    //             DacAudio.Play(&SimonGreen);
    //             break;
    //         case 3:
    //             DacAudio.RemoveFromPlayList(&SimonRed);
    //             DacAudio.RemoveFromPlayList(&SimonBlue);
    //             DacAudio.RemoveFromPlayList(&SimonGreen);
    //             DacAudio.Play(&SimonYellow);
    //             break;
    //     }
    // }
}

void playNeedy() {
    if (needy_cued && millis() >= needy_time) {
        needy_cued = false;
        switch (needy_sound) {
            case 0:
                DacAudio.Play(&SoundAct);
                break;
            case 1:
                DacAudio.Play(&SoundWarn);
                break;
        }
        if (needy_repeat) {
            // Serial.println("Repeat this in 1 sec...");
            needy_time += 1500;
            needy_repeat = false;
            needy_cued = true;
        }
    }
}

void playCapDis() {
    for (byte ii=0; ii<3; ii++){
        if (CapDisMods[ii].tone_playing){
            CapDisMods[ii].distimeleft += CapDisMods[ii].disdirection * time_step;

            CapDisMods[ii].disvol = 127 - (CapDisMods[ii].distimeleft/1000.0 * disvolstepsec/1000.0);
            if (CapDisMods[ii].disvol < 0){
                CapDisMods[ii].disvol = 0;
            }
            CapDisMods[ii].disfreq = 450 - (CapDisMods[ii].distimeleft/1000.0 * disfreqstepsec/1000.0);
            DisInstrument.SetFrequency(CapDisMods[ii].disfreq);
            DisInstrument.Volume = disvollut[CapDisMods[ii].disvol];
        }
    }
    // if (cap_tone_playing){
    //     distimeleft += disdirection * time_step;

    //     disvol = 127 - (distimeleft/1000.0 * disvolstepsec/1000.0);
    //     if (disvol < 0){
    //         disvol = 0;
    //     }
    //     disfreq = 450 - (distimeleft/1000.0 * disfreqstepsec/1000.0);
    //     DisInstrument.SetFrequency(disfreq);
    //     DisInstrument.Volume = disvollut[disvol];
    // }
}

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************

void setup() {
    // Start serial connection
    DEBUG_SERIAL(115200);
    DEBUG_PRINTLN("== KTOME: Widget ==");
    // Serial.begin(115200);
    // while (!Serial);
    // Serial.println("== KTOME: Widget ==");

    // Start CAN bus
    CAN_ID = can_ids.Widgets;
    ktomeCAN.setId(CAN_ID);
    ktomeCAN.start();
    DEBUG_PRINT("My ID is:   0b");
    DEBUG_PADZERO(CAN_ID);
    DEBUG_PRINTLNBIN(CAN_ID);

    // display.init();
    // u8g2Fonts.begin(display);
    EINK_INIT();
    EINK_FONTS(display);

    // Temp init of eink display
    serial_number[0] = 'K';
    serial_number[1] = 'T';
    serial_number[2] = '0';
    serial_number[3] = 'M';
    serial_number[4] = 'E';
    serial_number[5] = '1';
    DEBUG_PRINTLN("Serial set!");
    EINK_DRAW();
    DEBUG_PRINTLN("Serial drawn!");

    // SimonSays.SetInstrument(3);
    // SimonSays.SetWaveForm(WAVE_SINE);

    // New to stop pop
    DEBUG_PRINTLN("Set Simon instrument...");
    // SimonSays.Init();
    SimonSays.SetInstrument(0);
    SimonSays.SetWaveForm(WAVE_SINE);
    XT_Envelope_Class *SimonEnv = SimonSays.AddEnvelope();
    int16_t note_dur = 480; // Duration for envelope parts in ms, so aiming for 500ms total
    int16_t note_pad = 18;
    SimonEnv->AddPart(note_dur-note_pad,127,127);
    SimonEnv->AddPart(note_pad,0);
    SimonEnv->AddPart(3000,0);
    
    SoundWarn.Repeat=2;

    DEBUG_PRINTLN("Set CapDis instrument...");
    DisInstrument.SetInstrument(0);
    DisInstrument.SetWaveForm(WAVE_SINE);
    DisInstrument.RepeatForever = true;
    DisInstrument.SetFrequency(300);
    DisInstrument.SetDuration(100000);
    // DacAudio.Play(&DisInstrument);

    // WiFi.mode(WIFI_MODE_NULL);
    // btStop();

    DEBUG_PRINTLN("Setup done!");
}

void loop() {
    lastmillis = thismillis; // Only for test - cap dis
    thismillis = millis();
    time_step = thismillis - lastmillis; // Only for test - cap dis

    #ifndef TEST_SOUND // SOUND TEST OFF

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
                (1000 / (1 + strike_number * 0.25)))) { // Sound a tick on cue in case we have a delay in CAN heartbeat
                tickmillis = millis();
                DacAudio.Play(&SoundTick);
                tockdone = false;
                tickdone = true;
                tickprotect = tickmillis + (1000 / (1 + strike_number * 0.25)) - 100;
            }
        }
    }
    playSimon();
    playNeedy();
    playCapDis();
    DacAudio.FillBuffer();

    #else // SOUND TEST ON

    distimeleft += disdirection * time_step;

    if (distimeleft <= 0){
        disdirection = 5;
    } else if (distimeleft >= 45000){
        disdirection = -1;
    }
    disvol = 127 - (distimeleft/1000.0 * disvolstepsec/1000.0);
    if (disvol < 0){
        disvol = 0;
    }
    disfreq = 450 - (distimeleft/1000.0 * disfreqstepsec/1000.0);
    DisInstrument.SetFrequency(disfreq);
    DisInstrument.Volume = disvollut[disvol];

    // if (thismillis >= test_timer) {
    //     // test_timer = thismillis + random(500)+ 2000;
    //     test_timer = thismillis + 20000;

    //     disdirection = -disdirection;
    //     if (disdirection==1){
    //         DEBUG_PRINTLN("Increasing...");
    //     } else {
    //         DEBUG_PRINTLN("Decreasing...");
    //     }
        
        // simon_sound = random(4);
        // simon_cued = true;
        // simon_time = millis();
        // playSimon();
        
        // DEBUG_PRINTLN("Playing tone...");
        // switch (sound_test_counter) {
        //     case 0:
        //         DacAudio.Play(&SoundStrike);
        //         break;
        //     case 1:
        //         DacAudio.Play(&SoundExpl);
        //         break;
        //     case 2:
        //         DacAudio.Play(&SoundWin);
        //         break;
        //     case 3:
        //         DacAudio.Play(&SoundAct);
        //         break;
        //     case 4:
        //         DacAudio.Play(&SoundWarn);
        //         break;
        //     case 5:
        //         simon_sound = random(4);
        //         simon_cued = true;
        //         simon_time = millis();
        //         playSimon();
        //         break;
        // // }
        // sound_test_counter++;
        // if (sound_test_counter >= 6){ sound_test_counter = 0; }
    // }

    #endif

    DacAudio.FillBuffer(); 
    // delay(1);
}