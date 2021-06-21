/* I have an untracked file called secrets.h with the following code
  /*
  //=============================================================
  //Variables for wifi server setup and api keys for IOT
  //Constants for WAKE frequency and UOM for sensors
  //=============================================================


  //-----------------------
  //WiFi connection
  //-----------------------
  char ssid[] = "your ssid"; // WiFi Router ssid
  char pass[] = "your key"; // WiFi Router password

  //-----------------------
  //Blynk connection
  //-----------------------
  char auth[] = "your key";
  //const char* server = "api.blynk.com";

  //-----------------------
  //Thinkspeak connection
  //-----------------------
  const char* server = "api.thingspeak.com";
  const char* api_key = "your key";

  //-----------------------
  //MQTT broker connection
  //-----------------------
  const char* mqttServer = "test.mosquitto.org";
  const int mqttPort = 1883;
  const char mainTopic[20] = "RoyalGorge/";


  //-----------------------
  //Metric or Imperial measurements
  //-----------------------
  #define METRIC

  //-----------------------
  //Anemometer Calibration
  //-----------------------
  //I see 2 switch pulls to GND per revolation. Not sure what others see
  #define WIND_TICKS_PER_REVOLUTION 2

  //-----------------------
  //Set how often to wake and read sensors
  //-----------------------
  //const int UpdateIntervalSeconds = 15 * 60;  //Sleep timer (900s) for my normal operation
  const int UpdateIntervalSeconds = 1 * 60;  //Sleep timer (60s) testing

  //-----------------------
  //Barometer Calibration
  //-----------------------
  #define BAROMETER_CALIBRATION_IN 6.2
  #define BAROMETER_CALIBRATION_Pa 2000 //need sane number
*/




//=======================================================================
//  wifi_connect: connect to WiFi or explicitly connect to Blynk, if used
//=======================================================================
void wifi_connect()
{
  if (App == "BLYNK")  // for posting datas to Blynk App
  {
    MonPrintf("Connecting to %s\n", App);
    Blynk.begin(auth, ssid, pass);
  }
  else if (App == "Thingspeak")  // for posting datas to Thingspeak website
  {
    MonPrintf("Connecting to WiFi\n");
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }
    MonPrintf("WiFi connected\n");
  }
  else
  {
    WiFi.begin(ssid, pass);
    MonPrintf(" is not a valid application");
  }
}
