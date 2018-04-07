/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
/* Handler for HTTP requests when in set-up mode.
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "globals.h"
#include "network.h"
#include "eepromutils.h"
#include "persistence.h"
#include "sensors.h"
#include "utils.h"

//static String favicon(
#define FAV_LEN 1150
static char favicon[FAV_LEN+1] =
"\x00\x00\x01\x00\x01\x00\x10\x10\x00\x00\x01\x00\x20\x00\x68\x04"
"\x00\x00\x16\x00\x00\x00\x28\x00\x00\x00\x10\x00\x00\x00\x20\x00"
"\x00\x00\x01\x00\x20\x00\x00\x00\x00\x00\x00\x04\x00\x00\x12\x0b"
"\x00\x00\x12\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff"
"\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xf8\xfa\xfa\xff\x7c\xc1"
"\xe5\xff\x22\x9f\xdd\xff\xaa\xd6\xec\xff\xff\xff\xfe\xff\xff\xff"
"\xff\xff\xb3\xda\xed\xff\x39\xa3\xdb\xff\x79\xbd\xe0\xff\xee\xf3"
"\xf3\xff\xfa\xf8\xf6\xff\xf6\xf6\xf7\xff\xf9\xf9\xf9\xff\xfd\xfe"
"\xff\xff\xff\xff\xff\xff\xdb\xec\xf6\xff\x33\xa0\xe2\xff\x02\x8e"
"\xeb\xff\x09\x92\xf1\xff\x02\x90\xe8\xff\x5e\xb5\xe6\xff\x66\xb6"
"\xe5\xff\x05\x8e\xe6\xff\x0a\x92\xf1\xff\x03\x91\xec\xff\x2a\xa0"
"\xe4\xff\xce\xe5\xf3\xff\xff\xff\xff\xff\xfd\xfe\xff\xff\xfb\xfd"
"\xfe\xff\xff\xff\xff\xff\xab\xd1\xef\xff\x00\xa5\xef\xff\x0e\xc7"
"\xf9\xff\x08\xc5\xf6\xff\x0b\xc3\xfa\xff\x00\x99\xe6\xff\x00\x95"
"\xe3\xff\x09\xc4\xfa\xff\x08\xc5\xf6\xff\x0f\xc6\xf9\xff\x00\xa7"
"\xf0\xff\x97\xca\xee\xff\xff\xff\xff\xff\xfb\xfd\xfe\xff\xfa\xfd"
"\xfe\xff\xff\xff\xff\xff\xb2\xd3\xf1\xff\x00\xca\xf5\xff\x02\xe6"
"\xfb\xff\x1f\xe1\xfa\xff\x54\xec\xff\xff\x1f\xb2\xec\xff\x08\xab"
"\xe9\xff\x03\xea\xfe\xff\x0a\xe0\xf9\xff\x51\xe7\xff\xff\x28\xd0"
"\xf8\xff\x9a\xcc\xee\xff\xff\xff\xff\xff\xfa\xfd\xfe\xff\xfb\xfd"
"\xfe\xff\xff\xff\xff\xff\xa1\xcc\xe7\xff\x00\xa4\xe4\xff\x08\xb8"
"\xf9\xff\x42\xd1\xfd\xff\x6b\xe0\xfa\xff\x29\x9e\xd5\xff\x0e\x96"
"\xd6\xff\x02\xb8\xf8\xff\x25\xc2\xfa\xff\x6d\xe6\xff\xff\x22\xb3"
"\xe7\xff\x8c\xc4\xe4\xff\xff\xff\xff\xff\xfa\xfd\xfe\xff\xf4\xf8"
"\xfa\xff\x83\xc3\xe7\xff\x12\x96\xe0\xff\x0d\x8e\xd3\xff\x0a\x94"
"\xd8\xff\x35\xae\xe1\xff\x2e\x9a\xcb\xff\x1d\x90\xd4\xff\x27\x94"
"\xd2\xff\x18\x91\xce\xff\x1f\xa7\xe4\xff\x1b\xa4\xdb\xff\x07\x8a"
"\xd3\xff\x13\x97\xe0\xff\x84\xc5\xe7\xff\xf4\xf8\xfa\xff\x2c\x9e"
"\xe2\xff\x00\x90\xeb\xff\x0c\x99\xf4\xff\x0a\x9c\xf4\xff\x11\x95"
"\xe0\xff\x28\x8d\xc8\xff\x19\x95\xe2\xff\x23\x9b\xe9\xff\x1e\x9c"
"\xeb\xff\x27\x9b\xe0\xff\x1b\x8e\xcd\xff\x09\x8e\xe0\xff\x0f\x9a"
"\xf3\xff\x0a\x99\xf4\xff\x00\x93\xed\xff\x2b\xa0\xe3\xff\x06\xa7"
"\xea\xff\x0b\xd3\xfa\xff\x06\xd3\xf8\xff\x0f\xd0\xfa\xff\x1b\xbc"
"\xf0\xff\x25\x9b\xdc\xff\x20\xc5\xed\xff\x10\xd2\xf3\xff\x19\xce"
"\xf1\xff\x29\xc4\xed\xff\x21\x9d\xdf\xff\x0a\xbd\xf4\xff\x06\xd4"
"\xf9\xff\x09\xd2\xf8\xff\x18\xcf\xfb\xff\x08\xa6\xeb\xff\x05\xb4"
"\xec\xff\x05\xe0\xfd\xff\x12\xda\xf8\xff\x5f\xe7\xff\xff\x45\xd4"
"\xf6\xff\x24\x9a\xd5\xff\x15\xd7\xf5\xff\x1e\xd1\xea\xff\x54\xd8"
"\xef\xff\x65\xe1\xf7\xff\x22\x99\xda\xff\x04\xcc\xf7\xff\x00\xdd"
"\xfa\xff\x30\xdd\xfa\xff\x68\xec\xff\xff\x1a\xb5\xeb\xff\x2c\x9e"
"\xda\xff\x00\xa4\xed\xff\x27\xbf\xfa\xff\x5d\xdd\xfc\xff\x20\xa9"
"\xdf\xff\x1f\x8d\xc6\xff\x26\xac\xde\xff\x23\xb2\xef\xff\x64\xd6"
"\xf5\xff\x50\xbd\xdf\xff\x0e\x89\xcb\xff\x0b\xa1\xdf\xff\x08\xae"
"\xf5\xff\x45\xd2\xfe\xff\x3b\xc7\xf1\xff\x2f\x9d\xd8\xff\xf5\xf4"
"\xf8\xff\x82\xbc\xe0\xff\x1a\x9c\xda\xff\x0e\x93\xd2\xff\x08\x89"
"\xd9\xff\x0d\x94\xe7\xff\x1d\x90\xd2\xff\x28\x97\xce\xff\x2a\x9b"
"\xcf\xff\x0e\x8b\xd3\xff\x0c\x93\xe7\xff\x10\x8d\xd8\xff\x0c\x8f"
"\xd2\xff\x1b\x9e\xda\xff\x7f\xbc\xdf\xff\xf2\xf3\xf8\xff\xfb\xfd"
"\xfe\xff\xff\xff\xff\xff\x9b\xc8\xe5\xff\x00\x8f\xe5\xff\x12\xa5"
"\xf7\xff\x0c\xa3\xf4\xff\x05\xa3\xf6\xff\x09\x94\xe0\xff\x09\x92"
"\xe0\xff\x0d\xa2\xf6\xff\x0e\xa3\xf5\xff\x0b\xa6\xf7\xff\x00\x93"
"\xe6\xff\xa5\xcb\xe6\xff\xff\xff\xff\xff\xfb\xfd\xfe\xff\xfa\xfd"
"\xfe\xff\xff\xff\xff\xff\xae\xd4\xf1\xff\x00\xc1\xf3\xff\x06\xe2"
"\xfa\xff\x0c\xdc\xf9\xff\x25\xdb\xfc\xff\x15\xad\xed\xff\x0d\xb3"
"\xee\xff\x05\xe1\xfb\xff\x06\xdd\xf9\xff\x25\xdc\xfb\xff\x0c\xbc"
"\xf3\xff\xb9\xd8\xf2\xff\xff\xff\xff\xff\xfb\xfd\xfe\xff\xfb\xfd"
"\xfe\xff\xff\xff\xff\xff\xa8\xcc\xec\xff\x00\xbc\xf1\xff\x05\xd5"
"\xfb\xff\x37\xd8\xfa\xff\x72\xef\xff\xff\x17\xa7\xe6\xff\x00\xa8"
"\xe8\xff\x03\xd8\xfe\xff\x20\xd3\xf8\xff\x75\xed\xff\xff\x24\xc0"
"\xf1\xff\xad\xcf\xed\xff\xff\xff\xff\xff\xfc\xfd\xfe\xff\xfd\xfe"
"\xfe\xff\xff\xff\xff\xff\xd9\xe6\xf3\xff\x2f\xa1\xdb\xff\x00\xa0"
"\xeb\xff\x38\xc8\xfb\xff\x2f\xba\xe9\xff\x5e\xab\xdb\xff\x58\xac"
"\xdd\xff\x00\x9c\xe6\xff\x2a\xbe\xf9\xff\x39\xc6\xf0\xff\x3e\xa5"
"\xda\xff\xdf\xe9\xf5\xff\xff\xff\xff\xff\xfd\xfe\xff\xff\xff\xff"
"\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xf7\xf5\xf8\xff\x7a\xb8"
"\xdd\xff\x24\x97\xd3\xff\xa5\xca\xe4\xff\xff\xfe\xfe\xff\xff\xfd"
"\xfd\xff\xa2\xc9\xe4\xff\x23\x96\xd3\xff\x7d\xb9\xdd\xff\xf7\xf6"
"\xf9\xff\xff\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
;

// fields in the settings page, for storing in EEPROM
struct {
    char    *html_field_name;
    char    *persistent_item_name;
    char    *display_name;  // unused. Should be used in generating the HTML form.
} name_mapping[] = {
    {"ssid",    "ssid",             "WiFi SSID",},
    {"pswd",    "rotpass",          "WiFi password",},
    {"lpswd",   "rotlpswd",         "Password for set-up access point",},
    {"ident",   "ident",            "Unit identifier",},
    {"host",    "rpthost",          "Server hostname",},
    {"port",    "port",             "Server port",},
    {"cpath",   "cfgpath",          "Settings path",},
    {"rpath",   "rptpath",          "Report path",},
    {0}
};

static int getNameIndex(const char *name)
{
    int i;
    for (i = 0; name_mapping[i].html_field_name; ++i)
    {
        if (!strcmp(name, name_mapping[i].html_field_name))
        {
            return i;
        }
    }
    return -1;
}

static const char page_head[] = \
        "<html>\n"
        "<head><title>ESP beehive monitor</title></head>\n"
        "<body>\n";
static const char page_tail[] = "</body></html>\n";

char OK_head[] = \
        "HTTP/1.0 200 OK\r\n"
        "Content-Type:text/html\r\n"
        "Server:ESP beehive monitor\r\n";

char home_page[] = \
        "<h1>Welcome to the ESP8266-based Beehive Monitor</h1>\n"
        "<p>Choose one of the following options:\n"
        "<ul>\n"
        "<li><a href='setup'>Initialize/change settings</a></li>\n"
        "<li><a href='sensors'>Show current sensor readings</a></li>\n"
        "<li><a href='sensors?refr=5'>Show current sensor readings with refresh</a></li>\n"
        "</ul>\n"
        ;
char settings_form_page[] = \
        "<form method='POST' action='/setup' >\n"
        "%%EXTRA%%"
        "<table>\n"
        "<tr><td>WiFi SSID</td><td><input type=\"text\" name=\"ssid\" width=\"30\" value=\"%%SSID%%\"></td></tr>\n"
        //"<tr><td>WiFi password</td><td><input type=\"password\" name=\"pswd\" width=\"30\"></td></tr>\n"
        "<tr><td>WiFi password (visible)</td><td><input type=\"text\" name=\"pswd\" width=\"30\"></td></tr>\n"
        "<tr><td>Unit identifier</td><td><input type=\"text\" name=\"ident\" width=\"30\" value=\"%%IDENT%%\"></td></tr>\n"
        "<tr><td>Server hostname</td><td><input type=\"text\" name=\"host\" width=\"30\" value=\"%%HOST%%\"></td></tr>\n"
        "<tr><td>Server port</td><td><input type=\"text\" name=\"port\" width=\"30\" value=\"%%PORT%%\"></td></tr>\n"
        "<tr><td>Settings path</td><td><input type=\"text\" name=\"cpath\" width=\"30\" value=\"%%CFGPATH%%\"></td></tr>\n"
        "<tr><td>Report path</td><td><input type=\"text\" name=\"rpath\" width=\"30\" value=\"%%RPTPATH%%\"></td></tr>\n"
        //"<tr><td>Password for set-up access point</td><td><input type=\"password\" name=\"lpswd\" width=\"30\"></td></tr>\n"
        "<tr><td>Password for set-up access point (visible)</td><td><input type=\"text\" name=\"lpswd\" width=\"30\"></td></tr>\n"
        "</table>\n"
        "<input type=\"checkbox\" name=\"monmode\" value=\"Start monitoring\">\n"
        "<br><input type=\"submit\" value=\"Send\">\n"
        "</form>\n"
        ;
char sensors_page[] = \
        "<h2>Current readings</h2>\n"
        "<table border=1>\n"
        "<tr><th>Sensor</th><th>ID</th><th>Value</th></hr>\n"
        "<tr><td>Humidity</td><td>%%DHTLOC%%</td><td>%%HUM%%</td></tr>\n"
        "<tr><td>Temperature</td><td>%%DHTLOC%%</td><td>%%DHTTEMP%%</td></tr>\n"
        "<tr><td>Temperature</td><td><!-- %%TEMPID1%% -->%%TEMPDESC1%%</td><td>%%TEMP1%%</td></tr>\n"
        "<tr><td>Temperature</td><td><!-- %%TEMPID2%% -->%%TEMPDESC2%%</td><td>%%TEMP2%%</td></tr>\n"
        "<tr><td>Temperature</td><td><!-- %%TEMPID3%% -->%%TEMPDESC3%%</td><td>%%TEMP3%%</td></tr>\n"
        "</table>\n"
        ;
static enum {GET_FIRST_LINE, GET_HEADERS, GET_BODY} http_message_state;

// Data content of any in-flight message
char *path = 0;
char *qry = 0;
char *command = 0;
int content_length = 0;
char *req_body = 0;
int req_body_space = 0;
const int body_growth = 512;

int response_status;

typedef bool (*ptr_to_process_line_func)(char*,int);

static void getMessage(WiFiClient client,
            ptr_to_process_line_func process_first_line,
            ptr_to_process_line_func process_header_line,
            ptr_to_process_line_func process_body_line
    )
{
    unsigned long timeout;
    char inbuf[1024];
    int read_len;
    int got_so_far;
    ptr_to_process_line_func processLine[3] = {
            process_first_line,
            process_header_line,
            process_body_line
        };
    timeout = millis();
    while (!client.available())
    {
        if ( (millis() - timeout) > 5000)
        {
#ifndef QUIET
            Serial.println(">>> Client Timeout !");
#endif
            client.stop();
            return;
        }
    }
    http_message_state = GET_FIRST_LINE;
    content_length = 0;
    got_so_far = 0;
    while (client.available() && http_message_state != GET_BODY)
    {
        char *p;
        int to_read;
        to_read = (sizeof inbuf) - got_so_far - 1;
        if (to_read <= 0)
        {
            Serial.println("Line too long for our buffer. Aborting transaction.");
            return;
        }
        read_len = client.read((uint8_t*)&inbuf[got_so_far], to_read);
        if (read_len <= 0)
        {
#ifndef QUIET
            Serial.println("End of reading");
#endif
            if (inbuf[0])
            {
                (*processLine[http_message_state])(inbuf, 0);
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
                if (!(*processLine[http_message_state])(inbuf, term_len))
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
    if (inbuf[0])
    {
        (*processLine[http_message_state])(inbuf, 0);
        inbuf[0] = '\0';
    }
    while (client.available() && http_message_state == GET_BODY)
    {
        read_len = client.read((uint8_t*)inbuf, sizeof inbuf - 1);
        if (read_len <= 0)
        {
            break;
        }
        inbuf[read_len] = '\0';
        (*process_body_line)(inbuf, 0);
    }
}

// processLine functions return 0 at end-of-message, 1 otherwise
static bool getCommandLine(char *inbuf, int term_len)
{
    char *start_of_path, *start_of_qry, *end;
#ifndef QUIET
    Serial.print("Command line: ");
    Serial.print(inbuf);
    Serial.println("'");
#endif
    if ( (start_of_path = strchr(inbuf, ' ')) <= 0)
    {
#ifndef QUIET
        Serial.println("no space");
#endif
        return 0;
    }
    *start_of_path++ = '\0';
    if ( (start_of_qry = strchr(start_of_path, '?')) > 0)
    {
        // found query string
        *start_of_qry++ = '\0';
        end = strchr(start_of_qry, ' ');
    }
    else
    {
        end = strchr(start_of_path, ' ');
    }
    if (end <= 0)
    {
#ifndef QUIET
        Serial.println("no space 2");
#endif
        return 0;
    }
    *end = '\0';
    strdupWithFree(inbuf, &command);
    strdupWithFree(start_of_path, &path);
    strdupWithFree(start_of_qry, &qry);
#ifndef QUIET
    Serial.print("command: ");
    Serial.print(command);
    Serial.println("'");
    Serial.print("path: '");
    Serial.print(path);
    Serial.println("'");
#endif
    http_message_state = GET_HEADERS;
    return 1;   // command is never end-of-message, except on error
}

static bool getHeader(char *inbuf, int term_len)
{
    char *p1;
    // Not allowing for continuation lines (yet!)
#ifndef QUIET
    Serial.print("Header line: '");
    Serial.print(inbuf);
    Serial.println("'");
#endif
    if (!*inbuf)
    {
        http_message_state = GET_BODY;
        if (content_length > 0)
        {
            return 1;
        }
        return 0;
    }
    if ( (p1 = strchr(inbuf, ':')) <= 0)
    {
        return 0;
    }
    *p1++ = '\0';
    if (!strcmp(inbuf, "Content-Length"))
    {
        content_length = atoi(p1);
    }
    return 1;   // a header is never end-of-message if there's a body, except on error
}

static bool getRequestBodyLine(char *inbuf, int term_len)
{
    int len_buf = strlen(inbuf);
    if (!req_body)
    {
        int sz = body_growth;
        if (sz <= len_buf)
        {
            sz += len_buf;
        }
        req_body = (char *)malloc(sz);
        *req_body = '\0';
        req_body_space = sz;
    }
    else
    {
        int len_req_body = strlen(req_body);
        if (req_body_space <= (len_req_body + len_buf))
        {
            int sz = req_body_space + body_growth;
            if (sz <= (len_req_body + len_buf))
            {
                sz += len_buf;
            }
            req_body = (char *)realloc(req_body, sz);
            req_body_space = sz;
        }
    }
    strcat(req_body, inbuf);
}
static bool getStatusLine(char *inbuf, int term_len)
{
}
static bool getResponseBodyLine(char *inbuf, int term_len)
{
}

static void processHomePath(WiFiClient client)
{
    client.print(OK_head);
    client.print("\r\n");
    client.print(page_head);
    client.print(home_page);
    client.print(page_tail);
}

static void processSettingsPath(WiFiClient client)
{
    char    *p, *name, *value;
    uint8_t changed_something = 0;
    uint8_t switch_to_monitor_mode = 0;
    String post_extra_response;
    // We'll end up sending the form page, but for POST, it will have extra stuff in it to indicate
    // how the changes went.
    if (!strcmp(command, "POST"))
    {
        // parse input. Assumes application/x-www-form-urlencoded
        p = req_body;
        while (p && *p)
        {
            int item_index;
            name = p;
            if ( (p = strchr(p, '=')) <= 0)
            {
                break;
            }
            *p++ = '\0';
            value = p;
            if ( (p = strchr(p, '&')) > 0)
            {
                *p++ = '\0';
            }
            if (!strcmp(name, "monmode"))
            {
                switch_to_monitor_mode = 1;
                continue;
            }
            if ( (item_index = getNameIndex(name)) >= 0)
            {
                // found it
                char *src_p, *dst_p;
                char *item_name = name_mapping[item_index].persistent_item_name;
                // unescape HTTP %NN form. No need for a new string, as %NN always gets shorter.
                dst_p = 0;
                for (src_p = value; *src_p; ++src_p)
                {
                    if (*src_p == '+')
                    {
                        // replace + with space in situ before continuing with %-substitution and possible copying from src to dst
                        *src_p = ' ';
                    }
                    if (*src_p == '%')
                    {
                        if (!dst_p)
                        {
                            dst_p = src_p;
                        }
                        if (*(src_p+1) == '%')
                        {
                            // double %, so store just one of them
                            *dst_p++ = '%';
                            src_p++;    // skip the second one
                        }
                        else if (*(src_p+1) && *(src_p+2))
                        {
                            // and we have 2 further characters (should really check that they're hex digits)
                            char num_str[3];
                            num_str[0] = *(++src_p);
                            num_str[1] = *(++src_p);
                            num_str[2] = 0;
                            //*dst_p++ = (char) atoi(num_str, 16);
                            *dst_p++ = (char) strtol(num_str, 0, 16);
                        }
                    }
                    else if (dst_p)
                    {
                        *dst_p++ = *src_p;
                    }
                }
                if (dst_p)
                {
                    *dst_p = 0;
                }
                if (!strncmp(item_name, "rot", 3))
                {
                    // It's some sort of password to be obfuscated for storage
                    for (src_p=value; *src_p; ++src_p)
                    {
                        *src_p = (char)(((((unsigned int)(*src_p) << 8) + (unsigned int)(*src_p)) >> persistent_data.rot ) & 0xFF);
                    }
                    *src_p = 0;
                }
                if (setPersistentValue(item_name, value))
                {
                    changed_something = 1;
                }
            }
        }
        if (changed_something)
        {
            // Now check settings. Return appropriate page
            // If OK, write to EEPROM, and then enter normal running.
            uint8_t res;
            Serial.println("Writing settings to EEPROM");
            writeToEeprom();
            post_extra_response += "<p><b>Settings saved to EEPROM.</b>\n";
            Serial.println("Check connection to WiFi");
            if ( (res = connectWiFi()) != 0)
            {
                // failed
                Serial.println("Failed to connect to WiFi");
                post_extra_response += "<p><b>Failed to connect to WiFi '";
                post_extra_response += "'</b>\n<br>Please check settings</p>\n";
            }
            else if ( Serial.println("Check connection to TCP"), (res = connectTCP()) != 0)
            {
                // failed
                Serial.println("Failed to connect to web server");
                post_extra_response += "<p><b>Failed to connect to web server '";
                post_extra_response += "'</b>\n<br>Please check settings</p>\n";
            }
            else
            {
                post_extra_response += "<br>Connected successfully to WiFi and web server\n";
                if (switch_to_monitor_mode)
                {
                    post_extra_response += "<br>Switching to monitoring mode.\n<br>This website is now unavailable.</p>\n";
                    in_server_mode = 0;
                }
            }
        }
    }
    String page = settings_form_page;
    client.print(OK_head);
    page.replace("%%EXTRA%%", post_extra_response);
    page.replace("%%SSID%%", String(p_ssid ? p_ssid : ""));
    page.replace("%%IDENT%%", String(p_report_hostname ? p_identifier : ""));
    page.replace("%%HOST%%", String(p_report_hostname ? p_report_hostname : ""));
    page.replace("%%PORT%%", String(persistent_data.port));
    page.replace("%%CFGPATH%%", String(p_cfg_path ? p_cfg_path : ""));
    page.replace("%%RPTPATH%%", String(p_report_path ? p_report_path : ""));
    client.print("Content-Length:");
    client.print(strlen(page_head) + page.length() + strlen(page_tail));
    client.print("\r\n\r\n");
    client.print(page_head);
    client.print(page);
    client.print(page_tail);
}

static void processSensorsPath(WiFiClient client)
{
    SENSOR_DATA sensor_data = {0};
    readSensors(&sensor_data);
    String page = sensors_page;
    generateSensorPage(&page, sensor_data);
    client.print(OK_head);
    if (qry && !strncmp(qry, "refr=", 5))   // ignore the value, because...
    {
        client.print("Refresh:0;\r\n"); // ...there's not a lot of point waiting, as reading sensors is slow anyway
    }
    client.print("Content-Length:");
    client.print(strlen(page_head) + page.length() + strlen(page_tail));
    client.print("\r\n\r\n");
    client.print(page_head);
    client.print(page);
    client.print(page_tail);
}

static void processFaviconPath(WiFiClient client)
{
    int i;
    client.print(OK_head);
    client.print("Content-Length:");
    client.print(FAV_LEN);
    client.print("\r\nContent-Type:image/x-icon\r\n\r\n");
    // This is horrible, but I couldn't find a way to write the whole icon in one go, as there are NULs in it.
    for (i=0; i<FAV_LEN; ++i)
    {
        client.print(favicon[i]);
    }
}

static void send404(WiFiClient client)
{
    client.print("HTTP/1.0 404 Not found\r\nContent-Length:0\r\n\r\n");
}

struct despatch_item {
    char *path;
    void (*handler)(WiFiClient);
} despatch_table[] = {
    {"/",            processHomePath},
    {"/setup",       processSettingsPath},
    {"/sensors",     processSensorsPath},
    {"/favicon.ico", processFaviconPath},
    {0}
};

void getRequest(WiFiClient client)
{
    struct despatch_item *despatch_lookup;
    getMessage(client, &getCommandLine, &getHeader, &getRequestBodyLine);
    for (despatch_lookup = despatch_table; despatch_lookup->path; ++despatch_lookup)
    {
        Serial.print("compare '");
        Serial.print(path);
        Serial.print("' against '");
        Serial.print(despatch_lookup->path);
        Serial.println("'");
        if (!strcmp(despatch_lookup->path, path))
        {
#ifndef QUIET
            Serial.print("Handler for ");
            Serial.println(path);
#endif
            despatch_lookup->handler(client);
            return;
        }
    }
    // Didn't find a handler for that path, so it's a 404
    send404(client);
}

void getResponse(WiFiClient client)
{
    getMessage(client, &getStatusLine, &getHeader, &getResponseBodyLine);
}
