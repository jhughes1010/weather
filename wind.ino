//This function is in testing mode now

// Variables used in calculating the windspeed
volatile unsigned long timeSinceLastTick = 0;
volatile unsigned long validTimeSinceLastTick = 0;
volatile unsigned long lastTick = 0;

//=======================================================
//  readWindSpeed: Single instantaneous measurement of wind speed
//=======================================================
void readWindSpeed(struct sensorData *environment )
{
  float windSpeed = 0;
  int position;
  //int msBetweenSamples = 0;
  int msTotal = 0;
  int samples = 0;

  /*
    for (position = 0; position < 10; position++)
    {
    Serial.println(tickTime[position]);
    }*/
  //intentionally ignore the zeroth element
  for (position = 1; position < 5; position++)
  {
    //msBetweenSamples = tickTime[position + 1] - tickTime[position];
    if (tickTime[position])
    {
      msTotal += tickTime[position];
      samples ++;
    }
  }
  //Average samples
  if (msTotal && samples)
  {
    windSpeed = 1.49 * 1000 / (msTotal / samples);
  }
  else
  {
    Serial.println("No Wind data");
    windSpeed = 0;
  }
  //I see 2 ticks per revolution
  windSpeed = windSpeed / WIND_TICKS_PER_REVOLUTION;

#ifdef METRIC
  windSpeed =  windSpeed * 1.60934;
#endif
  MonPrintf("WindSpeed time period: %i\n", validTimeSinceLastTick);
  MonPrintf("WindSpeed: %f\n", windSpeed);
  windSpeed = int((windSpeed + .5) * 10) / 10;
  environment->windSpeed = windSpeed;
}
/*
  void readWindSpeedOld(struct sensorData *environment )
  {
  float windSpeed = 0;
  if (validTimeSinceLastTick != 0)
  {
    windSpeed = 1.49 * 1000 / validTimeSinceLastTick;
    //I see 2 ticks per revolution
    windSpeed = windSpeed / WIND_TICKS_PER_REVOLUTION;


    //Starting to get insane values
    if (windSpeed > 100)
    {
      windSpeed = 100;
    }
  }
  else
  {
    windSpeed = 0;
  }
  #ifdef METRIC
  windSpeed =  windSpeed * 1.60934;
  #endif
  MonPrintf("WindSpeed time period: %i\n", validTimeSinceLastTick);
  MonPrintf("WindSpeed: %f\n", windSpeed);
  windSpeed = int((windSpeed + .5) * 10) / 10;
  environment->windSpeed = windSpeed;

  //reset for next measurement
  validTimeSinceLastTick = 0;
  lastTick = 0;

  }
*/
//=======================================================
//  readWindDirection: Read ADC to find wind direction
//=======================================================
//This function is in testing mode now
void readWindDirection(struct sensorData *environment)
{
  int windPosition;
  String windDirection = "0";
  int analogCompare[15] = {150, 300, 450, 600, 830, 1100, 1500, 1700, 2250, 2350, 2700, 3000, 3200, 3400, 3900};
  //String windDirText[15] = {"SSW", "S", "WSW", "3", "SW", "W", "6", "ESE", "SE", "NNW", "NW", "ENE", "E", "NNE", "NE"}; //BLYNK does not seem to allow text
  String windDirText[15] = {"202.5", "180", "247.5", "000", "225", "270", "000", "112.5", "135", "337.5", "315", "67.5", "90", "22.5", "45"};
  char buffer[10];
  int vin = analogRead(WIND_DIR_PIN);

  for (windPosition = 0; windPosition < 15; windPosition++)
  {
    if (vin < analogCompare[windPosition])
    {
      windDirection = windDirText[windPosition];
      break;
    }
  }
  MonPrintf("Analog value: %i Wind direction: %s  \n", vin, windDirection);
  windDirection.toCharArray(buffer, 5);
  environment->windDirection = atof(buffer);
}

//=======================================================
//  windTick: ISR to capture wind speed relay closure
//=======================================================
//ISR

void windTick(void)
{
  static int count = 0;

  timeSinceLastTick = millis() - lastTick;
  //software debounce attempt
  if (timeSinceLastTick > 10 && count < 10)
  {
    //validTimeSinceLastTick = timeSinceLastTick;
    lastTick = millis();
    tickTime[count] = timeSinceLastTick;
    count++;
  }
}
