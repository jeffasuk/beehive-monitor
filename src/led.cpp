/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include "globals.h"
#include "led.h"

static enum {LED_OFF, LED_FLASH, LED_PULSE} mode = LED_OFF;
static uint8_t led_state;   // on/off

// values for "flash"
static int time_of_last_led_change;
static int led_period_on = 0;
static int led_period_off = 0;

// values for "pulse"
static int led_nb_pulses;
static int led_pulse_length;
static int led_gap_length;
static int led_period_between;
static int led_pulse_index;
static int start_time_of_this_cycle;
static int led_pulsing_period;

static void checkSwitchForFlash(int period, int new_state)
{
    int time_from_last_change = millis() - time_of_last_led_change;
    if (time_from_last_change >= period)
    {
        // time to switch
        digitalWrite(LED_PIN, led_state = new_state);
        time_of_last_led_change = millis();
    }
}

static void checkSwitchForPulse()
{
    int time_from_last_cycle_start = millis() - start_time_of_this_cycle;
    if (time_from_last_cycle_start >= (led_pulsing_period + led_period_between))
    {
        // off the end of a full cycle. Go back to the start.
        digitalWrite(LED_PIN, led_state = 0);
        start_time_of_this_cycle = millis();
        return;
    }
    if (time_from_last_cycle_start >= led_pulsing_period)
    {
        // in period between pulse series
        digitalWrite(LED_PIN, led_state = 1);
        return;
    }
    // in pulsing period
    digitalWrite(LED_PIN, led_state = ( (time_from_last_cycle_start % (led_pulse_length+led_gap_length)) < led_pulse_length) ? 0 : 1);
}

void setLEDflashing(int period_on, int period_off)
{
    mode = LED_FLASH;
    led_period_on = period_on;
    led_period_off = period_off;
    if (!led_period_on)
    {
        // no on period, so turn it off
        digitalWrite(LED_PIN, led_state = 1);
        return;
    }
    digitalWrite(LED_PIN, led_state = 0);   // start with it on
    time_of_last_led_change = millis();
}

void setLEDpulse(int nb_pulses, int pulse_length, int gap_length, int period_between)
{
    mode = LED_PULSE;
    led_nb_pulses = nb_pulses;
    led_pulse_length = pulse_length;
    led_gap_length = gap_length;
    led_period_between = period_between;
    led_pulse_index = 0;
    led_pulsing_period = (led_nb_pulses * led_pulse_length) + ((led_nb_pulses - 1) * led_gap_length);
    start_time_of_this_cycle = millis();
}

// set LED appropriately according to time and current settings
void setLED()
{
    if (mode == LED_FLASH)
    {
        if (!led_period_on)
        {
            // no on period, so turn it off
            digitalWrite(LED_PIN, led_state = 1);
            return;
        }
        checkSwitchForFlash(led_state ? led_period_off : led_period_on, 1 - led_state);
    }
    else if (mode == LED_PULSE)
    {
        if (!led_nb_pulses)
        {
            // no on period, so turn it off
            digitalWrite(LED_PIN, led_state = 1);
            return;
        }
        checkSwitchForPulse();
    }
    else
    {
        // no effective mode, so turn it off
        digitalWrite(LED_PIN, led_state = 1);
    }
}
