struct tm timeinfo;

//=======================================================================
//  printLocalTime: prints local timezone based time
//=======================================================================
void printLocalTime()
{
  if (!getLocalTime(&timeinfo))
  {
    MonPrintf("Failed to obtain time\n");
    return;
  }
  Serial.printf("Date:%02i %02i %i Time: %02i:%02i:%02i\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void printLocalTimeLCD(void)
{
  if (!getLocalTime(&timeinfo))
  {
    MonPrintf("Failed to obtain time\n");
    return;
  }
  display.printf("Date:%02i %02i %i\nTime: %02i:%02i:%02i\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

//=======================================================================
//  printTimeNextWake: diagnostic routine to print next wake time
//=======================================================================
void printTimeNextWake( void)
{
  getLocalTime(&timeinfo);
  Serial.printf("Time to next wake: %i seconds\n", nextUpdate - mktime(&timeinfo) );
}

//=======================================================================
//  updateWake: calculate next time to wake
//=======================================================================
void updateWake (void)
{
  MonPrintf("Checking for low battery\n");
  checkBatteryVoltage();
  int muliplierBatterySave = 1;
  if (lowBattery)
  {
    MonPrintf("Flag set for low battery\n");
    muliplierBatterySave = 10;
  }
  getLocalTime(&timeinfo);
  //180 added to wipe out any RTC timing error vs NTP server - causing 2 WAKES back to back
  nextUpdate = mktime(&timeinfo) + UpdateIntervalSeconds * muliplierBatterySave + 180;
  nextUpdate = nextUpdate - nextUpdate % (UpdateIntervalSeconds * muliplierBatterySave);
  // Intentional offset for data aquire before display unit updates
  // guarantees fresh data
  if (nextUpdate > 120)
  {
    nextUpdate -= 60;
  }
}
