OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);


//Entry point for all sensor data reading
int readSensors( void)
{

}
void readTemperature (struct sensorData *environment)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  temperatureSensor.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  environment->temperatureC = temperatureSensor.getTempCByIndex(0);

  // Check if reading was successful
  if (environment->temperatureC != DEVICE_DISCONNECTED_C)
  {
    environment->temperatureF = environment->temperatureC * 9 / 5 + 32;
    Serial.printf("Temperature for the device 1 (index 0) is: %5.1f C: %5.1f F", environment->temperatureC, environment->temperatureF);
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    environment->temperatureF = -40;
    environment->temperatureC = -40;
  }
}
