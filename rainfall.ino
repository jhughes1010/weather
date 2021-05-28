// Variables used in software delay to supress spurious counts on rain_tip
volatile unsigned long timeSinceLastTip = 0;
volatile unsigned long validTimeSinceLastTip = 0;
volatile unsigned long lastTip = 0;


void clearRainfall(void)
{
  memset(&rainfall, 0x00, sizeof(rainfall));
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
    MonPrintf("Hour %i: %u\n", hourCount, rainfall.hourlyRainfall[hourCount]);
  }
}
int last24(void)
{
  int hour;
  int totalRainfall = 0;
  for (hour = 0; hour < 24; hour++)
  {
    totalRainfall += rainfall.hourlyRainfall[hour];
  }
  MonPrintf("Total rainfall: %i\n", totalRainfall);
  return totalRainfall;
}
//ISR
void rainTick(void)
{
  timeSinceLastTip = millis() - lastTip;
  //software debounce attempt
  if (timeSinceLastTip > 400)
  {
    validTimeSinceLastTip = timeSinceLastTip;
    rainTicks++;
    lastTip = millis();
  }
}
