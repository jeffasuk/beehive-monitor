/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <Arduino.h>    // for the various int typedefs

// to switch off serial reporting
//#define QUIET

#define LED_PIN 2
#define SETUP_PIN 14

// sensors
#define MAX_TEMPERATURE_SENSORS 8

extern uint8_t in_server_mode;
extern char *new_etag;

// settable
extern char *p_etag;
extern char *p_ssid;
extern char *p_passrot;
extern char *p_lpswdrot;
extern char *p_report_hostname;
extern char *p_report_path;
extern char *p_cfg_path;
extern char *p_identifier;
extern char *p_dhtlocation;
extern char *p_onewireids;

struct PERSISTENT_DATA {    // this structure can be stored in EEPROM
    uint16_t port;
    uint32_t time_to_sleep;
    uint32_t time_between_reports;
    uint8_t dht22_pin;
    uint8_t onewire_pin;
    int8_t  rot;
};
extern struct PERSISTENT_DATA persistent_data;

// This defines the datatypes, names and where to store them in runtime memory
typedef enum { PERS_INT8, PERS_INT16, PERS_INT32, PERS_UINT8, PERS_UINT16, PERS_UINT32, PERS_FLOAT, PERS_STR } PERSISTENT_DATA_TYPE;
struct PERSISTENT_INFO_STR {
    PERSISTENT_DATA_TYPE    type;
    char    *name;
    void    *value;
};
typedef struct PERSISTENT_INFO_STR PERSISTENT_INFO;
extern PERSISTENT_INFO persistents[];

// This defines the names and runtime storage locations for string-type data to be stored in EEPROM
struct PERSISTENT_STRING_INFO_STR {
    char    *name;
    char    **value;    // where to put a pointer to malloced area for the data itself
};
typedef struct PERSISTENT_STRING_INFO_STR PERSISTENT_STRING_INFO;
extern PERSISTENT_STRING_INFO persistent_strings[];
