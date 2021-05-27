//None of these functions are validated/working yet

// Variables used in calculating the windspeed
volatile unsigned long timeSinceLastTick = 0;
volatile unsigned long validTimeSinceLastTick = 0;
volatile unsigned long lastTick = 0;
//volatile int windTickCnt = 0;


void readWindSpeed(struct sensorData *environment )
{
  float windSpeed=0;
  if (validTimeSinceLastTick != 0)
  {
    windSpeed = 1.49 * 1000 / validTimeSinceLastTick;
    //I see 2 ticks per revolution
    windSpeed = windSpeed / 2;
    if (windSpeed > 100)
    {
      windSpeed = 100;
    }
  }

  else
  {
    windSpeed = 0;
  }
  Serial.printf("WindSpeed time period: %i\n", validTimeSinceLastTick);
  Serial.printf("WindSpeed: %f\n", windSpeed);
  //Serial.printf("windTick: %i\n", windTickCnt);
  windSpeed = int((windSpeed+.5)*10)/10;
  environment->windSpeed = windSpeed;
}

//This function is in testing mode now
void readWindDirection(struct sensorData *environment)
{
  int windPosition;
  String windDirection = "N";
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
  Serial.printf("Analog value: %i Wind direction: %s  \n", vin, windDirection);
  windDirection.toCharArray(buffer, 5);
  environment->windDirection = atof(buffer);
}


//ISR
void windTick(void)
{
  timeSinceLastTick = millis() - lastTick;
  //software debounce attempt
  if (timeSinceLastTick > 10)
  {
    validTimeSinceLastTick = timeSinceLastTick;
    //windTickCnt++;
    lastTick = millis();
  }
}
