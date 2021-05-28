OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);


//Entry point for all sensor data reading
int readSensors( void)
{

}
void readTemperature (struct sensorData *environment)
{
  MonPrintf("Requesting temperatures...");
  temperatureSensor.requestTemperatures();
  MonPrintf("DONE");
  environment->temperatureC = temperatureSensor.getTempCByIndex(0);

  // Check if reading was successful
  if (environment->temperatureC != DEVICE_DISCONNECTED_C)
  {
    environment->temperatureF = environment->temperatureC * 9 / 5 + 32;
    MonPrintf("Temperature for the device 1 (index 0) is: %5.1f C: %5.1f F\n", environment->temperatureC, environment->temperatureF);
  }
  else
  {
    MonPrintf("Error: Could not read temperature data");
    environment->temperatureF = -40;
    environment->temperatureC = -40;
  }
}

void readBattery (struct sensorData *environment)
{
  int val;
  float Vout;
  const int R1 = 27000;
  const int R2 = 100000;
  // Reading Battery Level in %
  val = analogRead(VOLT_PIN);
  Vout = (val * 3.3 ) / 4095.0; // formula for calculating voltage out
  environment->batteryVoltage = (float)Vout * ( R2 + R1) / R2 ; // formula for calculating voltage in
  MonPrintf("Battery digital :%i percentage: %6.2f\n", val, environment->batteryVoltage);
}
