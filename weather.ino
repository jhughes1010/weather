// weather station v3.0 code that fully supports deep sleep modes
// code rewrite by James Hughes - KB0HHM
// jhughes1010@gmail.com
//
//Supporting the following project: https://www.instructables.com/Solar-Powered-WiFi-Weather-Station-V30/

#define VERSION "1.3.2"

//=============================================
// Changelog
//=============================================
/*
 *  v1.3.2
 *      1. I2C OLED diagnostics added (if needed)
 *      2. 
    v1.3.1
        1. Corrects missing quotes on #define VERSION statement
        2. max retry if 15 connect attempts added and then we bail on WiFi connect. This prevents us from hitting the WDT limit and rebooting


    v1.3 supports 24h rainfall data, not 23h
        supports current 60 min rainfall, not
        current "hour" that looses data at top
        of the hour.

        lowBattery flag fix and multiplies wake time by 10
        when battery is < 15 % remaining

        Alternate pinout for Thomas Krebs PCB
        design that does not use devkit ESP32

        remove esp_deep_sleep.h as it is not needed

        modified the #include to clearly discern system includes <file.h>

        uv.begin added to sensorEnable()

        renamed historicalData to rainfallData and added rainfallInterval as an additional mqtt topic for non historical accumulation

        addred rssi topic on mqtt publish listing

        clearer reporting to console on sensor.begin statuses. Program should run with no sensors now and not hang








*/

//===========================================
// Includes
//===========================================
#include "secrets.h"
#include <esp_wifi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#ifdef BH1750Enable
#include <BH1750.h>
#endif
#include <BME280I2C.h>
#include <Adafruit_SI1145.h>
#include <stdarg.h>
#include <PubSubClient.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <driver/rtc_io.h>
//OLED diagnostics board
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//===========================================
// Defines
//===========================================
//If you are using a Thomas Krebs designed PCB that does not use a standard devkit, place a #define KREBS_PCB in your secrets.h
#ifndef KREBS_PCB
#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define PR_PIN       15  //photoresistor pin 
#define VOLT_PIN     33  //voltage divider for battery monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4
#define LED_BUILTIN   2  //Diagnostics using built-in LED, may be set to 12 for newer boards that do not use devkit sockets
#define SEC 1E6          //Multiplier for uS based math
#define WDT_TIMEOUT 60   //watchdog timer

#else
#define WIND_SPD_PIN        26  //reed switch based anemometer count
#define RAIN_PIN            25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN        35  //variable voltage divider output based on varying R network with reed switches
#define PR_PIN              34  //photoresistor pin
#define VOLT_PIN            33  //voltage divider for battery monitor
#define TEMP_PIN            15  // DS18B20 hooked up to GPIO pin 15
#define LED_PIN             14  //Diagnostics using built-in LED
//#define MODE_PIN          12  //Load Switch
#endif

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
  unsigned int coreF;
  unsigned int coreC;
};

//rainfall is stored here for historical data uses RTC
struct rainfallData
{
  unsigned int intervalRainfall;
  unsigned int hourlyRainfall[24];
  unsigned int current60MinRainfall[12];
  unsigned int hourlyCarryover;
  unsigned int priorHour;
  unsigned int minuteCarryover;
  unsigned int priorMinute;
};

struct sensorStatus
{
  int uv;
  int bme;
  int lightMeter;
  int temperature;
};


//===========================================
// RTC Memory storage
//===========================================
RTC_DATA_ATTR volatile int rainTicks = 0;
RTC_DATA_ATTR int lastHour = 0;
RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR struct rainfallData rainfall;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned int elapsedTime = 0;

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
struct sensorStatus status;
long rssi = 0;

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

  setCpuFrequencyMhz (80);
  rtc_gpio_init(GPIO_NUM_12);
  rtc_gpio_set_direction(GPIO_NUM_12, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_set_level(GPIO_NUM_12, 1);

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
  MonPrintf("Version %s\n\n", VERSION);
  BlinkLED(1);
  bootCount++;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  printLocalTimeLCD();
  printADCLCD();
  display.printf("SSID: %s\n", ssid);
  display.print("BOOT: ");
  display.println(bootCount);
  display.display();

  updateWake();
  wakeup_reason();
  if (WiFiEnable)
  {
    rssi = wifi_connect();
    if (rssi != RSSI_INVALID)
    {
      sensorEnable();
      sensorStatusToConsole();
      //Calibrate Clock - My ESP RTC is noticibly fast
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      printLocalTime();
      printTimeNextWake();
      processSensorUpdates();
      WiFi.disconnect();
      esp_wifi_stop();
    }
  }

  UpdateIntervalModified = nextUpdate - mktime(&timeinfo);
  if (UpdateIntervalModified < 3)
  {
    UpdateIntervalModified = 3;
  }
  //pet the dog!
  esp_task_wdt_reset();
  BlinkLED(2);
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
  //Get Sensor data
  readSensors(&environment);

  //move rainTicks into interval container
  rainfall.intervalRainfall = rainTicks;

  //move rainTicks into hourly containers
  MonPrintf("Current Hour: %i\n\n", timeinfo.tm_hour);

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
  if (App == "MQTT")
  {
    SendDataMQTT(&environment);
  }
  display.printf("Temp: %4.1f F\n", environment.temperatureF);
  display.printf("Pressure: %4.1f inHg\n", environment.barometricPressure);
  display.display();
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
      //I manually call this line to zero out EEPROM array once and only once, then remove this line
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

  rtc_gpio_set_level(GPIO_NUM_12, 0);
  esp_sleep_enable_timer_wakeup(UpdateIntervalModified * SEC);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);
  elapsedTime = (int)millis() / 1000;
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
    delay(350);
  }
}

//===========================================
// sensorEnable: Initialize i2c and 1w sensors
//===========================================
void sensorEnable(void)
{
  status.temperature = Wire.begin();
  status.bme = bme.begin();
  status.uv = uv.begin();
#ifdef BH1750Enable
  status.lightMeter = lightMeter.begin();
#endif
  temperatureSensor.begin();                //returns void - cannot directly check
}

//===========================================
// sensorStatusToConsole: Output .begin return values
//===========================================
void sensorStatusToConsole(void)
{
  MonPrintf("----- Sensor Statuses -----\n");
  MonPrintf("BME status:         %i\n", status.bme);
  MonPrintf("UV status:          %i\n", status.uv);
  MonPrintf("lightMeter status:  %i\n", status.lightMeter);
  MonPrintf("temperature status: %i\n\n", status.temperature);
}
