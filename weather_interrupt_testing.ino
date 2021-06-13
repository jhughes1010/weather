// weather station v3.0 code that fully supports deep sleep modes
// code rewrite by James Hughes - KB0HHM
// jhughes1010@gmail.com
//
//

//===========================================
// Includes
//===========================================
#include "esp_deep_sleep.h"
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Wire.h"
#include <BH1750.h>
#include <BME280I2C.h>
#include "Adafruit_SI1145.h"
#include <stdarg.h>
#include <PubSubClient.h>

//===========================================
// Defines
//===========================================

#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define VOLT_PIN     33  //voltage divider for battery monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4

#define LED           2  //Diagnostics using built-in LED

#define SerialMonitor
#define SEC 1E6
//===========================================
// Externs
//===========================================
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm timeinfo;
extern DallasTemperature temperatureSensor;
//extern struct sensor;


//===========================================
// Custom structures
//===========================================
struct sensorData
{
  float temperatureC;
  float temperatureF;
  float windSpeed;
  float windDirection;
  float barometricPressure;
  float BMEtemperature;
  float humidity;
  float UVIndex;
  float lux;
  float batteryVoltage;
};

//rainfall is stored here for historical data uses RTC
struct historicalData
{
  unsigned int hourlyRainfall[24];
  unsigned int current60MinRainfall[12];
};


//===========================================
// RTC Storage
//===========================================
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR struct historicalData rainfall;

//===========================================
// Defines
//===========================================
BH1750 lightMeter(0x23);
BME280I2C bme;
Adafruit_SI1145 uv = Adafruit_SI1145();

//===========================================
// setup: 
//===========================================
void setup() {

  int UpdateIntervalModified = 0;
  Serial.begin(115200);
  delay(25);
  MonPrintf("\nWeather station - Deep sleep version.\n");
  MonPrintf("print control\n");
  Wire.begin();
  bme.begin();
  lightMeter.begin();
  temperatureSensor.begin();

  pinMode(WIND_SPD_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);

  wakeup_reason();

  // ESP32 Deep Sleep Mode
  MonPrintf("Going to sleep now...\n\n\n\n\n");
  UpdateIntervalModified = nextUpdate - mktime(&timeinfo);
  if (UpdateIntervalModified <= 0)
  {
    UpdateIntervalModified = 3;
  }

  esp_deep_sleep_enable_timer_wakeup(UpdateIntervalModified * SEC);
  MonPrintf("Waking in %i seconds\n", UpdateIntervalModified);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);
  esp_deep_sleep_start();
}

//===================================================
// loop: these are not the droids you are looking for
//===================================================
void loop()
{
  //no loop code
}


//===========================================================
// wakeup_reason: action based on WAKE reason
// 1. Power up
// 2. WAKE on EXT0 - increment rain tip gauge count and sleep
// 3. WAKE on TIMER - send sensor data to IOT target
//===========================================================
//check for WAKE reason and respond accordingly
void wakeup_reason()
{
  struct sensorData environment;
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      MonPrintf("Wakeup caused by external signal using RTC_IO");
      rainTicks++;
      printTimeNextWake();
      printLocalTime();
      break;
    //case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER :
      if (! uv.begin()) {
        Serial.println("Didn't find Si1145");
        while (1);
      }
      if (! bme.begin()) {
        Serial.println("Didn't find BME280");
        while (1);
      }

      //Rainfall interrupt pin set up
      delay(100); //possible settling time on pin to charge
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);
      Serial.println("Wakeup caused by timer");
      updateWake();
      wifi_connect();
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printTimeNextWake();
      printLocalTime();

      //read sensors
      readSensors(&environment);

      //move rainTicks into hourly containers
      MonPrintf("Current Hour: %i\n\n", timeinfo.tm_hour);
      addTipsToHour(rainTicks);
      clearRainfallHour(timeinfo.tm_hour + 1);
      rainTicks = 0;

      //send sensor data to IOT destination
      sendData(&environment);
      SendDataMQTT(&environment);
      WiFi.disconnect();
      delay(5000);
      break;
    //case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    //case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default :
      MonPrintf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      wifi_connect();
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printLocalTime();
      updateWake();
      clearRainfall();
      WiFi.disconnect();
      delay(5000);
      break;
  }
}

//===========================================
// MonPrintf: diagnostic printf to terminal
//===========================================
void MonPrintf( const char* format, ... ) {
  char buffer[200];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end( args );
#ifdef SerialMonitor
  Serial.printf("%s", buffer);
#endif
}
