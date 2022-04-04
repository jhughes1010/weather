//=======================================================================
//  wifi_connect: connect to WiFi or explicitly connect to Blynk, if used
//=======================================================================
void wifi_connect()
{

  //delay(5000);
  if (App == "BLYNK")  // for posting datas to Blynk App
  {
    MonPrintf("Connecting to %s\n", App);
    Blynk.begin(auth, ssid, pass);
  }
  else if (App == "Thingspeak")  // for posting datas to Thingspeak website
  {
    MonPrintf("Connecting to WiFi\n");
    WiFi.begin(ssid, pass);
    //WiFi.persistent(false);
    //WiFi.setAutoConnect(false);
    //WiFi.setAutoReconnect(true);
    //WiFi.setTxPower(WIFI_POWER_2dBm);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }
    MonPrintf("WiFi connected\n");
  }
  else if (App == "MQTT")  // for posting datas to Thingspeak website
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
    //WiFi.begin(ssid, pass);
    MonPrintf(" is not a valid application");
  }
}
