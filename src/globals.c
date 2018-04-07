/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include "globals.h"

uint8_t in_server_mode = 0;

char *new_etag = 0;

char *p_etag = 0;
char *p_ssid = 0;
char *p_passrot = 0;
char *p_lpswdrot = 0;
char *p_report_hostname = 0;
char *p_report_path = 0;
char *p_cfg_path = 0;
char *p_identifier = 0;
char *p_dhtlocation = 0;    // Friendly name for where the DHT is.
char *p_onewireids = 0; // map onewire devide IDs to friendly names, e.g. 2F588134=Brood box front|2F63815E=Outside

struct PERSISTENT_DATA persistent_data = {
     0, // port for server  Gets set in initial web set-up.
     0, // time_to_sleep            set zero for no sleep
    10, // time_between_reports     set zero for no delay
    12, // dht22_pin
    13, // onewire_pin
    3,  // rotate for passwords
};

// names of values that can be set from server and get saved to EEPROM
PERSISTENT_INFO persistents[] = {
    {PERS_UINT16, "port",                   &persistent_data.port},
    {PERS_UINT32, "time_to_sleep",          &persistent_data.time_to_sleep},
    {PERS_UINT32, "time_between_reports",   &persistent_data.time_between_reports},
    {PERS_UINT8,  "dht22_pin",              &persistent_data.dht22_pin},
    {PERS_UINT8,  "onewire_pin",            &persistent_data.onewire_pin},
    {PERS_INT8,   "rot",                    &persistent_data.rot},
    {0}
};

// names of string-type values that can be set from server and get saved to EEPROM
PERSISTENT_STRING_INFO persistent_strings[] = {
    {"etag",        &p_etag},   // this one is unusual in being set from an HTTP header, not from response content
    {"ssid",        &p_ssid},
    {"rotpass",     &p_passrot},
    {"rotlpswd",    &p_lpswdrot},
    {"rpthost",     &p_report_hostname},
    {"rptpath",     &p_report_path},
    {"cfgpath",     &p_cfg_path},
    {"ident",       &p_identifier},
    {"dhtloc",      &p_dhtlocation},
    {"onewireids",  &p_onewireids},
    {0}
};
