# beehive-monitor
An ESP8266-based remote monitor for beehives.

This is designed to take readings from a DHT22 (or DHT11) combined humidity and temperature sensor, a weight sensor, and a number of further temperature sensors using the OneWire protocol (e.g. ds18b20). Its main expected mode of operation is to wake occasionally from deep sleep, connect to a local WiFi access point and report current sensor readings to an HTTP server. Alternatively, it can create its own local WiFi access point and web server from which sensor readings can be obtained by a client connecting to that access point. It can read configuration settings from the same HTTP server that receives the reports, allowing a certain amount of remote administration.
