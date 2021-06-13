
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -7 * 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

//=======================================================================
//  printLocalTime: prints local timezone based time
//=======================================================================
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    MonPrintf("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S\n");
  MonPrintf("Date:%i %i %i Time: %i:%i:%i\n", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

//=======================================================================
//  printTimeNextWake: diagnostic routine to print next wake time
//=======================================================================
void printTimeNextWake( void)
{
  getLocalTime(&timeinfo);
  MonPrintf("Time to next wake: %i seconds\n", nextUpdate);
}

//=======================================================================
//  updateWake: calculate next time to wake
//=======================================================================
void updateWake (void)
{
  getLocalTime(&timeinfo);
  nextUpdate = mktime(&timeinfo) + UpdateIntervalSeconds;
}
