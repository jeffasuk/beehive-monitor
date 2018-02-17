/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <Arduino.h>
#include <EEPROM.h>
#include "globals.h"

char magic_tag[] = "BHM";    // To indicate that we've written to EEPROM, so it's OK to use the values.

uint8_t eepromIsUninitialized()
{
    char in;
    int i;
    int ret = 0;
    EEPROM.begin(512);
#ifndef QUIET
    Serial.println("Checking magic tag");
#endif
    for (i=0; i < sizeof magic_tag; ++i)
    {
        in = EEPROM.read(i);
        if (in != magic_tag[i])
        {
#ifndef QUIET
            Serial.print("Found difference on byte ");
            Serial.println(i);
#endif
            ret = i+1;
            break;
        }
    }
    EEPROM.end();
    return ret;
}

static void writeMagicTag()
{
    char in;
    int i;
    EEPROM.begin(512);
#ifndef QUIET
    Serial.println("Writing magic tag");
#endif
    for (i=0; i < sizeof magic_tag; ++i)
    {
        EEPROM.write(i, magic_tag[i]);
    }
    EEPROM.end();
    delay(100);
}

static void readWriteEeprom(int dowrite)
{
    uint8_t *p;
    int c, l;
    PERSISTENT_STRING_INFO *pers_str_ptr;

    l = sizeof persistent_data;
    EEPROM.begin(512);
    for (p = (uint8_t*)&persistent_data, l = sizeof persistent_data, c=sizeof magic_tag; l; c++, l--)
    {
        if (dowrite)
        {
#ifndef QUIET
            char buf[10];
            Serial.print("EEP write ");
            Serial.print(c);
            Serial.print(" ");
            sprintf(buf, " %x", *p & 0xff);
            Serial.print(buf);
            Serial.println();
#endif
            EEPROM.write(c, *p++);
        }
        else
        {
            *p++ = EEPROM.read(c);
        }
    }
    // That's the easy bit; the struct. Now for the variable-length strings.
    // In the EEPROM, use Pascal-style (length,value) strings for ease of memory allocation.
    for (pers_str_ptr = persistent_strings; pers_str_ptr->name; ++pers_str_ptr)
    {
        uint16_t string_len;
        char    *p;
        if (dowrite)
        {
            if (*(pers_str_ptr->value))
            {
                string_len = strlen(*(pers_str_ptr->value));
                EEPROM.write(c++, string_len & 0xff);
                EEPROM.write(c++, (string_len >> 8) & 0xff);
                for (p = *(pers_str_ptr->value); *p; ++p)
                {
                    EEPROM.write(c++, *p);
                }
            }
            else
            {
                // 16 bits of zero for item with no value
                EEPROM.write(c++, 0);
                EEPROM.write(c++, 0);
            }
        }
        else
        {
            string_len = (EEPROM.read(c++) & 0xff) + ((EEPROM.read(c++) << 8) & 0xff00);
            if (string_len > 200)
            {
                Serial.print("String too long: ");
                Serial.print(pers_str_ptr->name);
                Serial.print(", ");
                Serial.println(string_len);
                break;
            }
            if (*(pers_str_ptr->value))
            {
                free(*(pers_str_ptr->value));
            }
            if (string_len)
            {
                p = *(pers_str_ptr->value) = (char*)malloc(string_len+1);
                while (string_len--)
                {
                    *p++ = EEPROM.read(c++);
                }
                *p = '\0';
            }
            else
            {
                *(pers_str_ptr->value) = 0;
            }
        }
    }
    EEPROM.end();
    if (dowrite)
    {
        delay(100);
    }
}
void readFromEeprom()
{
    readWriteEeprom(0);
}
void writeToEeprom()
{
    writeMagicTag();
    readWriteEeprom(1);
}
