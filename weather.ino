// weather station v3.0 code that fully supports deep sleep modes
// code rewrite by James Hughes - KB0HHM
// jhughes1010@gmail.com
//
//Supporting the following project: https://www.instructables.com/Solar-Powered-WiFi-Weather-Station-V30/
//version 1.2 RC1
#define VERSION 1.2

//===========================================
// Includes
//===========================================
#include "esp_deep_sleep.h"
#include "secrets.h"
//#include <WiFi.h>
#include <esp_wifi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Wire.h"
#ifdef BH1750Enable
#include <BH1750.h>
#endif
#include <BME280I2C.h>
#include "Adafruit_SI1145.h"
#include <stdarg.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <esp_task_wdt.h>

//===========================================
// Defines
//===========================================
#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define PR_PIN       15  //photoresistor pin 
#define VOLT_PIN     33  //voltage divider for battery monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4
#define LED_BUILTIN   2  //Diagnostics using built-in LED, may be set to 12 for newer boards that do not use devkit sockets
#define SEC 1E6          //Multiplier for uS based math
#define WDT_TIMEOUT 60

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
  char windCardinalDirection[5];
  float barometricPressure;
  float BMEtemperature;
  float humidity;
  float UVIndex;
  float lux;
  int photoresistor;
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
#ifdef BH1750Enable
BH1750 lightMeter(0x23);
#endif
BME280I2C bme;
Adafruit_SI1145 uv = Adafruit_SI1145();
bool lowBattery = false;
bool WiFiEnable = false;

//===========================================
// ISR Prototypes
//===========================================
void IRAM_ATTR rainTick(void);
void IRAM_ATTR windTick(void);

//===========================================
// setup:
//===========================================
void setup()
{
  long UpdateIntervalModified = 0;

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  //set hardware pins
  pinMode(WIND_SPD_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  delay(25);

  //Title message
  MonPrintf("\nWeather station - Deep sleep version.\n");
  MonPrintf("Version %5.2f\n\n", VERSION);
  BlinkLED(1);
  bootCount++;

  //Initialize i2c and 1w sensors
  Wire.begin();
  bme.begin();
#ifdef BH1750Enable
  lightMeter.begin();
#endif
  temperatureSensor.begin();

  updateWake();
  wakeup_reason();
  if (WiFiEnable)
  {
    MonPrintf("Connecting to WiFi\n");
    //Calibrate Clock - My ESP RTC is noticibly fast
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
    printTimeNextWake();
    processSensorUpdates();
  }

  UpdateIntervalModified = nextUpdate - mktime(&timeinfo);
  if (UpdateIntervalModified <= 0)
  {
    UpdateIntervalModified = 3;
  }
  //pet the dog!
  esp_task_wdt_reset();
  sleepyTime(UpdateIntervalModified);
}

//===================================================
// loop: these are not the droids you are looking for
//===================================================
void loop()
{
  //no loop code
}

//====================================================
// processSensorUpdates: Connect to WiFi, read sensors
// and record sensors at IOT destination and MQTT
//====================================================
void processSensorUpdates(void)
{
  struct sensorData environment;
#ifdef USE_EEPROM
  readEEPROM(&rainfall);
#endif
  wifi_connect();

  //Get Sensor data
  readSensors(&environment);

  //move rainTicks into hourly containers
  MonPrintf("Current Hour: %i\n\n", timeinfo.tm_hour);
  addTipsToHour(rainTicks);
  clearRainfallHour(timeinfo.tm_hour + 1);
  rainTicks = 0;

  //Start sensor housekeeping
  addTipsToHour(rainTicks);
  clearRainfallHour(timeinfo.tm_hour + 1);
  rainTicks = 0;
  //Conditional write of rainfall data to EEPROM
#ifdef USE_EEPROM
  conditionalWriteEEPROM(&rainfall);
#endif
  //send sensor data to IOT destination
  sendData(&environment);

  //send sensor data to MQTT
#ifdef MQTT
  SendDataMQTT(&environment);
#endif

  WiFi.disconnect();
  esp_wifi_stop();
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
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  MonPrintf("Wakeup reason: %d\n", wakeup_reason);
  switch (wakeup_reason)
  {
    //Rain Tip Gauge
    case ESP_SLEEP_WAKEUP_EXT0 :
      MonPrintf("Wakeup caused by external signal using RTC_IO\n");
      WiFiEnable = false;
      rainTicks++;
      break;

    //Timer
    case ESP_SLEEP_WAKEUP_TIMER :
      MonPrintf("Wakeup caused by timer\n");
      WiFiEnable = true;
      //Rainfall interrupt pin set up
      delay(100); //possible settling time on pin to charge
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);
      break;

    //Initial boot or other default reason
    default :
      MonPrintf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      WiFiEnable = true;
      //jh initEEPROM(); //my debug only
      break;
  }
}

//===========================================
// sleepyTime: prepare for sleep and set
// timer and EXT0 WAKE events
//===========================================
void sleepyTime(long UpdateIntervalModified)
{
  Serial.println("\n\n\nGoing to sleep now...");
  Serial.printf("Waking in %i seconds\n\n\n\n\n\n\n\n\n\n", UpdateIntervalModified);
  //updateWake();
  esp_deep_sleep_enable_timer_wakeup(UpdateIntervalModified * SEC);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);
  esp_deep_sleep_start();
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
    delay(150);
    //LED OFF
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
