//mqtt data send

const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


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
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  MQTTPublishInt("RoyalGorge/temperatureF/", (int)environment->temperatureF, false);
  MQTTPublishInt("RoyalGorge/temperatureC/", (int)environment->temperatureC, false);
  MQTTPublishFloat("RoyalGorge/rainfall/", rainfall.hourlyRainfall[hourPtr] * 0.011, false);
  MQTTPublishFloat("RoyalGorge/rainfall24/", last24() * 0.011, false);
  MQTTPublishInt("RoyalGorge/windSpeed/", (int)environment->windSpeed, false);
  MQTTPublishInt("RoyalGorge/windDirection/", (int)environment->windDirection, false);

  MQTTPublishFloat("RoyalGorge/batteryVoltage/", environment->batteryVoltage, false);
  MQTTPublishFloat("RoyalGorge/lux/", environment->lux, false);
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

void MQTTPublishInt(const char topic[], int value, bool retain)
{
  int status = 0;
  char buffer[256];

  sprintf(buffer, "%i", value);
  MonPrintf("%s: %s\n", topic, buffer);
  status = client.publish(topic, buffer, retain);
  MonPrintf("MQTT status: %i\n", status);
  delay(1000);
}

void MQTTPublishFloat(const char topic[], float value, bool retain)
{
  int status = 0;
  char buffer[256];

  sprintf(buffer, "%06.3f", value);
  MonPrintf("%s: %s\n", topic, buffer);
  status = client.publish(topic, buffer, retain);
  MonPrintf("MQTT status: %i\n", status);
  delay(1000);
}
