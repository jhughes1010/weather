#include "esp_deep_sleep.h"
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>

//externs
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm timeinfo;

#define RAIN_PIN     25

#define US 1E6
const int UpdateInterval = 1 * 15 * US;  // e.g. 0.33 * 60 * 1000000; // Sleep time

//========================= Enable Blynk or Thingspeak ===================================

// configuration control constant for use of either Blynk or Thingspeak
const String App = "BLYNK";         //  alternative is line below
//const String App = "Thingspeak"; //  alternative is line above

//RTC storage
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR unsigned char hourlyRainfall[24];


void setup() {
  int UpdateIntervalModified = 0;
  Serial.begin(115200);
  delay(25);
  Serial.println("\nWeather station - test bed to understand RAIN_PIN.\n");

  //set hardware pins
  pinMode(RAIN_PIN, INPUT);     // Rain sensor


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
    esp_deep_sleep_enable_timer_wakeup(300 * US);
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
      //move rainTicks into hourly containers
      Serial.printf("Current Hour: %i\n\n", timeinfo.tm_hour);
      addTipsToHour(rainTicks);
      clearRainfallHour(timeinfo.tm_hour + 1);
      rainTicks = 0;
      printHourlyArray();
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

void rainTick(void)
{
  rainTicks++;
}
