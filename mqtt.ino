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
  MQTTPublishInt("RoyalGorge/windSpeed/", (int)environment->windSpeed, true);
  MQTTPublishInt("RoyalGorge/windDirection/", (int)environment->windDirection, true);
  MQTTPublishFloat("RoyalGorge/rainfall/", rainfall.hourlyRainfall[hourPtr] * 0.011, true);
  MQTTPublishFloat("RoyalGorge/rainfall24/", last24() * 0.011, true);


  MQTTPublishFloat("RoyalGorge/batteryVoltage/", environment->batteryVoltage, true);
  MQTTPublishFloat("RoyalGorge/lux/", environment->lux, true);
  MQTTPublishFloat("RoyalGorge/relHum/", environment->humidity, true);
  MQTTPublishFloat("RoyalGorge/pressure/", environment->barometricPressure, true);
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

void MQTTPublishInt(const char topic[], int value, bool retain)
{
  int retryCount = 0;
  int status = 0;
  char buffer[256];
  if(!client.connected()) reconnect();
  client.loop();
  sprintf(buffer, "%i", value);
  MonPrintf("%s: %s\n", topic, buffer);
  while (!status && retryCount < 5)
  {
    status = client.publish(topic, buffer, retain);
    MonPrintf("MQTT status: %i\n", status);
    delay(50);
    retryCount++;
  }
}

void MQTTPublishFloat(const char topic[], float value, bool retain)
{
  int retryCount = 0;
  int status = 0;
  char buffer[256];
  if(!client.connected()) reconnect();
  client.loop();
  sprintf(buffer, "%6.3f", value);
  MonPrintf("%s: %s\n", topic, buffer);
  while (!status && retryCount < 5)
  {
    status = client.publish(topic, buffer, retain);
    MonPrintf("MQTT status: %i\n", status);
    delay(50);
    retryCount++;
  }
}

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
