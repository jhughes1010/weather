//=======================================================================
//  wifi_connect: connect to WiFi or explicitly connect to Blynk, if used
//=======================================================================
long wifi_connect()
{
  long wifi_signal = 0;

  MonPrintf("Connecting to %s\n", App);
  if (App == "BLYNK")  // for posting datas to Blynk App
  {

    Blynk.begin(auth, ssid, pass);
  }
  else if ((App == "Thingspeak") || (App == "MQTT")) // for posting datas to Thingspeak website
  {
    MonPrintf("Connecting to WiFi\n");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }
    MonPrintf("WiFi connected\n");
    wifi_signal = WiFi.RSSI();
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
    MonPrintf(" is not a valid application");
  }
  return wifi_signal;
}
