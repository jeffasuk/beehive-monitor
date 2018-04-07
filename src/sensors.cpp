/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <Arduino.h>
#include "DHT.h"
#include "OneWire.h"
#include "globals.h"
#include "utils.h"
#include "sensors.h"


static void showaddr(unsigned char  addr[8])
{
    char buf[17];
    Serial.print(formatAddr(buf, addr));
}

// WARNING. Uses a single static buffer, so not thread safe.
// The assumption is that the returned string will be immediately copied into a page being generated.
static char *getFriendlyOneWireName(unsigned char addr[8])
{
    static int buflen = 0;
    static char *single_buffer_for_friendly_names = 0;
    int required_length;
    char addr_buf[18];  // 16 hex chars plus a '=' and a NUL
    char *p;
    formatAddr(addr_buf, addr);
    addr_buf[16] = '=';
    addr_buf[17] = '\0';
    // see if we have a friendly name for this addr
    if (p_onewireids && (p = strstr(p_onewireids, addr_buf)) > 0)
    {
        char *p2;
        int length_of_friendly_name;
        addr_buf[16] = '\0';    // truncate at the '='
        p += 17;
        if ( (p2 = strchr(p, '|')) > 0)
        {
            length_of_friendly_name = p2 - p;
        }
        else
        {
            // it was the last one, so no separating '|'
            length_of_friendly_name = strlen(p);
        }
        required_length = length_of_friendly_name + 26;
        if (buflen < required_length)
        {
            single_buffer_for_friendly_names = (char*)realloc(single_buffer_for_friendly_names, required_length);
            buflen = required_length;
        }
        sprintf(single_buffer_for_friendly_names, "%s ", addr_buf);
        strncat(single_buffer_for_friendly_names, p, length_of_friendly_name);
    }
    else
    {
        // no friendly names at all, or not one for this addr. Just use the addr itself.
        addr_buf[8] = '\0';
        required_length = 17;
        if (buflen < required_length)
        {
            single_buffer_for_friendly_names = (char*)realloc(single_buffer_for_friendly_names, required_length);
            buflen = required_length;
        }
        strcpy(single_buffer_for_friendly_names, addr_buf);
    }
    return single_buffer_for_friendly_names;
}

void generateSensorPage(String *page, SENSOR_DATA sensor_data)
{
    char    print_buf[17];  // big enough for int or float or OneWire address
    char    token_buf[17];  // big enough for biggest markup token
    int     tmp_idx;
    page->replace("%%HUM%%",        printff(print_buf, sensor_data.hum_and_temp.humidity));
    page->replace("%%DHTTEMP%%",    printff(print_buf, sensor_data.hum_and_temp.temperature_c));
    page->replace("%%DHTLOC%%",     p_dhtlocation ? p_dhtlocation : "DHT");

    for (tmp_idx = 0; tmp_idx < 3; ++tmp_idx)
    {
        sprintf(token_buf, "%%%%TEMPID%d%%%%", tmp_idx + 1);
        page->replace(token_buf,    formatAddr(print_buf, sensor_data.temperature[tmp_idx].addr));
        sprintf(token_buf, "%%%%TEMPDESC%d%%%%", tmp_idx + 1);
        page->replace(token_buf,    getFriendlyOneWireName(sensor_data.temperature[tmp_idx].addr));
        sprintf(token_buf, "%%%%TEMP%d%%%%", tmp_idx + 1);
        page->replace(token_buf,    printff(print_buf, sensor_data.temperature[tmp_idx].temperature_c));
    }
}

