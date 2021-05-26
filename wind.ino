//None of these functions are validated/working yet

// Variables used in calculating the windspeed
volatile unsigned long timeSinceLastTick = 0;
volatile unsigned long validTimeSinceLastTick = 0;
volatile unsigned long lastTick = 0;
volatile int windTickCnt = 0;
//float windSpeed;

void readWindSpeed(struct sensorData *environment )
{
  int windSpeed;
  if (validTimeSinceLastTick != 0)
  {
    windSpeed = 1.49 * 1000 / validTimeSinceLastTick;
    //I see 2 ticks per revolution
    windSpeed = windSpeed / 2;
    if (windSpeed > 50)
    {
      windSpeed = 50;
    }
  }

  else
  {
    windSpeed = 0;
  }
  Serial.printf("WindSpeed time: %i\n", validTimeSinceLastTick);
  Serial.printf("windTick: %i\n", windTickCnt);
  environment->windSpeed = windSpeed;
}

//This function is in testing mode now
void readWindDirection(struct sensorData *environment)
{
  int windPosition;
  String windDirection = "N";
  int analogCompare[15] = {150, 300, 400, 600, 900, 1100, 1500, 1700, 2250, 2350, 2700, 3000, 3200, 3400, 4000};
  String windDirText[15] = {"X", "S", "X", "X", "X", "E", "X", "X", "X", "X", "X", "X", "X", "X", "N"};
  int vin = analogRead(WIND_DIR_PIN);

  for (windPosition = 0; windPosition < 15; windPosition++)
  {
    if (vin < analogCompare[windPosition])
    {
      windDirection = windDirText[windPosition];
      break;
    }
  }
  Serial.printf("Analog value: %i Wind direction: %s\n", vin, windDirection);
  environment->windDirection;
}


//ISR
void windTick(void)
{
  const int aquireCount = 30;
  timeSinceLastTick = millis() - lastTick;
  //if (timeSinceLastTick > 50)
  {
    validTimeSinceLastTick = timeSinceLastTick;
    windTickCnt++;
    lastTick = millis();
  }
}
