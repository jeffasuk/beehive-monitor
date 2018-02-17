/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
typedef enum {
    DS_OK = 0,
    DS_NOTHING_FOUND,
    DS_INVALID_CRC,
    DS_UNKNOWN_DEVICE
} DS_RESULT_CODE;

#define DHTLIB_OK 0
#define DHTLIB_ERROR_CHECKSUM -1
#define DHTLIB_ERROR_TIMEOUT -2

// inter-module i/f
typedef struct {
    int     ok;
    float   humidity;
    float   temperature_f;
    float   temperature_c;
} HUMIDITY_AND_TEMPERATURE_DATA;
typedef struct {
    int             ok;
    unsigned char   addr[8];
    float           temperature_f, temperature_c;
} TEMPERATURE_DATA;

typedef struct {
    HUMIDITY_AND_TEMPERATURE_DATA hum_and_temp;
    int     nb_temperature_sensors;
    TEMPERATURE_DATA temperature[MAX_TEMPERATURE_SENSORS];
} SENSOR_DATA;

// OneWire return codes
#define ONEWIRE_OK  0
#define ONEWIRE_CHKSUM_ERR -1
#define ONEWIRE_TIMEOUT -2

void readSensors(SENSOR_DATA *result);
int readDsTemp(int start, TEMPERATURE_DATA *res);
extern void generateSensorPage(String *page, SENSOR_DATA sensor_data);
