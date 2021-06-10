/* I have an untracked file called secrets.h with the following code
/*
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "yourSSID"; // WiFi Router ssid
char pass[] = "password"; // WiFi Router password

// copy it from the mail received from Blynk
char auth[] = "APIKey";
//const char* server = "api.thingspeak.com";

// Thingspeak Write API
const char* server = "api.thingspeak.com";
const char* api_key = "APIKey"; // API write key

//MQTT server connect details
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char mainTopic[20] = "RoyalGorge/";  //change to your desired MQTT main topic
*/
//============================ Connect to WiFi Network =========================================

void wifi_connect()
{
  if (App == "BLYNK")  // for posting datas to Blynk App
  {
    MonPrintf("Connecting to %s\n", App);
    //digitalWrite(LED, 0);
    Blynk.begin(auth, ssid, pass);
  }
  else if (App == "Thingspeak")  // for posting datas to Thingspeak website
  {
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }
    MonPrintf("WiFi connected\n");
  }
  else
  {
    WiFi.begin(ssid, pass);
    //Serial.print(App);
    MonPrintf(" is not a valid application");
  }
}
