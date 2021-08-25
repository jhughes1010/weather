# Weather
weather station scratch code
This is a dev area for working the deep sleep integration on the opengreenenergy weatherstation on instructables.com
https://www.instructables.com/Solar-Powered-WiFi-Weather-Station-V30/

My build notes and ramblings can be found at https://jameshughes.atlassian.net/wiki/spaces/WSV/overview


## MCU will wake for 3 reasons:

1. [![Generic badge](https://img.shields.io/badge/BOOT-OK-blue.svg)](https://shields.io/)Initial power on - When device powers on, MCU will briefly connect to the WiFi to get current time.

2. [![Generic badge](https://img.shields.io/badge/WAKE-EXT0-blue.svg)](https://shields.io/)Wake on rain tip counter to increment counter and go back to sleep.

3. [![Generic badge](https://img.shields.io/badge/WAKE-TIMER-blue.svg)](https://shields.io/)Wake at set time interval to read sensors and send to IOT data collection point.


## Open Items - Software

[![Generic badge](https://img.shields.io/badge/STATUS-OPEN-critical.svg)](https://shields.io/)Rainfall in last 24h is currently last 23h, I'll circle back and fix this when everything else is working.
