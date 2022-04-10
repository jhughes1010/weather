//=============================================================
//Variables for wifi server setup and api keys for IOT
//Constants for WAKE frequency and UOM for sensors
//=============================================================

//===========================================
//Controls supression of the MonPrintf function to serial
//===========================================
#define SerialMonitor

//===========================================
//WiFi connection
//===========================================
char ssid[] = "ssid"; // WiFi Router ssid
char pass[] = "password"; // WiFi Router password

//===========================================
//Blynk connection
//===========================================
char auth[] = "password";
const char* server = "api.blynk.com";

//===========================================
//Thinkspeak connection
//===========================================
const char* thingspeak_server = "api.thingspeak.com";
const char* api_key = "api_key";

//===========================================
//MQTT broker connection
//===========================================
//const char* mqttServer = "test.mosquitto.org";
const char* mqttServer = "192.168.5.74";
const int mqttPort = 1883;
const char* mqttUser = "username";
const char* mqttPassword = "password";
const char mainTopic[20] = "MainTopic/";


//===========================================
//Metric or Imperial measurements
//===========================================
//#define METRIC

//===========================================
//Use optional NVM for backup
//This is a failsafe for RESET events out of
//system control
//===========================================
//#define USE_EEPROM

//===========================================
//BH1750 Enable
//===========================================
#define BH1750Enable

//===========================================
//Anemometer Calibration
//===========================================
//I see 2 switch pulls to GND per revolation. Not sure what others see
#define WIND_TICKS_PER_REVOLUTION 2

//===========================================
//Set how often to wake and read sensors
//===========================================
//const int UpdateIntervalSeconds = 15 * 60;  //Sleep timer (900s) for my normal operation
const int UpdateIntervalSeconds = 5 * 60;  //Sleep timer (60s) testing

//===========================================
//Battery calibration
//===========================================
//batteryCalFactor = measured battery voltage/ADC reading
#define batteryCalFactor .0011804

//===========================================
//Timezone information
//===========================================
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -7 * 3600;
const int   daylightOffset_sec = 3600;

//========================= Enable Blynk, MQTT or Thingspeak ===================================

// configuration control constant for use of either Blynk or Thingspeak
//const String App = "BLYNK";         //  alternative is line below
//const String App = "Thingspeak"; //  alternative is line above
const String App = "MQTT";         //  alternative is line below
