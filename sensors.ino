OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);


//Entry point for all sensor data reading
void readSensors(struct sensorData *environment)
{
  readWindSpeed(environment);
  readWindDirection(environment);
  readTemperature(environment);
  readLux(environment);
  readBME(environment);
  readBattery(environment);
}
void readTemperature (struct sensorData *environment)
{
  MonPrintf("Requesting temperatures...\n");
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
  //float Vbat;
  const int R1 = 33000*1.05;
  const int R2 = 100000*.95;
  // Reading Battery Level in %
  val = analogRead(VOLT_PIN);
  //Vout = (val * 3.3 ) / 4095.0; // formula for calculating voltage out
  //environment->batteryVoltage = (float)Vout * ( R2 + R1) / R2 ; // formula for calculating voltage in
  environment->batteryVoltage = val*.001009;
  MonPrintf("Battery digital :%i voltage: %6.2f\n", val, environment->batteryVoltage);
}

void readLux(struct sensorData *environment)
{
  environment->lux = lightMeter.readLightLevel();
  MonPrintf("LUX value: %6.2f\n", environment->lux);
}

void readBME(struct sensorData *environment)
{
  bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Celsius, BME280::PresUnit_Pa);
  //bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Fahrenheit, BME280::PresUnit_inHg);
  MonPrintf("BME barometric pressure: %6.2f  BME temperature: %6.2f  BME humidity: %6.2f\n", environment->barometricPressure, environment->BMEtemperature, environment->humidity);
}
