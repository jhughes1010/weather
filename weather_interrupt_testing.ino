#include "esp_deep_sleep.h"
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//externs
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm timeinfo;
extern DallasTemperature sensors;

//=================== Pin assignment definitions ==========================================

#define WIND_SPD_PIN 14
#define RAIN_PIN     25
#define WIND_DIR_PIN 35
#define VOLT_PIN     33
#define TEMP_PIN 4  // DS18B20 hooked up to GPIO pin 4

#define US 1E6
const int UpdateInterval = 5 * 60;  // e.g. 0.33 * 60 * 1000000; // Sleep time

//========================= Enable Blynk or Thingspeak ===================================

// configuration control constant for use of either Blynk or Thingspeak
const String App = "BLYNK";         //  alternative is line below
//const String App = "Thingspeak"; //  alternative is line above

//RTC storage
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR unsigned char hourlyRainfall[24];


//globals
int temperature;


void setup() {
  int UpdateIntervalModified = 0;
  Serial.begin(115200);
  delay(25);
  Serial.println("\nWeather station - test bed to understand RAIN_PIN.\n");
  sensors.begin();

  //Rainfall interrupt pin set up
  pinMode(RAIN_PIN, INPUT);     // Rain sensor
  delay(100); //possible settling time on pin to charge
  //jh attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);


  // Wind speed sensor setup. The windspeed is calculated according to the number
  //  of ticks per second. Timestamps are captured in the interrupt, and then converted
  //  into mph.
  pinMode(WIND_SPD_PIN, INPUT_PULLUP);     // Wind speed sensor
  attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, FALLING);


  wakeup_reason();



  //Prove RTC based variable is doing its job
  Serial.print("RainTicks: ");
  Serial.println(rainTicks);

  // ESP32 Deep Sleep Mode
  Serial.println("Going to sleep now...\n\n\n\n\n");
  UpdateIntervalModified = nextUpdate - mktime(&timeinfo);
  Serial.printf("Waking in %i seconds\n", UpdateIntervalModified);
  if (UpdateIntervalModified > 0)
  {
    esp_deep_sleep_enable_timer_wakeup(UpdateIntervalModified * US );
  }
  else
  {
    esp_deep_sleep_enable_timer_wakeup(3 * US);
  }
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
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      Serial.println("Wakeup caused by external signal using RTC_IO");
      rainTicks++;
      //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printTimeNextWake();
      printLocalTime();

      break;
    //case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER :
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      Serial.println("Wakeup caused by timer");
      updateWake();

      //connect to WiFi
      wifi_connect();

      //init and get the time
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      printTimeNextWake();
      printLocalTime();


      //read sensors
      readWindVelocity();
      //readWindDirection();
      temperature = readTemperature();
      //move rainTicks into hourly containers
      Serial.printf("Current Hour: %i\n\n", timeinfo.tm_hour);
      addTipsToHour(rainTicks);
      clearRainfallHour(timeinfo.tm_hour + 1);
      rainTicks = 0;
      //jh printHourlyArray();
      //send sensor data
      Send_Data();

      //reset rainTicks to 0 as number has been added to hourly totals

      break;
    //case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    //case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default :
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);

      //connect to WiFi
      wifi_connect();

      //init and get the time
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printLocalTime();
      updateWake();

      clearRainfall();
      break;
  }
}
