extern const char* server;
extern const char* api_key;
//=======================================================================
//  sendData: all sensor data in structure is sent to Blynk or Thingspeak
//=======================================================================
void sendData(struct sensorData *environment)
{
  int hourPtr = timeinfo.tm_hour;
  // code block for uploading data to BLYNK App

  if (App == "BLYNK") { // choose application
    //Data assigned to Blynk virtual pins
    //jh choose to send F or C


    Blynk.virtualWrite(V1, environment->humidity );
    Blynk.virtualWrite(V2, environment->barometricPressure );
    Blynk.virtualWrite(V3, environment->UVIndex);
    Blynk.virtualWrite(V4, environment->windSpeed );
    Blynk.virtualWrite(V5, environment->windDirection);
#ifndef METRIC
    Blynk.virtualWrite(V0, environment->temperatureF );
    Blynk.virtualWrite(V6, rainfall.hourlyRainfall[hourPtr] * 0.011);
    Blynk.virtualWrite(V7, last24() * 0.011);
#else
    Blynk.virtualWrite(V0, environment->temperatureC );
    Blynk.virtualWrite(V6, rainfall.hourlyRainfall[hourPtr] * 0.011 * 25.4);
    Blynk.virtualWrite(V7, last24() * 0.011 * 25.4);
#endif

    Blynk.virtualWrite(V8, environment->BMEtemperature);
    Blynk.virtualWrite(V9, environment->batteryVoltage);
    Blynk.virtualWrite(V10, environment->lux);
    Blynk.virtualWrite(V11, bootCount);
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
      postStr += String(bootCount);
      postStr += "&field2=";
      postStr += String(environment->humidity);
      postStr += "&field3=";
      postStr += String(environment->barometricPressure);
      postStr += "&field4=";
      postStr += String(environment->UVIndex);
      postStr += "&field5=";
      postStr += String(environment->windSpeed );
      postStr += "&field6=";
      postStr += String(environment->windDirection);
      postStr += "&field7=";
#ifdef METRIC
      postStr += String(rainfall.hourlyRainfall[hourPtr] * 0.011 * 25.4);
#else
      postStr += String(rainfall.hourlyRainfall[hourPtr] * 0.011);
#endif
      postStr += "&field8=";
#ifdef METRIC
      postStr += String(last24() * 0.011 * 25.4);
#else
      postStr += String(last24() *  0.011);
#endif
      postStr += "&field9=";
      postStr += String(environment->batteryVoltage);
      postStr += "&field10=";
#ifdef METRIC
      postStr += String(environment->temperatureC);
#else
      postStr += String(environment->temperatureF);
#endif
      postStr += " HTTP/1.1\r\nHost: a.c.d\r\nConnection: close\r\n\r\n";
      postStr += "";
      client.print(postStr);
      Serial.println(postStr);
      delay(5000);
    }
  }
}
