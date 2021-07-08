//=============================================================
//Variables for wifi server setup and api keys for IOT
//Constants for WAKE frequency and UOM for sensors
//=============================================================

//NOTE: Rename this file to secrets.h or change #include "secrets.h" to "sec.h"


//===========================================
//WiFi connection
//===========================================
char ssid[] = "mySSID"; // WiFi Router ssid
char pass[] = "myPassword"; // WiFi Router password

//===========================================
//Blynk connection
//===========================================
char auth[] = "Blynk_API_key";
//const char* server = "api.blynk.com";

//===========================================
//Thinkspeak connection
//===========================================
const char* server = "api.thingspeak.com";
const char* api_key = "Thingspeak_API_key";

//===========================================
//MQTT broker connection
//===========================================
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char mainTopic[20] = "mainTopicName/";
const char* mqttUser = "username";
const char* mqttPassword = "password";


//===========================================
//Metric or Imperial measurements
//===========================================
#define METRIC

//===========================================
//Anemometer Calibration
//===========================================
//I see 2 switch pulls to GND per revolation. Not sure what others see
#define WIND_TICKS_PER_REVOLUTION 2

//===========================================
//Set how often to wake and read sensors
//===========================================
const int UpdateIntervalSeconds = 5 * 60;  //Sleep timer (900s) for my normal operation
//const int UpdateIntervalSeconds = 1 * 60;  //Sleep timer (60s) testing

//===========================================
//Barometer Calibration
//===========================================
#define BAROMETER_CALIBRATION_IN 6.2
#define BAROMETER_CALIBRATION_Pa 2000 //need sane number

//========================= Enable Blynk or Thingspeak ===================================

// configuration control constant for use of either Blynk or Thingspeak
const String App = "BLYNK";         //  alternative is line below
//const String App = "Thingspeak"; //  alternative is line above
