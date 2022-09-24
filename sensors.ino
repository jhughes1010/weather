OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

extern "C"
{
  uint8_t temprature_sens_read();
}

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
  readPR(environment);
  readBME(environment);
  readUV(environment);
  readBattery(environment);
  readESPCoreTemp(environment);
}

//=======================================================
//  readTemperature: Read 1W DS1820B
//=======================================================
void readTemperature (struct sensorData *environment)
{
  MonPrintf("Requesting temperatures...\n");
  temperatureSensor.requestTemperatures();
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
  environment->batteryADC = analogRead(VOLT_PIN);
  environment->batteryVoltage = environment->batteryADC * batteryCalFactor;
  MonPrintf("Battery digital ADC :%i voltage: %6.2f\n", environment->batteryADC, environment->batteryVoltage);
  //check for low battery situation
  if (environment->batteryVoltage < batteryLowVoltage)
  {
    lowBattery = true;
  }
  else
  {
    lowBattery = false;
  }
}

//=======================================================
//  checkBatteryVoltage: set/reset low voltage flag only
//=======================================================
void checkBatteryVoltage (void)
{
  int adc;
  float voltage;
  adc = analogRead(VOLT_PIN);
  voltage = adc * batteryCalFactor;
  //check for low battery situation
  if (voltage < batteryLowVoltage)
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
#ifdef BH1750Enable
  if (status.lightMeter)
  {
    environment->lux = lightMeter.readLightLevel();
  }
  else
  {
    environment->lux = -1;
  }
#else
  environment->lux = -3;
#endif
  MonPrintf("LUX value: %6.2f\n", environment->lux);
}

//=======================================================
//  readPR: photoresistor ADC read
//=======================================================
void readPR(struct sensorData *environment)
{
  int vin;

  vin = analogRead(PR_PIN);

  MonPrintf("photoresistor value: %i photoresistor\n", vin);
  environment->photoresistor = vin;
}
//=======================================================
//  readBME: BME sensor read
//=======================================================
void readBME(struct sensorData *environment)
{
  if (status.bme)
  {
#ifndef METRIC
    bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Fahrenheit, BME280::PresUnit_inHg);
    environment->barometricPressure += ALTITUDE_OFFSET_IMPERIAL;
#else
    bme.read(environment->barometricPressure, environment->BMEtemperature, environment->humidity, BME280::TempUnit_Celsius, BME280::PresUnit_Pa);
    environment->barometricPressure += ALTITUDE_OFFSET_METRIC;
#endif

  }
  else
  {
    //set to insane values
    environment->barometricPressure = -100;
    environment->BMEtemperature = -100;
    environment->humidity = -100;
  }
  MonPrintf("BME barometric pressure: %6.2f  BME temperature: %6.2f  BME humidity: %6.2f\n", environment->barometricPressure, environment->BMEtemperature, environment->humidity);

}

//=======================================================
//  readUV: get implied uv sensor value
//=======================================================
void readUV(struct sensorData *environment)
{
  if (status.uv)
  {
    environment->UVIndex = (float) uv.readUV() / 100;
  }
  else
  {
    environment->UVIndex = -1;
  }
  MonPrintf("UV Index: %f\n", environment->UVIndex);
  MonPrintf("Vis: %i\n", uv.readVisible());
  MonPrintf("IR: %i\n", uv.readIR());
}

void readESPCoreTemp(struct sensorData *environment)
{
  unsigned int coreF, coreC;
  coreF = temprature_sens_read();
  coreC = (coreF - 32) * 5 / 9;

  environment->coreF = coreF;
  environment->coreC = coreC;
}


void printADCLCD( void)
{
  int adc;
  adc = analogRead(VOLT_PIN);
  display.printf("ADC: %i\n", adc);
}
