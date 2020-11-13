#include <time.h>
#include "Defs.h"
#include "TimeF.h"



// ------------------------------------------------------------------------
bool TimeFunctions::IsWeekend()
{
  Serial.println("*** IsWeekend()");
  Serial.print("- day of week: ");
  // 0 Monday, 6 Sunday
  int dayOfWeek = ((GetTimeInSeconds() / 86400L) + 5) % 7;
  Serial.println(dayOfWeek);

  if (dayOfWeek < 5)
    return false;
  else
    return true;
}


// ------------------------------------------------------------------------
String TimeFunctions::Time2String(const long seconds)
{
//  Serial.println("Time2String()");
//  Serial.println(seconds);
  char value[100];
  if (seconds < 3600)
    sprintf(value, "%dm %ds", seconds / 60, seconds % 60);
  else if (seconds < 24 * 3600)
    sprintf(value, "%dh %dm", seconds / 3600, (seconds % 3600) / 60);
  else
    sprintf(value, "%dd %dh", seconds / (24 * 3600), (seconds % (24 * 3600) / 3600));

//  Serial.println(value);
  return String(value);
}


// ------------------------------------------------------------------------
long TimeFunctions::GetTimeInSeconds()
{
  time_t nowSecs = time(nullptr);
  return nowSecs;
}


// ------------------------------------------------------------------------
int TimeFunctions::GetHours()
{
  time_t nowSecs = GetTimeInSeconds();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  int hours = (timeinfo.tm_hour + UTC_OFFSET) % 24;
  return hours;
}


// ------------------------------------------------------------------------
int TimeFunctions::GetMinutes()
{
  time_t nowSecs = GetTimeInSeconds();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  return timeinfo.tm_min;
}


// ------------------------------------------------------------------------
bool TimeFunctions::IsSleepingTime()
{
  Serial.println("*** IsSleepingTime()");
  time_t nowSecs = GetTimeInSeconds();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  int hours = (timeinfo.tm_hour + UTC_OFFSET) % 24;
  unsigned long sleepingExtension = IsWeekend() ? 3*3600L : 0;
  Serial.print("- sleepingExtension: ");
  Serial.println(sleepingExtension);

  if ((hours < SLEEP_TIME_END + sleepingExtension) || (hours >= SLEEP_TIME_BEGIN))
    return true;
  else
    return false;
}


// ------------------------------------------------------------------------
bool TimeFunctions::SleepingTimeJustChanged(bool started)
{
  time_t nowSecs = GetTimeInSeconds();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  int hours = (timeinfo.tm_hour + UTC_OFFSET) % 24;

  // sleeping time just started?
  if (started)
  {
    if ((hours == SLEEP_TIME_BEGIN) && (timeinfo.tm_min < 6))
      return true;
    else
      return false;
  }

  // sleeping time just ended?
  else
  {
    if ((hours == SLEEP_TIME_END) && (timeinfo.tm_min < 6))
      return true;
    else
      return false;
  }
}


// ------------------------------------------------------------------------
String TimeFunctions::GetTimeString(unsigned long t)
{
  time_t nowSecs;

  if (t == 0)
    nowSecs = GetTimeInSeconds();
  else
    nowSecs = t;
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  int hours = (timeinfo.tm_hour + UTC_OFFSET) % 24;

  char buf[100];
  sprintf(buf, "%02d:%02d.%02d", hours, timeinfo.tm_min, timeinfo.tm_sec);
  return String(buf);
}


// ------------------------------------------------------------------------
void TimeFunctions::SetClock()
{
  // configTime(int timeZone, int daylightOffsetInSeconds, server1, server2)
  configTime(2*3600, 3600, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "UTC+00:00", 1);

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }
  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current seconds: ");
  Serial.println(nowSecs);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}


//
