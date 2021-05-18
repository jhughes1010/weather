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
String readWindDirection(void)
{
  String windDir;
  int vin = analogRead(WIND_DIR_PIN);

  if      (vin < 150) windDir = "202.5";
  else if (vin < 300) windDir = "180";
  else if (vin < 400) windDir = "247.5";
  else if (vin < 600) windDir = "225";
  else if (vin < 900) windDir = "292.5";
  else if (vin < 1100) windDir = "270";
  else if (vin < 1500) windDir = "112.5";
  else if (vin < 1700) windDir = "135";
  else if (vin < 2250) windDir = "337.5";
  else if (vin < 2350) windDir = "315";
  else if (vin < 2700) windDir = "67.5";
  else if (vin < 3000) windDir = "90";
  else if (vin < 3200) windDir = "22.5";
  else if (vin < 3400) windDir = "45";
  else if (vin < 4000) windDir = "0";
  else windDir = "0";
}


//ISR
void windTick(void)
{
  timeSinceLastTick = millis() - lastTick;
  lastTick = millis();
}
