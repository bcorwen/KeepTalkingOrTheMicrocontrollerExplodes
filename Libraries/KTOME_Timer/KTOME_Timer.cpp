#include <Arduino.h>
#include <KTOME_Timer.h>
#include <KTOME_common.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Common aspects

KTOME_Timer::KTOME_Timer()
{
}

void KTOME_Timer::init()
{
    timerdisp = Adafruit_7segment(); // Default pins: I2C SCL = GPIO22, I2C SDA = GPIO21
    timerdisp.begin(0x70);           // Timer uses I2C pins @ 0x70 (Data = 21, Clock = 22)
    for (byte ii = 0; ii < 2; ii++)
    {
        leds[ii].init(pin_array[ii]);
    }
    reset();
}

void KTOME_Timer::reset()
{
    for (byte ii = 0; ii < 2; ii++)
    {
        leds[ii].write(false);
    }

    strike_count = 0;
    updateStrikes();

    timerdisp.print(10000);
    timerdisp.writeDisplay();
}

void KTOME_Timer::gameStart()
{ // Mark the macros at game start, from which delta_t will be measured
    this_macros = micros();
    time_left = time_length;
}

void KTOME_Timer::explode()
{
    timer_display = "----";
    drawDisplay();
    draw_colon = false;
    blink_start = millis();
    setStrikes(0);

    for (byte ii = 0; ii < 2; ii++)
    {
        leds[ii].blink(true, 100, 1);
    }
}

void KTOME_Timer::defuse()
{
    // blink_start = millis();
    timerdisp.blinkRate(1);
    blink_start = millis();

    if (getStrikes() == 2)
    {
        leds[0].write(true);
        leds[1].write(true);
    }
}

// Timer aspects

void KTOME_Timer::convertTimeToStr()
{ // Create string for the display
    if (time_left >= (60 * us_conv))
    { // Over 1 minute on the clock...
        int32_t holding_digit;
        holding_digit = (int32_t)(time_left / (600 * us_conv)) + 48;
        timer_display.setCharAt(0, holding_digit);
        holding_digit = (int32_t)((time_left % (600 * us_conv)) / (60 * us_conv)) + 48;
        timer_display.setCharAt(1, holding_digit);
        holding_digit = (int32_t)((time_left % (60 * us_conv)) / (10 * us_conv)) + 48;
        timer_display.setCharAt(2, holding_digit);
        holding_digit = (int32_t)((time_left % (10 * us_conv)) / (us_conv)) + 48;
        timer_display.setCharAt(3, holding_digit);
        draw_colon = true;
    }
    else if (time_left > 0)
    { // Under 1 minute left...
        int32_t holding_digit;
        holding_digit = (int32_t)(time_left / (10 * us_conv)) + 48;
        timer_display.setCharAt(0, holding_digit);
        holding_digit = (int32_t)((time_left % (10 * us_conv)) / (1000 * ms_conv)) + 48;
        timer_display.setCharAt(1, holding_digit);
        holding_digit = (int32_t)((time_left % (1000 * ms_conv)) / (100 * ms_conv)) + 48;
        timer_display.setCharAt(2, holding_digit);
        holding_digit = (int32_t)((time_left % (100 * ms_conv)) / (10 * ms_conv)) + 48;
        timer_display.setCharAt(3, holding_digit);
        draw_colon = true;
    }
    else
    { // Timer ran out
        timer_display = "0000";
        draw_colon = false;
        // time_string = true;
    }
}

void KTOME_Timer::blinkTime(bool game_won)
{
    if (game_won)
    {
        if (millis() >= (blink_start + 1500))
        {
            timerdisp.blinkRate(0);
        }
    }
    else
    {
        if (millis() >= (blink_start + 100))
        {
            timer_display = "    ";
            draw_colon = false;
            drawDisplay();
        }
    }
}

