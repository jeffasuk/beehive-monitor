/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/

#include <ESP8266WiFi.h>
#include "globals.h"
#include "led.h"
#include "network.h"
#include "power.h"
#include "eepromutils.h"

/* The following macro caters for all the various int types in persistent data.
    Although using a macro doesn't reduce code size, it does make modifications/bug fixes easier.
*/
#define SHOW_SIMPLE_VALUE(VAR_TYPE) \
{ \
    VAR_TYPE newval; \
    VAR_TYPE *current_value = (VAR_TYPE*)p_settable->value; \
    Serial.println(*((VAR_TYPE*)(p_settable->value))); \
}

#ifndef QUIET
void showSettings()
{
    PERSISTENT_INFO *p_settable;
    PERSISTENT_STRING_INFO *pers_str_ptr;
    // simple values
    for (p_settable = persistents; p_settable->name; ++p_settable)
    {
        Serial.print("  ");
        Serial.print(p_settable->name);
        Serial.print("=");
        switch (p_settable->type)
        {
          case PERS_INT8:
            SHOW_SIMPLE_VALUE(int8_t);
            break;
          case PERS_UINT8:
            SHOW_SIMPLE_VALUE(uint8_t);
            break;
          case PERS_INT16:
            SHOW_SIMPLE_VALUE(int16_t);
            break;
          case PERS_UINT16:
            SHOW_SIMPLE_VALUE(uint16_t);
            break;
          case PERS_INT32:
            SHOW_SIMPLE_VALUE(int32_t);
            break;
          case PERS_UINT32:
            SHOW_SIMPLE_VALUE(uint32_t);
            break;
          case PERS_FLOAT:
            SHOW_SIMPLE_VALUE(float);
            break;
          case PERS_STR:
            {
                // not sure how to do this w.r.t. writing to EEPROM
            }
            break;
        }
    }
    // string values
    for (pers_str_ptr = persistent_strings; pers_str_ptr->name; ++pers_str_ptr)
    {
        Serial.print("  ");
        Serial.print(pers_str_ptr->name);
        Serial.print("=");
        if (*(pers_str_ptr->value))
        {
            Serial.print("'");
            Serial.print(*(pers_str_ptr->value));
            Serial.print("'");
        }
        else
        {
            Serial.print("empty");
        }
        Serial.println();
    }
}
#endif

void setup()
{                
    int do_server_mode = 0;
    Serial.begin(115200);
    delay(100);
    Serial.println();
    pinMode(SETUP_PIN, INPUT_PULLUP);     
    pinMode(LED_PIN, OUTPUT);     
    delay(100);
    if (eepromIsUninitialized())
    {
        do_server_mode = 1;
        Serial.print("No valid data in EEPROM.");
    }
    else
    {
        Serial.println("Reading settings from EEPROM");
        readFromEeprom();
#ifndef QUIET
        showSettings();
#endif
    }
    if (!do_server_mode && digitalRead(SETUP_PIN) == LOW)
    {
        do_server_mode = 1;
        Serial.print("Set-up signal found.");
    }
    if (do_server_mode)
    {
        Serial.println(" Entering server mode.");
        WiFi.mode(WIFI_AP_STA); // AP for now, then STA to check WiFi config when it's been set
        startServerMode();
        in_server_mode = 1;
        return;
    }
    // sleep a little in software while hardware wakes up
    delay(2000);
    WiFi.mode(WIFI_STA);    // Don't need AP now
    connectWiFi();
}

// the loop routine runs over and over again forever:
void loop()
{
    int i;
    uint8_t write_changes;

    setLED();

    if (in_server_mode)
    {
        handleSetupRequest();
        return;
    }
    write_changes = getSettings();  // Set flag for writing persistent data, but send report first.
                                    // All changes will have been written to the active locations already.

    sendReport();

    if (write_changes)
    {
        Serial.println("writeToEeprom");
        if (new_etag)
        {
            // server did send an ETag, so store it
            if (p_etag)
            {
                free(p_etag); // get rid of any previous one
            }
            p_etag = new_etag;
            new_etag = 0;
        }
        // FIXME: else if response was not "no change", clear out old etag
        writeToEeprom();
    }

    if (persistent_data.time_between_reports > persistent_data.time_to_sleep)
    {
        int remaining = (persistent_data.time_between_reports - persistent_data.time_to_sleep) * 1000;
        setLEDflashing(500, 500);
        while (remaining > 0)
        {
            delay(20);  // Must be short enough for the LED pulser to have a chance of doing what we want.
            remaining -= 20;
            setLED();
        }
    }
    setLEDflashing(0, 0);   // off
    // optionally go to sleep for a bit
    if (persistent_data.time_to_sleep)
    {
        hw_sleep(persistent_data.time_to_sleep);
    }
}
