void clearRainfall(void)
{
  memset(hourlyRainfall, 0x00, 24);
}

void clearRainfallHour(int hourPtr)
{
  hourlyRainfall[hourPtr % 24] = 0;
}

void addTipsToHour(int count)
{
  int hourPtr = timeinfo.tm_hour;
  hourlyRainfall[hourPtr] = hourlyRainfall[hourPtr] + count;
}

void printHourlyArray (void)
{
  int hourCount = 0;
  for (hourCount = 0; hourCount < 24; hourCount++)
  {
    Serial.printf("Hour %i: %u\n", hourCount, hourlyRainfall[hourCount]);
  }
}
int last24(void)
{
  int hour;
  int totalRainfall=0;
  for (hour = 0; hour < 24; hour++)
  {
    totalRainfall += hourlyRainfall[hour];
  }
  Serial.printf("Total rainfall: %i\n",totalRainfall);
  return totalRainfall;
}
//ISR
void rainTick(void)
{
  rainTicks++;
}
