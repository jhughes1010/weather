# Weather
weather station scratch code
This is a dev area for working the deep sleep integration on the opengreenenergy weatherstation on instructables.com
https://www.instructables.com/Solar-Powered-WiFi-Weather-Station-V30/

##Still trying to debug my 3.3V operation issues. It works well using 5V USB power



## MCU will wake for 3 reasons:

1. Initial power on - When device powers on, MCU will briefly connect to the WiFi to get current time.

2. EXT0 - ESP32 will wake on rain tip counter to increment counter and go back to sleep.

3. Timer - wakes at set time interval to read sensors and send to IOT data collection point.


## Open Items - Software
Rainfall in last 24h is currently last 23h, I'll circle back and fix this when everything else is working.
