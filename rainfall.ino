void clearRainfall(void)
{
  
  memset(&rainfall, 0x00, sizeof(&rainfall));
}

void clearRainfallHour(int hourPtr)
{
  rainfall.hourlyRainfall[hourPtr % 24] = 0;
}

void addTipsToHour(int count)
{
  int hourPtr = timeinfo.tm_hour;
  rainfall.hourlyRainfall[hourPtr] = rainfall.hourlyRainfall[hourPtr] + count;
}

void printHourlyArray (void)
{
  int hourCount = 0;
  for (hourCount = 0; hourCount < 24; hourCount++)
  {
    Serial.printf("Hour %i: %u\n", hourCount, rainfall.hourlyRainfall[hourCount]);
  }
}
int last24(void)
{
  int hour;
  int totalRainfall=0;
  for (hour = 0; hour < 24; hour++)
  {
    totalRainfall += rainfall.hourlyRainfall[hour];
  }
  Serial.printf("Total rainfall: %i\n",totalRainfall);
  return totalRainfall;
}
//ISR
void rainTick(void)
{
  rainTicks++;
}
