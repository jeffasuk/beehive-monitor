/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <string.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "globals.h"
#include "led.h"
#include "network.h"
#include "http.h"
#include "persistence.h"
#include "sensors.h"
#include "utils.h"

WiFiClient client;

char APtag[] = "beehivemonitor  ";

char sensors_page_xml[] =
        "<reading type=\"humidity\" ident=\"DHT\" desc=\"%%DHTLOC%%\" value=\"%%HUM%%\"/>\n"
        "<reading type=\"temperature\" ident=\"DHT\" desc=\"%%DHTLOC%%\" value=\"%%DHTTEMP%%\"/>\n"
        "<reading type=\"temperature\" ident=\"TEMPID1\" desc=\"%%TEMPDESC1%%\" value=\"%%TEMP1%%\"/>\n"
        "<reading type=\"temperature\" ident=\"TEMPID2\" desc=\"%%TEMPDESC2%%\" value=\"%%TEMP2%%\"/>\n"
;

static char *unrot(char *rotated_pswd)
{
    char *passclear, *in, *out;
    int lenpswd;
    lenpswd = strlen(rotated_pswd);
    passclear = (char*)malloc(lenpswd+1);
    for (in=rotated_pswd, out=passclear; *in; ++in, ++out)
    {
        *out = (char)(((((unsigned int)(*in) << 8) + (unsigned int)(*in)) >> (8 - persistent_data.rot) ) & 0xFF);
    }
    *out = 0;
    return passclear;
}

static void startAccessPoint()
{
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    char *tag_p;
    uint8_t *mac_ptr;

    WiFi.persistent(0); // Don't save config to flash.

    // Fill trailing spaces in tag with chars determined by the MAC address for a little uniqueness
    tag_p = APtag + sizeof APtag - 2;   // pointer to final char of APtag
    mac_ptr = mac + WL_MAC_ADDR_LENGTH - 1;
    WiFi.softAPmacAddress(mac);
    while (tag_p >= APtag && *tag_p == ' ' && mac_ptr >= mac)
    {
        *tag_p-- = 'A' + (*mac_ptr-- % 26);
    }
    if (p_lpswdrot)
    {
        char *passclear;
        passclear = unrot(p_lpswdrot);
        WiFi.softAP(APtag, passclear);
        WiFi.softAP(APtag, passclear);
        memset(passclear, 0, strlen(passclear)); // Don't leave the password lying about in memory.
        free(passclear);
    }
    else
    {
        WiFi.softAP(APtag, "beehiveESP");   // NB. Password must be at least 8 characters
    }
}

WiFiServer server(80);
static void startWebServer()
{
    server.begin();
    Serial.print("Set-up server started on address ");
    IPAddress myAddress = WiFi.localIP();
    Serial.println(myAddress);
}

void startServerMode()
{
    setLEDflashing(100, 100);
    startAccessPoint();
    startWebServer();
}

void handleSetupRequest()
{
    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    getRequest(client);
}

uint8_t connectWiFi()
{
    int ret = 0;
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(p_ssid);

    WiFi.persistent(0); // Don't save config to flash.

    /* I'm not sure of the exact behaviour of, or relationship between, WiFi.begin and WiFi.status.
        It would seem to be obvious that you 'begin' and then get status until the status is OK,
        but when I do that:
            - I always get a timeout
            - but the WiFi actually seems to be working OK
        The following method, of occasionally repeating the begin while checking status, seems to work.
    */
    setLEDpulse(1, 100, 0, 400);
    for (int i=0; WiFi.status() != WL_CONNECTED && i < 5; ++i)
    {
        char *passclear;
        unsigned long end_time;
        setLED();
        end_time = millis() + 6000;
        passclear = unrot(p_passrot);
        WiFi.begin(p_ssid, passclear);
        memset(passclear, 0, strlen(passclear)); // Don't leave the password lying about in memory.
        free(passclear);
        passclear = 0;
        delay(100);
        while ( (millis() < end_time) && WiFi.status() != WL_CONNECTED)
        {
            setLED();
            Serial.print(".");
            delay(100);
        }
    }
    Serial.println("");
    setLEDflashing(0, 0);
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("Timed out.");
        return 1;
    }
    Serial.print("WiFi connected: IP ");  
    Serial.println(WiFi.localIP());
    return 0;
}

int connectTCP()
{

#ifndef QUIET
    Serial.print("connecting to ");
    Serial.print(p_report_hostname);
    Serial.print(":");
    Serial.println(persistent_data.port);
#endif

    // Use WiFiClient class to create TCP connections
    if (! client.connect(p_report_hostname, persistent_data.port))
    {
#ifndef QUIET
        Serial.println("connection failed");
#endif
        return 1;
    }
#ifndef QUIET
    Serial.println("TCP connected");
#endif
    return 0;
}

static enum {GET_STATUS, GET_HEADERS, GET_BODY, END_OF_RESPONSE} http_response_state;

static int content_length = -1;
static int received_length;
static uint8_t changed_something;

static void handleHeader(char *name, char *value)
{
    if (!strcmp(name, "Content-Length"))
    {
        content_length = atoi(value);
    }
    else if (!strcmp(name, "ETag"))
    {
        strdupWithFree(value, &new_etag);
    }
#ifndef QUIET
    Serial.print("got header '");
    Serial.print(name);
    Serial.print("' : '");
    Serial.print(value);
    Serial.println("'");
#endif
}

