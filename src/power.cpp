/* Licensed under GNU General Public License v3.0
  See https://github.com/jeffasuk/beehive-monitor/blob/master/LICENSE
  jeff at jamcupboard.co.uk
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>


void hw_sleep(int nb_sec)
{
#ifndef QUIET
    Serial.print("sleep ");
    Serial.print(nb_sec);
    Serial.println(" seconds");
#endif
    ESP.deepSleep(nb_sec * 1000000);
}
