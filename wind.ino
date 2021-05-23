//None of these functions are validated/working yet

// Variables used in calculating the windspeed
volatile unsigned long timeSinceLastTick = 0;
volatile unsigned long lastTick = 0;
float windSpeed;

int readWindVelocity(void )
{
  if (timeSinceLastTick != 0)
  {
    windSpeed = 1000.0 / timeSinceLastTick;
  }
  else
  {
    windSpeed = 0;
  }
  return windSpeed;
}
//This function is in testing mode now
String readWindDirection(void)
{
  int windPosition;
  String windDirection = "N";
  int analogCompare[15] = {150,300,400,600,900,1100,1500,1700,2250,2350,2700,3000,3200,3400,4000};
  String windDirText[15]={"X","S","X","X","X","E","X","X","X","X","X","X","X","X","N"};
  int vin = analogRead(WIND_DIR_PIN);

  for(windPosition=0;windPosition<15;windPosition++)
  {
    if(vin < analogCompare[windPosition])
    {
      windDirection=windDirText[windPosition];
      break;
    }
  }
  Serial.printf("Wind direction: %s\n",windDirection);
  return windDirection;
}


//ISR
void windTick(void)
{
  timeSinceLastTick = millis() - lastTick;
  lastTick = millis();
}
