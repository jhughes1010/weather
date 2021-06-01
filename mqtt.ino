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

  MQTTPublishInt("RoyalGorge/temperatureF/", (int)environment->temperatureF, true);
  MQTTPublishInt("RoyalGorge/temperatureC/", (int)environment->temperatureC, true);
  MQTTPublishFloat("RoyalGorge/rainfall/", rainfall.hourlyRainfall[hourPtr] * 0.011, true);
  MQTTPublishFloat("RoyalGorge/rainfall24/", last24() * 0.011, true);
  MQTTPublishInt("RoyalGorge/windSpeed/", (int)environment->windSpeed, true);
  MQTTPublishInt("RoyalGorge/windDirection/", (int)environment->windDirection, true);

  MQTTPublishFloat("RoyalGorge/batteryVoltage/", environment->batteryVoltage, true);
  MQTTPublishFloat("RoyalGorge/lux/", environment->lux, true);
  delay(5000);
}

void MQTTPublishInt(const char topic[], int value, bool retain)
{
  char buffer[256];

  sprintf(buffer, "%i", value);
  client.publish(topic, buffer, retain);
  delay(1000);
}

void MQTTPublishFloat(const char topic[], float value, bool retain)
{
  char buffer[256];

  sprintf(buffer, "%06.3f", value);
  client.publish(topic, buffer, retain);
  delay(1000);
}
