OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

//=======================================================
//  readSensors: Read all sensors and battery voltage
//=======================================================
//Entry point for all sensor data reading
void readSensors(struct sensorData *environment)
{
  readWindSpeed(environment);
  readWindDirection(environment);
  readTemperature(environment);
  readLux(environment);
  readBME(environment);
  readUV(environment);
  readBattery(environment);
}

//=======================================================
//  readTemperature: Read 1W DS1820B
//=======================================================
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
    MonPrintf("Error: Could not read temperature data\n");
    environment->temperatureF = -40;
    environment->temperatureC = -40;
  }
}

//=======================================================
//  readBattery: read analog volatage divider value
//=======================================================
void readBattery (struct sensorData *environment)
{
  int val;
  float Vout;
  val = analogRead(VOLT_PIN);
  //this value may need tweaking for your voltage divider
  //cabibration = 4.2V/analog value read @ 4.2V
  environment->batteryADC = val;
  environment->batteryVoltage = val * batteryCalFactor;
  MonPrintf("Battery digital :%i voltage: %6.2f\n", val, environment->batteryVoltage);
  //check for low battery situation
  if (environment->batteryVoltage < 3.78)
  {
    lowBattery = true;
  }
  else
  {
    lowBattery = false;
  }
}

//=======================================================
//  readLux: LUX sensor read
//=======================================================
void readLux(struct sensorData *environment)
{
  environment->lux = lightMeter.readLightLevel();
  MonPrintf("LUX value: %6.2f\n", environment->lux);
}

//=======================================================
//  readBME: BME sensor read
//=======================================================
void readBME(struct sensorData *environment)
{
#ifndef METRIC
  bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Fahrenheit, BME280::PresUnit_inHg);
#else
  bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Celsius, BME280::PresUnit_Pa);
#endif
  MonPrintf("BME barometric pressure: %6.2f  BME temperature: %6.2f  BME humidity: %6.2f\n", environment->barometricPressure, environment->BMEtemperature, environment->humidity);
}

//=======================================================
//  readUV: get implied uv sensor value
//=======================================================
void readUV(struct sensorData *environment)
{
  environment->UVIndex = uv.readUV() / 100;
  MonPrintf("UV Index: %f\n", environment->UVIndex);
  MonPrintf("Vis: %i\n", uv.readVisible());
  MonPrintf("IR: %i\n", uv.readIR());
}
