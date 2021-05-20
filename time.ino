
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -7 * 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void printTimeNextWake( void)
{
  //struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.printf("Time to next wake: %i seconds\n", nextUpdate);
}

void updateWake (void)
{
  getLocalTime(&timeinfo);
  nextUpdate = mktime(&timeinfo) + UpdateInterval;
}
