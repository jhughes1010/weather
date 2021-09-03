// Variables used in software delay to supress spurious counts on rain_tip
volatile unsigned long timeSinceLastTip = 0;
volatile unsigned long validTimeSinceLastTip = 0;
volatile unsigned long lastTip = 0;

//=======================================================================
//  clearRainfall: zero out rainfall counter structure
//=======================================================================
void clearRainfall(void)
{
  memset(&rainfall, 0x00, sizeof(rainfall));
}



//=======================================================================
//
//  Hourly accumulation routines
//
//=======================================================================


//=======================================================================
//  clearRainfallHour: zero out specific hour element of rainfall structure array
//=======================================================================
void clearRainfallHour(int hourPtr)
{
  //Clear carryover if hourPtr is not matching prior hourPtr value (we have a new hour)
  if (rainfall.priorHour != hourPtr)
  {
    rainfall.hourlyCarryover = 0;
  }
  //move contents of oldest hour to the carryover location and set hour to zero
  rainfall.hourlyCarryover += rainfall.hourlyRainfall[hourPtr % 24];
  rainfall.hourlyRainfall[hourPtr % 24] = 0;

  rainfall.priorHour = hourPtr;
}

//=======================================================================
//  addTipsToHour: increment current hour tip count
//=======================================================================
void addTipsToHour(int count)
{
  int hourPtr = timeinfo.tm_hour;
  rainfall.hourlyRainfall[hourPtr] = rainfall.hourlyRainfall[hourPtr] + count;
}

//=======================================================================
//  printHourlyArray: diagnostic routine to print hourly rainfall array to terminal
//=======================================================================
void printHourlyArray (void)
{
  int hourCount = 0;
  for (hourCount = 0; hourCount < 24; hourCount++)
  {
    MonPrintf("Hour %i: %u\n", hourCount, rainfall.hourlyRainfall[hourCount]);
  }
}

//=======================================================================
//  last24: return tip counter for last 24h
//=======================================================================
int last24(void)
{
  int hour;
  int totalRainfall = 0;
  for (hour = 0; hour < 24; hour++)
  {
    totalRainfall += rainfall.hourlyRainfall[hour];
  }
  //add carryover value
  totalRainfall += rainfall.hourlyCarryover;

  MonPrintf("Total rainfall (last 24 hours): %i\n", totalRainfall);
  return totalRainfall;
}


//=======================================================================
//
//  Minute accumulation routines
//
//=======================================================================
// NOTE: When speaking of minutes and minute array, we use 5 min as
// minimum grouping for minute-by-minute rainfall



//=======================================================================
//  clearRainfallMinute: zero out specific minute element of rainfall structure array
//=======================================================================
void clearRainfallMinute(int hourPtr)
{
  //Clear carryover if hourPtr is not matching prior hourPtr value (we have a new hour)
  if (rainfall.priorHour != hourPtr)
  {
    rainfall.hourlyCarryover = 0;
  }
  //move contents of oldest hour to the carryover location and set hour to zero
  rainfall.hourlyCarryover += rainfall.hourlyRainfall[hourPtr % 24];
  rainfall.hourlyRainfall[hourPtr % 24] = 0;

  rainfall.priorHour = hourPtr;
}

//=======================================================================
//  addTipsToMinute: increment current hour tip count
//=======================================================================
void addTipsToMinute(int count)
{
  int minute = timeinfo.tm_hour;
  rainfall.current60MinRainfall[minute] = rainfall.current60MinRainfall[minute] + count;
}

//=======================================================================
//  printMinuteArray: diagnostic routine to print minute rainfall array to terminal
//=======================================================================
void printMinuteArray (void)
{
  int minute = 0;
  for (minute = 0; minute < 12; minute++)
  {
    MonPrintf("Minute %i: %u\n", minute * 5, rainfall.current60MinRainfall[minute]);
  }
}

//=======================================================================
//  last60min: return tip counter for last 60 minutes
//=======================================================================
int last60min(void)
{
  int minute;
  int totalRainfall = 0;
  for (minute = 0; minute < 12; minute++)
  {
    totalRainfall += rainfall.current60MinRainfall[minute];
  }
  //add carryover value
  totalRainfall += rainfall.minuteCarryover;

  MonPrintf("Total rainfall (last 60 minutes): %i\n", totalRainfall);
  return totalRainfall;
}




//=======================================================================
//  rainTick: ISR for rain tip gauge count
//=======================================================================
//ISR
void IRAM_ATTR rainTick(void)
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
