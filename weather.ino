// weather station v3.0 code that fully supports deep sleep modes
// code rewrite by James Hughes - KB0HHM
// jhughes1010@gmail.com
//
//Supporting the following project: https://www.instructables.com/Solar-Powered-WiFi-Weather-Station-V30/
#define VERSION 1.1

//===========================================
// Includes
//===========================================
#include "esp_deep_sleep.h"
//#include <rom/rtc.h>
//#include "esp32.h"
#include "secrets.h"
//#include "sec.h"  //alternate file for other github users
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
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

//===========================================
// Defines
//===========================================

#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define VOLT_PIN     33  //voltage divider for battery monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4
#define LED_BUILTIN   2  //Diagnostics using built-in LED
#define SEC 1E6          //Multiplier for uS based math
//===========================================
// Externs
//===========================================
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm timeinfo;
extern DallasTemperature temperatureSensor;



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
  int batteryADC;
};

//rainfall is stored here for historical data uses RTC
struct historicalData
{
  unsigned int hourlyRainfall[24];
  unsigned int current60MinRainfall[12];
};


//===========================================
// RTC Memory storage
//===========================================
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR struct historicalData rainfall;
RTC_DATA_ATTR int bootCount = 0;

//===========================================
// Global instantiation
//===========================================
BH1750 lightMeter(0x23);
BME280I2C bme;
Adafruit_SI1145 uv = Adafruit_SI1145();
bool lowBattery = false;
long tickTime[10] = {0};

//===========================================
// setup:
//===========================================
void setup()
{

  int UpdateIntervalModified = 0;
  //DisableBrownOutDetector();
  Serial.begin(115200);
  delay(25);
  MonPrintf("\nWeather station - Deep sleep version.\n");
  MonPrintf("Version %f\n\n",VERSION);
  MonPrintf("print control\n");
  Wire.begin();
  bme.begin();
  lightMeter.begin();
  //temperatureSensor.begin();

  pinMode(WIND_SPD_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  for(int x=0;x<10;x++)
  {
    tickTime[x]=0;
  }
  BlinkLED(1);
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
  MonPrintf("Wakeup reason: %d\n", wakeup_reason);
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      MonPrintf("Wakeup caused by external signal using RTC_IO\n");
      rainTicks++;
      printTimeNextWake();
      printLocalTime();
      break;
    //case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER :
      bootCount++;
      //Rainfall interrupt pin set up
      delay(100); //possible settling time on pin to charge
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      //jh debug attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);
      MonPrintf("Wakeup caused by timer\n");
      
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
#ifdef MQTT
      SendDataMQTT(&environment);
#endif
      WiFi.disconnect();
      updateWake();
      //delay(5000);
      break;
    //case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    //case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;

    
    default :
      MonPrintf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      bootCount++;
      wifi_connect();

      //Next 2 sections are a hack because system always reboots
      //read sensors
      readSensors(&environment);

      //send sensor data to IOT destination
      sendData(&environment);
      SendDataMQTT(&environment);

      //BlinkLED(3);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printLocalTime();
      updateWake();
      clearRainfall();
      WiFi.disconnect();
      //delay(5000);
      break;
  }
  //BlinkLED(wakeup_reason);
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

//===========================================
// DisableBrownOutDetector: RESET Brownout detection bit
//===========================================
void DisableBrownOutDetector(void)
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
}

//===========================================
// EnableBrownOutDetector: RESET Brownout detection bit
//===========================================
void EnableBrownOutDetector(void)
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
}

//===========================================
// BlinkLED: Blink BUILTIN x times
//===========================================
void BlinkLED(int count)
{
  int x;
  //if reason code =0, then set count =1 (just so I can see something)
  if (!count)
  {
    count = 1;
  }
  for (x = 0; x < count; x++)
  {
    //LED ON
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    //LED OFF
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
