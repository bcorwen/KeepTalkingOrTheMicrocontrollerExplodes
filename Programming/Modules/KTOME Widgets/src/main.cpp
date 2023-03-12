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

#define DEBUG 1
#define TEST_SOUND 1
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

XT_Instrument_Class SimonSays;
int8_t PROGMEM ToneRed[] = {NOTE_CS5, SCORE_END};
int8_t PROGMEM ToneBlue[] = {NOTE_E5, SCORE_END};
int8_t PROGMEM ToneGreen[] = {NOTE_G5, SCORE_END};
int8_t PROGMEM ToneYellow[] = {NOTE_B5, SCORE_END};
XT_MusicScore_Class SimonRed(ToneRed, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonBlue(ToneBlue, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonGreen(ToneGreen, TEMPO_MODERATO, &SimonSays);
XT_MusicScore_Class SimonYellow(ToneYellow, TEMPO_MODERATO, &SimonSays);

XT_Instrument_Class DisInstrument;
// int8_t DisTone[] = {NOTE_CS5, SCORE_END};
// XT_MusicScore_Class DisScore(DisTone, TEMPO_PRESTISSIMO, &DisInstrument);
// XT_Sequence_Class DisSequence;

bool simon_delay;
byte simon_sound;
bool simon_cued;
int32_t simon_time;
bool needy_cued;
byte needy_sound;
bool needy_repeat;
int32_t needy_time;

int8_t sound_test_counter;

XT_MusicScore_Class *test_array[4] = {&SimonRed, &SimonBlue, &SimonGreen, &SimonYellow};

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
            if (ktomeCAN.can_msg[1] < '5') {
                simon_sound = ktomeCAN.can_msg[1] - '0';
                simon_delay = ktomeCAN.can_msg[2] - '0';
                simon_cued = true;
                if (simon_delay) {
                    simon_time = millis() + 100;
                } else {
                    simon_time = millis();
                }
            } else {
                needy_sound = ktomeCAN.can_msg[1] - '5';
                Serial.println(needy_sound);
                needy_cued = true;
                needy_time = millis();
                if (needy_sound == 1) {
                    needy_repeat = true;
                } else {
                    needy_repeat = false;
                }
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

//**********************************************************************
// FUNCTIONS: Main
//**********************************************************************

int32_t test_timer;
int8_t disdirection = 1;
int16_t disvol = 40;
int16_t disfreq = FNOTE_C3;
int16_t disvolstep = 1;
int16_t disfreqstepdiv = 10;
int32_t time_step;
int32_t lastmillis;

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
    // DisSequence.AddPlayItem(&DisScore); // This should repeat this note
    DisInstrument.RepeatForever = true;
    DisInstrument.SetFrequency(disfreq);
    DisInstrument.SetDuration(50);
    DisInstrument.Volume = disvol;

    // WiFi.mode(WIFI_MODE_NULL);
    // btStop();

    DEBUG_PRINTLN("Setup done!");
}

void loop() {
    lastmillis = thismillis; // Only for test - cap dis
    time_step = thismillis - lastmillis; // Only for test - cap dis
    thismillis = millis();

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
    DacAudio.FillBuffer();

    #else // SOUND TEST ON

        // Cap dis code
        disvol += (disvolstep*disdirection*time_step/1000.0);
        disfreq += ((disfreq / disfreqstepdiv)*disdirection*time_step/1000.0);
        DisInstrument.SetFrequency(disfreq);
        DisInstrument.Volume = disvol;
        DEBUG_PRINT("Setting dis vars - vol: ");
        DEBUG_PRINT(disvol);
        DEBUG_PRINT(", freq: ");
        DEBUG_PRINTLN(disfreq);

    if (thismillis >= test_timer) {
        test_timer = thismillis + random(500)+ 2000;

        disdirection = -disdirection;
        if (disdirection==1){
            DEBUG_PRINTLN("Increasing...");
        } else {
            DEBUG_PRINTLN("Decreasing...");
        }
        
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
    }

    #endif

    DacAudio.FillBuffer(); 
    // delay(1);
}