void KTOME_Timer::drawDisplay()
{ // Write string to display
    timerdisp.writeDigitNum(0, (int)(timer_display.charAt(0) - 48));
    timerdisp.writeDigitNum(1, (int)(timer_display.charAt(1) - 48));
    timerdisp.writeDigitNum(3, (int)(timer_display.charAt(2) - 48));
    timerdisp.writeDigitNum(4, (int)(timer_display.charAt(3) - 48));
    timerdisp.drawColon(draw_colon);
    timerdisp.writeDisplay();
}

void KTOME_Timer::setGameLength(int32_t game_length)
{
    time_length = game_length; // in microseconds
    Serial.print("Game set to: ");
    Serial.print(time_length / us_conv);
    Serial.println(" seconds");
    time_left = time_length;
    convertTimeToStr();
    drawDisplay();
}

void KTOME_Timer::updateTimer(byte gamemode, bool game_win)
{ // Run once per loop when game is running to calculate amount of time that has ticked over
    if (gamemode == 3)
    {
        int32_t delta_t = micros() - this_macros;
        this_macros += delta_t;
        time_left = time_left - (delta_t * (1 + strike_count * 0.25));

        // Serial.print("Delta t this loop: ");
        // Serial.println(delta_t);
        // Serial.print("Time left: ");
        // Serial.println(time_left / us_conv);

        convertTimeToStr();
        drawDisplay();
    }
    else if (gamemode == 4)
    {
        // Handle the blinking display on game success / blanking when
        blinkTime(game_win);
        drawDisplay();
        // if (game_win)
        // {

        // }
        // else
        // {
        //     // convertTimeToStr();
        // }
    }
    else
    {
        //drawDisplay();
    }
}

bool KTOME_Timer::hasSecondTickedOver()
{ // True if one second has ticked over, use for heartbeat CAN message
    if (time_left >= (60 * us_conv))
    {
        if (timer_display != timer_display_prev)
        {
            timer_display_prev = timer_display;
            return true;
        }
        return false;
    }
    else
    {
        if ((timer_display.charAt(0) != timer_display_prev.charAt(0)) || (timer_display.charAt(1) != timer_display_prev.charAt(1)))
        {
            timer_display_prev = timer_display;
            return true;
        }
        return false;
    }
}

int32_t KTOME_Timer::getTimeLeft()
{ // Return time left numerically
    return time_left;
}

String KTOME_Timer::getTimeStr()
{ // Return time left as a string of what is displayed on the timer, for use with the Button
    return timer_display;
}

bool KTOME_Timer::hasTimerExpired()
{ // Check if timer has reached zero
    return (time_left <= 0);
}

// Strike aspects

void KTOME_Timer::setHardcore(bool hardcore_mode)
{
    if (hardcore_mode)
    {
        strike_limit = 1;
    }
    else
    {
        strike_limit = 3;
    }
}

void KTOME_Timer::setStrikes(byte strike_count)
{
    this->strike_count = strike_count;
    assignStrikes();
}

void KTOME_Timer::addStrikes(byte strike_add)
{
    strike_count += strike_add;
    assignStrikes();
}

byte KTOME_Timer::getStrikes()
{
    return strike_count;
}

void KTOME_Timer::assignStrikes()
{
    switch (strike_count)
    {
    case 0:
        leds[0].write(false);
        leds[1].write(false);
        break;
    case 1:
        leds[0].write(true);
        leds[1].write(false);
        break;
    case 2:
        leds[0].blink(true, 250);
        leds[1].blink(true, 250);
        break;
    case 3:
        leds[0].write(true);
        leds[1].write(true);
        break;
    }
}

void KTOME_Timer::updateStrikes()
{
    for (byte ii = 0; ii < 2; ii++)
    {
        leds[ii].update();
    }
}

bool KTOME_Timer::reachedStrikeLimit()
{
    if (strike_count >= strike_limit)
    {
        return true;
    }
    return false;
}