int readDsTemp(OneWire *ds, int reset_search, TEMPERATURE_DATA *res)
{
    char i;
    char present = 0;
    char data[12];

    if (reset_search)
    {
#ifndef QUIET
        Serial.println("Reset search for onewire devices");
#endif
        ds->reset_search();
    }

    res->ok = DS_NOTHING_FOUND;
    if (!ds->search(res->addr))
    {
#ifndef QUIET
        Serial.println("End of onewire devices");
#endif
        return 0;
    }

    if (OneWire::crc8(res->addr, 7) != res->addr[7])
    {
#ifndef QUIET
        Serial.println("Invalid CRC");
#endif
        res->ok = DS_INVALID_CRC;
        return 1;
    }

    if (res->addr[0] != 0x10 and res->addr[0] != 0x28 and res->addr[0] != 0x22)
    {
#ifndef QUIET
        Serial.println("Unknown device");
#endif
        res->ok = DS_UNKNOWN_DEVICE;
        return 1;
    }

#ifndef QUIET
    Serial.println("Found onewire device ");
    showaddr(res->addr);
    Serial.println("");
#endif
    ds->reset();
    ds->select(res->addr);
    ds->write(0x44); // start conversion

    delay(1000);     // maybe 750ms is enough, maybe not

    present = ds->reset();
    ds->select(res->addr);    
    ds->write(0xBE);         // Read Scratchpad

    for (i = 0; i < sizeof data; i++)
    {
        data[i] = ds->read();
    }

    res->ok = ONEWIRE_OK;
    res->temperature_c = (float)( ((data[1] << 8) + data[0]) & 0xffff) / 16.0;
    res->temperature_f = res->temperature_c * 1.8 + 32.0;
#ifndef QUIET
    {
        char num_buf[7];
        Serial.print("temp (");
        sprintf(num_buf, "0x%x", ((data[1] << 8) + data[0]) & 0xffff);
        Serial.print(num_buf);
        Serial.print(") ");
        Serial.println(res->temperature_c);
    }
#endif
    return 1;   // try to find next sensor
}

void readSensors(SENSOR_DATA *result)
{
    int ds_start;
    TEMPERATURE_DATA temp_data;
    int got_values[MAX_TEMPERATURE_SENSORS];
    int end_wait_for_dht;
    DHT dht(persistent_data.dht22_pin, DHT22, 11);
    OneWire  *ds = new OneWire(persistent_data.onewire_pin);

    digitalWrite(LED_PIN, 0);
    dht.begin();
    end_wait_for_dht = millis() + 3000;

    ds_start = 1;
    memset(got_values, 0, sizeof got_values);
    pinMode(persistent_data.onewire_pin, INPUT_PULLUP);
    while (readDsTemp(ds, ds_start, &temp_data))
    {
        if (temp_data.ok == ONEWIRE_OK)
        {
            int first_empty = -1;
            int stored = 0;
            int i;
            for (i = 0; i < MAX_TEMPERATURE_SENSORS; ++i)
            {
                if (!memcmp(temp_data.addr, result->temperature[i].addr, sizeof temp_data.addr))
                {
#ifndef QUIET
                    Serial.print("replace temp for ");
                    showaddr(temp_data.addr);
                    Serial.print(" at idx ");
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.println(temp_data.temperature_c);
#endif
                    result->temperature[i] = temp_data;
                    stored = 1;
                    break;
                }
                if (first_empty < 0 && !result->temperature[i].addr[0])    // Assumes addr never starts with 0. May be invalid.
                {
                    first_empty = i;
                }
            }
            if (!stored && first_empty >= 0)
            {
                // Not seen this addr before, so put data in an empty slot.
#ifndef QUIET
                Serial.print("store temp for ");
                showaddr(temp_data.addr);
                Serial.print(" at idx ");
                Serial.print(first_empty);
                Serial.print(": ");
                Serial.println(temp_data.temperature_c);
#endif
                result->temperature[first_empty] = temp_data;
            }
        }
#ifndef QUIET
        else
        {
            Serial.println("not OK");
        }
#endif
        ds_start = 0;
    }

#ifndef QUIET
    Serial.print("Read DHT on pin ");
    Serial.println(persistent_data.dht22_pin);
#endif
    end_wait_for_dht -= millis();
    Serial.println(end_wait_for_dht);
    if (end_wait_for_dht > 0)
    {
        delay(end_wait_for_dht);
    }
    {
        float hum, temp;
        hum = dht.readHumidity();
        temp = dht.readTemperature(false);
        if (!isnan(hum) && !isnan(temp))
        {
            result->hum_and_temp.humidity = hum;
            result->hum_and_temp.temperature_c = temp;
            result->hum_and_temp.temperature_f = temp * 1.8 + 32.0;
        }
#ifndef QUIET
        else
        {
            // otherwise, leave previous values unchanged
            Serial.println("Failed to read from DHT sensor!");
        }
#endif
    }
    digitalWrite(LED_PIN, 1);
    delete ds;
}
