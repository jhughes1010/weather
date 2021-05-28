#include "esp_deep_sleep.h"
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <stdarg.h>

//externs
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm timeinfo;
extern DallasTemperature temperatureSensor;
//extern struct sensor;


//data struct for instantaneous sensor values
struct sensorData
{
  float temperatureC;
  float temperatureF;
  float windSpeed;
  float windDirection;
  float barometricPressure;
  float UVIndex;
  float LightIndex;
  float batteryVoltage;
};

//rainfall is stored here for historical data uses RTC
struct historicalData
{
  unsigned int hourlyRainfall[24];
  unsigned int current60MinRainfall[12];
};

//=================== Pin assignment definitions ==========================================

#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define VOLT_PIN     33  //voltage divider for battery monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4

#define LED           2  //Diagnostics using built-in LED

#define SerialMonitor
#define SEC 1E6
const int UpdateIntervalSeconds = 5 * 60;  //Sleep timer (300s)

//========================= Enable Blynk or Thingspeak ===================================

// configuration control constant for use of either Blynk or Thingspeak
const String App = "BLYNK";         //  alternative is line below
//const String App = "Thingspeak"; //  alternative is line above

//RTC storage
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR struct historicalData rainfall;


//globals
//int temperature;


void setup() {
  int UpdateIntervalModified = 0;
  Serial.begin(115200);
  delay(25);
  MonPrintf("\nWeather station - Deep sleep version.\n");
  MonPrintf("print control\n");
  temperatureSensor.begin();

  pinMode(WIND_SPD_PIN, INPUT);     // Wind speed sensor
  pinMode(RAIN_PIN, INPUT);     // Rain sensor
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

void loop()
{
  //no loop code
}



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
      //Rainfall interrupt pin set up

      delay(100); //possible settling time on pin to charge
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);
      Serial.println("Wakeup caused by timer");
      updateWake();
      //digitalWrite(LED, 0);
      //connect to WiFi
      wifi_connect();
      //digitalWrite(LED, 0);
      //init and get the time
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      printTimeNextWake();
      printLocalTime();


      //read sensors
      readBattery(&environment);
      readWindSpeed(&environment);
      readWindDirection(&environment);
      readTemperature(&environment);
      //move rainTicks into hourly containers
      MonPrintf("Current Hour: %i\n\n", timeinfo.tm_hour);
      addTipsToHour(rainTicks);
      clearRainfallHour(timeinfo.tm_hour + 1);
      rainTicks = 0;
      //jh printHourlyArray();
      //send sensor data
      Send_Data(&environment);

      //reset rainTicks to 0 as number has been added to hourly totals
      WiFi.disconnect();
      delay(5000);
      break;
    //case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    //case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default :
      MonPrintf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);

      //connect to WiFi
      //digitalWrite(LED, 0);
      wifi_connect();
      //digitalWrite(LED, 0);
      //init and get the time
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printLocalTime();
      updateWake();

      clearRainfall();
      readBattery(&environment);
      WiFi.disconnect();
      delay(5000);
      break;
  }
}

//#include <stdio.h>


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