// returns 0 at end-of-response, 1 otherwise
static int processLine(char *buf, int term_len)
{
    switch(http_response_state)
    {
      case GET_STATUS:
        {
            char *proto, *stat, *comment;
            proto = buf;
            stat = strchr(buf, ' ');
            *(stat++) = '\0';
            comment = strchr(stat, ' ');
            *(comment++) = '\0';
            http_response_state = GET_HEADERS;
            content_length = -1;
        }
        break;
      case GET_HEADERS:
        // NB. Does not handle continuation lines
        if (*buf == '\0')
        {
#ifndef QUIET
            Serial.println("End of headers");
#endif
            if (content_length == 0)
            {
#ifndef QUIET
                Serial.println("End of empty response");
#endif
                return 0;
            }
            http_response_state = GET_BODY;
            received_length = 0;
            changed_something = 0;
        }
        else
        {
            char *value;
            value = strchr(buf, ':');
            if (value > 0)
            {
                *(value++) = '\0';
                while (*value && *value == ' ')
                {
                    value++;
                }
                handleHeader(buf, value);
            }
        }
        break;
      case GET_BODY:
        // each line is name=value
        {
            char *value_str;
            received_length += strlen(buf) + term_len;
            if ( (value_str = strchr(buf, '=')) > 0)
            {
                *(value_str++) = '\0';
#ifndef QUIET
                Serial.print(buf);
                Serial.print(" = '");
                Serial.print(value_str);
                Serial.println("'");
#endif
                if (setPersistentValue(buf, value_str))
                {
#ifndef QUIET
                    Serial.println("        changed");
#endif
                    changed_something = 1;
                    break;
                }
            }
        }
        if (content_length >= 0 && received_length >= content_length)
        {
#ifndef QUIET
            Serial.println("End of response");
#endif
            return 0;
        }
        break;
    }
    return 1;
}

void getResponse()
{
    unsigned long timeout;
    int read_len;
    int got_so_far;
    timeout = millis();
    while (!client.available())
    {
        setLED();
        if ( (millis() - timeout) > 5000)
        {
#ifndef QUIET
            Serial.println(">>> Client Timeout !");
#endif
            client.stop();
            return;
        }
    }
    http_response_state = GET_STATUS;
    content_length = 0;
    got_so_far = 0;
    while (client.available() && http_response_state != END_OF_RESPONSE)
    {
        char inbuf[80];
        char *p;
        setLED();
        read_len = client.read((uint8_t*)&inbuf[got_so_far], 80 - got_so_far - 1);
        if (read_len <= 0)
        {
#ifndef QUIET
            Serial.println("End of reading");
#endif
            if (inbuf[0])
            {
                processLine(inbuf, 0);
            }
            break;
        }
        got_so_far += read_len;
        inbuf[got_so_far] = '\0';
        if ( (p = strchr(inbuf, '\n')) > 0)
        {
            // at least one line
            int extra_data_len;
            int term_len;
            while ( (p = strchr(inbuf, '\n')) > 0)
            {
                *p = '\0';
                term_len = 1;
                if (p > inbuf && *(p-1) == '\r')
                {
                    term_len = 2;
                    *(p-1) = '\0';
                }
                if (!processLine(inbuf, term_len))
                {
                    return;
                }
                ++p;
                extra_data_len = got_so_far - (p - inbuf );
                if (extra_data_len <= 0)
                {
                    inbuf[0] = '\0';
                    got_so_far = 0;
                    break;
                }
                // need to move text after the \n to start of buffer
                memmove(inbuf, p, extra_data_len);
                inbuf[extra_data_len] = '\0';
                got_so_far = extra_data_len;
            }
        }
    }
    client.stop();
}

uint8_t getSettings()
{
    // get any changed settings
#ifndef QUIET
    Serial.println("get settings");
#endif
    changed_something = 0;
    setLEDpulse(2, 50, 50, 400);
    if (!connectTCP())
    {
        client.print("GET ");
        client.print(p_cfg_path);
        client.print("?ident=");
        client.print(p_identifier);
        client.print(" HTTP/1.0\r\n");
        if (p_etag)
        {
            client.print("If-None-Match:");
            client.print(p_etag);
            client.print("\r\n");
        }
        client.print("\r\n");
        getResponse();
    }
    setLEDflashing(0, 0);
    return changed_something;
}

void sendReport()
{
    char buf[10];   // enough for a number
    int i;
    SENSOR_DATA sensor_data = {0};

#ifndef QUIET
    Serial.println("sendReport");
#endif
    if (!connectTCP())
    {
        // send readings
        // Also potentially collect any changed settings, although we shouldn't get any in response to a report.
        String page = sensors_page_xml;
        readSensors(&sensor_data);
        setLEDpulse(3, 40, 40, 300);
        //setLED();
        generateSensorPage(&page, sensor_data);
        client.print("POST ");
        client.print(p_report_path);
        client.print("?ident=");
        client.print(p_identifier);
        client.print(" HTTP/1.0\r\nContent-Type:text/xml\r\nContent-Length:");
        client.print(page.length());
        client.print("\r\n\r\n");
        client.print(page);
        getResponse();
    }
    setLEDflashing(0, 0);
}
