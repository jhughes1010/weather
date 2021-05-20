extern const char* server;
extern const char* api_key;

void Send_Data()
{
  int hourPtr = timeinfo.tm_hour;
  // code block for uploading data to BLYNK App

  if (App == "BLYNK") { // choose application
    Blynk.virtualWrite(0, temperature );   // virtual pin 0
    //Blynk.virtualWrite(1, humidity ); // virtual pin 1
    //Blynk.virtualWrite(2, pressure / 100 );  // virtual pin 2
    //Blynk.virtualWrite(3, UVindex);    // virtual pin 3
    // Blynk.virtualWrite(4, windSpeed*1.492 ); // virtual pin 4
    //Blynk.virtualWrite(4, windSpeed * 2.4 * 4.5 ); // virtual pin 4
    //Blynk.virtualWrite(5, windDir);    // virtual pin 5
    Blynk.virtualWrite(V6, hourlyRainfall[hourPtr] * 0.011);
    Blynk.virtualWrite(V7, last24() * 0.011);
    //Blynk.virtualWrite(7, batteryVolt);    // virtual pin 7
    //Blynk.virtualWrite(8, sensors.getTempCByIndex(0));    // virtual pin 8
    delay(1000);
  }

  // code block for uploading data to Thingspeak website

  else if (App == "Thingspeak")
  {
    Serial.println("Attempting to connect to Thingspeak");
    // Send data to ThingSpeak
    WiFiClient client;
    if (client.connect(server, 80))
    {
      Serial.println("Connect to ThingSpeak - OK");
      Serial.println("");
      Serial.println("********************************************");

      int hourPtr = timeinfo.tm_hour;

      String postStr = "";
      postStr += "GET /update?api_key=";
      postStr += api_key;
      postStr += "&field1=";
      postStr += String(hourlyRainfall[hourPtr]);
      /*postStr += "&field2=";
        postStr += String(humidity);
        postStr += "&field3=";
        postStr += String(pressure / 100);
        postStr += "&field4=";
        postStr += String(UVindex);
        postStr += "&field5=";
        //postStr+=String(windSpeed*1.492); //speed in mph
        postStr += String(windSpeed * 2.4 * 4.5); //speed in Km/h
        postStr += "&field6=";
        postStr += String(windDir);
        postStr += "&field7=";
        postStr += String(float(rainTicks) * 0.011, 3);
        postStr += "&field8=";
        postStr += String(batteryVolt);
        postStr += "&field9=";
        postStr += String(sensors.getTempCByIndex(0));*/
      postStr += " HTTP/1.1\r\nHost: a.c.d\r\nConnection: close\r\n\r\n";
      postStr += "";
      client.print(postStr);
      Serial.println(postStr);
      delay(5000);
      //*******************************************************************************
    }
  }
}
