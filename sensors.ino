OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

//data struct for sensors
struct sensor
{
  int temperatureC;
  int temperatureF;
  int windSpeed;
  char* windDirection;
  float barometricPressure;
}

//rainfall is stored here for historical data
struct history
{
  unsigned char hourlyRainfall[24];
  unsigned char currentHourRainfall[12];
}
//Entry point for all sensor data reading
int readSensors(struct sensorData)
{

}
int readTemperature (void)
{
  float tempF;
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);

  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    tempF = (float)tempC * 9 / 5 + 32;
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    tempF = -40;
  }
  return (int)tempF;
}
