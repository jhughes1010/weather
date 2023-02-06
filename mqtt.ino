//mqtt data send
WiFiClient espClient;
PubSubClient client(espClient);

struct config {
  char name[25];
  char device_class[25];
  char unit_of_measurement[20];
};

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
  if (discovery_enable){
    struct config sensor[22]={
      {"boot", "","#"},
      {"rssi", "","#"},
      {"temperatureF", "temperature","°F"},
      {"temperatureC", "temperature","°C"},
      {"windSpeed", "wind_speed",""}, 
      {"windDirection", "",""},
      {"windCardinalDirection", "",""},
      {"photoresistor", "",""},
      {"rainfallInterval", "",""},
      {"rainfall", "volume","L"},
      {"rainfall24", "volume","L"},
      {"batteryVoltage", "voltage","V"},
      {"lux", "illuminance","lx"},
      {"UVIndex", "",""},
      {"relHum", "","%"},
      {"pressure", "",""},
      {"caseTemperature", "temperature","°C"},
      {"batteryADC", "",""},
      {"ESPcoreF", "temperature","°F"},
      {"ESPcoreC", "temperature","°C"},
      {"timeEnabled", "",""},
      {"lowBattery", "",""},
    };
    for (int i=0;i<sizeof sensor/sizeof sensor[0];i++){
      configMQTT_HA(sensor[i]);
    };
  }
  MQTTPublish("boot/state", (int)bootCount, true);
  MQTTPublish("rssi/state", rssi, true);
  MQTTPublish("temperatureF/state", (int)environment->temperatureF, true);
  MQTTPublish("temperatureC/state", (int)environment->temperatureC, true);
  MQTTPublish("windSpeed/state", environment->windSpeed, true);
  MQTTPublish("windDirection/state", (int)environment->windDirection, true);
  MQTTPublish("windCardinalDirection/state", environment->windCardinalDirection, true);
  MQTTPublish("photoresistor/state", (int)environment->photoresistor, true);
#ifndef METRIC
  MQTTPublish("rainfallInterval/state", (float) (rainfall.intervalRainfall * 0.011), true);
  MQTTPublish("rainfall/state", (float) (rainfall.hourlyRainfall[hourPtr] * 0.011), true);
  MQTTPublish("rainfall24/state", (float) (last24() * 0.011), true);
#else
  MQTTPublish("rainfallInterval/state", (float) (rainfall.intervalRainfall * 0.011 * 25.4), true);
  MQTTPublish("rainfall/state", (float) (rainfall.hourlyRainfall[hourPtr] * 0.011 * 25.4), true);
  MQTTPublish("rainfall24/state", (float) (last24() * 0.011 * 25.4), true);
#endif

  MQTTPublish("batteryVoltage/state", (float) environment->batteryVoltage, true);
  MQTTPublish("lux/state", environment->lux, true);
  MQTTPublish("UVIndex/state", environment->UVIndex, true);
  MQTTPublish("relHum/state", environment->humidity, true);
  MQTTPublish("pressure/state", environment->barometricPressure, true);
  MQTTPublish("caseTemperature/state", environment->BMEtemperature, true);
  MQTTPublish("batteryADC/state", (int)environment->batteryADC, true);
  MQTTPublish("ESPcoreF/state", (int)environment->coreF, true);
  MQTTPublish("ESPcoreC/state", (int)environment->coreC, true);
  MQTTPublish("timeEnabled/state", (int)elapsedTime, true);
  MQTTPublish("lowBattery/state", lowBattery, true);
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

void configMQTT_HA( struct config sensor)
{
  // homeassistant/sensor/weatherA/temperatureC
  // homeassistant/sensor/weather/temperatureC
  // {"device_class": "temperature", "name": "temperatureC", "state_topic": "weather/temperatureC", "unit_of_measurement": "°C" }
  Serial.println("configMQTT_HA");
  char orgmainTopic[20];
  char confJson[300];
  char topic[60];
  memcpy( orgmainTopic, mainTopic, sizeof(orgmainTopic));
  memcpy( mainTopic, "homeassistant/", sizeof(mainTopic));
  char const *v_part1="{\"device_class\": \"";
  

  char const *v_part5="temperatureC\", \"unit_of_measurement\": \"°C\" }";
  
  
  strcpy(confJson, "{");
  if(strlen(sensor.device_class) != 0) {
    strcat(confJson, "\"device_class\": \"");
    strcat(confJson, sensor.device_class );
    strcat(confJson, "\",");
  }
  strcat(confJson, " \"name\": \"" );
  strcat(confJson, sensor.name );
  strcat(confJson, "\", \"state_topic\": \"" );
  strcat(confJson, orgmainTopic);
  strcat(confJson, sensor.name);
  strcat(confJson, "/state\"");
  //strcat(confJson, "");
  if(strlen(sensor.unit_of_measurement) != 0) {
    strcat(confJson, ", \"unit_of_measurement\": \"");
    strcat(confJson, sensor.unit_of_measurement);
    strcat(confJson, "\"");
  } 
  strcat(confJson, "}");


  strcpy(topic, "sensor/");
  strcat(topic, orgmainTopic);
  strcat(topic, sensor.name);
  strcat(topic, "/config");
  Serial.println(topic);
  Serial.println(confJson);
  MQTTPublish(topic,confJson, true);

  memcpy( mainTopic, orgmainTopic, sizeof(mainTopic));

}

//=======================================================================
//  MQTTPublishString: routine to publish string
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
//  MQTTPublishInt: routine to publish int values as strings
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
//  MQTTPublishFloat: routine to publish float values as strings
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
//  MQTTPublishBool: routine to publish bool values as strings
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
