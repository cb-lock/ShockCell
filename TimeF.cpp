#include <time.h>
#include "Defs.h"
#include "TimeF.h"
#include "Message.h"
#include "Session.h"
#include "User.h"


extern Message message;
extern Session session;
extern UserSet users;


// ------------------------------------------------------------------------
bool TimeFunctions::IsWeekend()
{
//  Serial.println("*** IsWeekend()");
//  Serial.print("- day of week: ");
  // 0 Monday, 6 Sunday
  int dayOfWeek = GetDayOfWeek();
//  Serial.println(dayOfWeek);

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
String TimeFunctions::Time2StringNoDays(const long seconds)
{
  return Time2String((seconds % 86400) + UTC_OFFSET*3600);
}


// ------------------------------------------------------------------------
String TimeFunctions::Time2StringNoDaysCompact(const long seconds)
{
  String str = Time2String((seconds % 86400) + UTC_OFFSET*3600);
  int pos = str.indexOf('h');
  if (pos >= 0)
    str.remove(pos, 2);
  pos = str.indexOf('m');
  if (pos >= 0)
    str.remove(pos, 1);
  return str;
}


// ------------------------------------------------------------------------
unsigned long TimeFunctions::GetMidnightToday()
{
  time_t nowSecs = GetTimeInSeconds();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  int hours = (timeinfo.tm_hour + UTC_OFFSET) % 24;
  return GetTimeInSeconds() - (hours*3600 + timeinfo.tm_min*60 + timeinfo.tm_sec);
}


// ------------------------------------------------------------------------
unsigned long TimeFunctions::WakeUpTime()
{
  int sleepingExtension = IsWeekend() ? 3 : 0;
  return GetMidnightToday() + 3600*(SLEEP_TIME_END_WORKDAYS + sleepingExtension);
}


// ------------------------------------------------------------------------
unsigned long TimeFunctions::SleepTime()
{
  return GetMidnightToday() + 3600*SLEEP_TIME_BEGIN;
}


// ------------------------------------------------------------------------
unsigned long TimeFunctions::GetTimeInSeconds()
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
  int sleepingExtension = IsWeekend() ? 3 : 0;
  Serial.print("- sleepingExtension: ");
  Serial.println(sleepingExtension);
  Serial.print("- hours: ");
  Serial.println(hours);
  //
  unsigned long sinceMidnight = GetMidnightToday();
  Serial.print("- sinceMidnight: ");
  Serial.println(sinceMidnight);
  Serial.println(GetTimeString(sinceMidnight));
  Serial.print("- now: ");
  Serial.println(GetTimeInSeconds());
  Serial.println(GetTimeString(GetTimeInSeconds()));
  //

  if ((hours < SLEEP_TIME_END_WORKDAYS + sleepingExtension) || (hours >= SLEEP_TIME_BEGIN))
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
    if ((hours == SLEEP_TIME_END_WORKDAYS) && (timeinfo.tm_min < 6))
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


// ------------------------------------------------------------------------
void TimeFunctions::ProcessSleepTime()
{
  Serial.println("*** ProcessSleepTime()");
  if (session.IsActiveSession())
  {
    Serial.println("- active session.");
    Serial.print("- sleep time: ");
    Serial.println(IsSleepingTime() ? "true" : "false");
    Serial.print("- wearer sleeps: ");
    Serial.println(users.GetWearer()->IsSleeping());
    // we assume an active session here
    if (IsSleepingTime() && (! users.GetWearer()->IsSleeping()))
    {
      // Go to sleep
      users.GetWearer()->SetSleeping(true);
      message.SendMessage("Good night "  + users.GetWearer()->GetName() + ", sleep well and frustrated.");
    }
    if ((! IsSleepingTime()) && users.GetWearer()->IsSleeping())
    {
      // Wake up
      users.GetWearer()->SetSleeping(false);
      message.SendMessage("Good morning "  + users.GetWearer()->GetName() + ", wake up boy!");
    }
  }
/*
  bool wasSleeping = users.GetWearer()->IsSleeping();
  if (timeFunc.SleepingTimeJustChanged(true))
  {
    users.GetWearer()->SetSleeping(true);
//        if (! wasSleeping)
    {
      msg = "Good night "  + users.GetWearer()->GetName() + ", sleep well and frustrated.";
      message.SendMessage(msg);
    }
  }
  // has the sleeping period just ended?
  if (timeFunc.SleepingTimeJustChanged(false))
  {
    users.GetWearer()->SetSleeping(false);
    if (wasSleeping)
    {
      msg = "Good morning "  + users.GetWearer()->GetName() + ", wake up boy!";
      message.SendMessage(msg);
    }
  }
  */
}


//
