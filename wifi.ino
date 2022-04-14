//=======================================================================
//  wifi_connect: connect to WiFi or explicitly connect to Blynk, if used
//=======================================================================
long wifi_connect()
{
<<<<<<< HEAD
  long wifi_signal = 0;

  MonPrintf("Connecting to %s\n", App);
=======
  bool WiFiConnectHalt = false;
  int retry = 0;
  long wifi_signal = 0;

  MonPrintf("Starting wifi for App = %s\n", App);
>>>>>>> develop
  if (App == "BLYNK")  // for posting datas to Blynk App
  {

    Blynk.begin(auth, ssid, pass);
  }
  else if ((App == "Thingspeak") || (App == "MQTT")) // for posting datas to Thingspeak website
  {
    MonPrintf("Connecting to WiFi\n");
    WiFi.begin(ssid, pass);
<<<<<<< HEAD
    while (WiFi.status() != WL_CONNECTED)
=======
    while (WiFi.status() != WL_CONNECTED && !WiFiConnectHalt )
>>>>>>> develop
    {
      delay(500);
      retry++;
      if (retry > 15)
      {
        MonPrintf("Max trys to connect to WiFi reached and failed");
        WiFiConnectHalt = true;
        wifi_signal = RSSI_INVALID;
        return wifi_signal;
      }
    }
    MonPrintf("WiFi connected\n");
    wifi_signal = WiFi.RSSI();
  }
  /*
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
  */
  else
  {
    MonPrintf(" is not a valid application");
  }
  return wifi_signal;
}
