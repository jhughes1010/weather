//mqtt data send



WiFiClient espClient;
PubSubClient client(espClient);


//=======================================================================
//  SendDataMQTT: send MQTT data to broker with 'retain' flag set to TRUE
//=======================================================================
void SendDataMQTT (struct sensorData *environment)
{
  char bufferTempF[5];
  char bufferTempC[5];
  char bufferRain[10];
  char bufferRain24[10];


  int hourPtr = timeinfo.tm_hour;
  client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);

  while (!client.connected()) {
    MonPrintf("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(1000);
    }
  }
  MQTTPublish("boot/", (int)bootCount, true);
  MQTTPublish("rssi/", rssi, true);
  MQTTPublish("temperatureF/", (int)environment->temperatureF, true);
  MQTTPublish("temperatureC/", (int)environment->temperatureC, true);
  MQTTPublish("windSpeed/", environment->windSpeed, true);
  MQTTPublish("windDirection/", (int)environment->windDirection, true);
  MQTTPublish("windCardinalDirection/", environment->windCardinalDirection, true);
  MQTTPublish("photoresistor/", (int)environment->photoresistor, true);
#ifndef METRIC
  MQTTPublish("rainfallInterval/", (float) (rainfall.intervalRainfall * 0.011), true);
  MQTTPublish("rainfall/", (float) (rainfall.hourlyRainfall[hourPtr] * 0.011), true);
  MQTTPublish("rainfall24/", (float) (last24() * 0.011), true);
#else
  MQTTPublish("rainfallInterval/", (float) (rainfall.intervalRainfall * 0.011 * 25.4), true);
  MQTTPublish("rainfall/", (float) (rainfall.hourlyRainfall[hourPtr] * 0.011 * 25.4), true);
  MQTTPublish("rainfall24/", (float) (last24() * 0.011 * 25.4), true);
#endif

  MQTTPublish("batteryVoltage/", environment->batteryVoltage, true);
  MQTTPublish("lux/", environment->lux, true);
  MQTTPublish("UVIndex/", environment->UVIndex, true);
  MQTTPublish("relHum/", environment->humidity, true);
  MQTTPublish("pressure/", environment->barometricPressure, true);
  MQTTPublish("caseTemperature/", environment->BMEtemperature, true);
  MQTTPublish("batteryADC/", (int)environment->batteryADC, true);
  MQTTPublish("ESPcoreF/", (int)environment->coreF, true);
  MQTTPublish("ESPcoreC/", (int)environment->coreC, true);
  MQTTPublish("timeEnabled/", (int)elapsedTime, true);
  MQTTPublish("lowBattery/", lowBattery, true);
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

//=======================================================================
//  MQTTPublish String: routine to publish string
//=======================================================================
void MQTTPublish(const char topic[], char *value, bool retain)
{
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%s", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublish Int: routine to publish int values as strings
//=======================================================================
void MQTTPublish(const char topic[], int value, bool retain)
{
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%i", value);
  MQTTSend(topicBuffer, payload, retain);
}


//=======================================================================
//  MQTTPublish Long: routine to publish int values as strings
//=======================================================================
void MQTTPublish(const char topic[], long value, bool retain)
{
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%li", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublish Float: routine to publish float values as strings
//=======================================================================
void MQTTPublish(const char topic[], float value, bool retain)
{
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%6.3f", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublish Bool: routine to publish bool values as strings
//=======================================================================
void MQTTPublish(const char topic[], bool value, bool retain)
{
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  if (value)
  {
    sprintf(payload, "true");
  }
  else
  {
    sprintf(payload, "false");
  }
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  reconnect: MQTT reconnect
//=======================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//=======================================================================
//  MQTTSend: MQTT send topic with value to broker
//=======================================================================
void MQTTSend(char *topicBuffer, char *payload, bool retain)
{
  int status = 0;
  int retryCount = 0;
#ifdef ExtendedMQTT
  MonPrintf("%s: %s\n", topicBuffer, payload);
#endif
  while (!status && retryCount < 5)
  {
    status = client.publish(topicBuffer, payload, retain);
#ifdef ExtendedMQTT
    MonPrintf("MQTT status: %i\n", status);
#endif
    delay(50);
    retryCount++;
  }
}
