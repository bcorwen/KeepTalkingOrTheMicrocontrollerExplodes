#include <Arduino.h>
#include <KTOME_Simon.h>

KTOME_Simon::KTOME_Simon()
{
}

void KTOME_Simon::start()
{
    for (byte ii = 0; ii < 4; ii++)
    {
        leds[ii].init(pin_array[ii], 5000, ii);
        switches[ii].init(pin_array[ii + 4], 200);
    }
    reset();
}

void KTOME_Simon::reset()
{
    for (byte ii = 0; ii < 4; ii++)
    {
        leds[ii].write(false);
    }
    stage = 0;
    strike_count = 0;
}

void KTOME_Simon::generate()
{

    stages = random(2) + 3;
    Serial.print(F("Number of stages: "));
    Serial.println(stages);

    Serial.print("Sequence: ");
    for (byte ii = 0; ii < stages; ii++)
    {
        sequence_lights[ii] = random(4);
        Serial.print(light_colours[sequence_lights[ii]]);
        if (ii < (stages - 1))
        {
            Serial.print(" > ");
        }
        else
        {
            Serial.println();
        }
    }
}

void KTOME_Simon::vowelSolution(bool serial_vowel)
{
    this->serial_vowel = serial_vowel;
    findSolution();
}

void KTOME_Simon::strikeSolution(byte strike_count)
{
    this->strike_count = strike_count;
    findSolution();
}

void KTOME_Simon::findSolution()
{
    byte vowel_row;
    if (serial_vowel)
    {
        vowel_row = 0;
    }
    else
    {
        vowel_row = 1;
    }
    Serial.print("Current Simon solution (for vowel=");
    Serial.print(serial_vowel);
    Serial.print(" & strikes=");
    Serial.print(strike_count);
    Serial.print("): ");
    for (byte ii = 0; ii < 4; ii++)
    {
        button_to_light[ii] = translation[strike_count][vowel_row][ii];
    }
    for (byte ii = 0; ii < stages; ii++)
    {
        Serial.print(light_colours[button_to_light[sequence_lights[ii]]]);
        if (ii < (stages - 1))
        {
            Serial.print(" > ");
        }
        else
        {
            Serial.println();
        }
    }
}

byte KTOME_Simon::inputCheck()
{
    byte return_result;
    for (byte ii = 0; ii < 4; ii++)
    {
        if (hasButtonBeenPushed(ii))
        {
            //   return_result = logicCheck(ii);
            return_result = (1 << (logicCheck(ii) + 4));
            return_result |= (1 << ii); // defuse bit, strike bit, no action bit, red bit, blue bit, green bit, yellow bit.
            //   if (!isDefused()) {
            //     return return_result; // return 1 on a strike
            //   } else {
            //     return 2; // return 2 for a defuse
            //   }
            if (isDefused())
            {
                return_result |= (1 << 6);
            }
            return return_result;
        }
    }
    return 0; // return 0 if no strike or defuse
}

bool KTOME_Simon::hasButtonBeenPushed(byte button_number)
{
    if (switches[button_number].hasChanged())
    {
        if (switches[button_number].isPressed())
        {
            Serial.print(button_number);
            Serial.println(" button pressed!");
            user_interrupt = true;
            lightsOut();
            // leds[button_number].blink(true, led_on_time, 1);
            leds[button_number].setPulse(255, 0, 1, 100, 300, 100, false, 1);
            return true;
        }
        else
        {
            // Serial.print(button_number);
            // Serial.println(" button released!");
            return false;
        }
    }
    else
    {
        return false;
    }
}

byte KTOME_Simon::logicCheck(byte button_number)
{

    uint32_t this_time = millis();

    Serial.print("Stage: ");
    Serial.print(stage);
    Serial.print(" | Step: ");
    Serial.println(step);

    if (button_to_light[sequence_lights[step]] == button_number)
    {                                           // Correct button pressed
        light_timing = this_time + reset_time;  // Push back time for demo to begin
        button_timing = this_time + reset_time; // Push back time for user's input to be reset
        step++;
        if (step > stage)
        {
            stage = step;
            light_timing = this_time + advance_time;
            button_timing = this_time + advance_time;
            Serial.println("Correct button, stage complete!");
        }
        else
        {
            Serial.println("Correct button, next step...");
        }
        return 0; // Correct input
    }
    else
    {
        light_timing = this_time + strike_time; // Demo to start up after shorter time
        button_timing = this_time;              // Assume end of user input now
        step = 0;
        return 1; // Trigger for a strike
    }
}

byte KTOME_Simon::getStage()
{
    return stage;
}

bool KTOME_Simon::isDefused()
{
    if (stage == stages)
    {
        return true;
    }
    else
    {
        return false;
    }
}

byte KTOME_Simon::update()
{

    byte return_result = 0;

    for (byte ii = 0; ii < 4; ii++)
    {
        leds[ii].update();
    }

    uint32_t this_time = millis();

    // Serial.print("user_interrupt: ");
    // Serial.println(user_interrupt);

    if (game_running)
    {
        if (button_timing <= this_time && user_interrupt)
        { // User has interruped demo but has now taken too long to input next button press...
            Serial.println("Switching back to demo...");
            user_interrupt = false;
            step = 0; // User needs to restart at the beginning of the sequence
            disp_step = 0;
            // light_timing = this_time;
        }

        if (light_timing <= this_time && !user_interrupt)
        { // We are due the next light in the demo and the user isn't inputting anything...
            Serial.println("Next light triggered...");
            // leds[sequence_lights[disp_step]].blink(true, led_on_time, 1);
            leds[sequence_lights[disp_step]].setPulse(255, 0, 1, 100, 300, 100, false, 1);
            return_result = (1 << sequence_lights[disp_step]);
            light_timing = this_time + led_on_time + led_off_time;
            button_timing = this_time;
            disp_step++;
            if (disp_step > stage)
            { // Sequence ended, restart
                Serial.println("which was the last in the seq! Back to first light...");
                disp_step = 0;
                light_timing = this_time + reset_time;
            }
        }
    }

    return return_result;
}

void KTOME_Simon::explode()
{
    lightsOut();
}

void KTOME_Simon::lightsOut()
{
    for (byte ii = 0; ii < 4; ii++)
    {
        leds[ii].write(false);
    }
}