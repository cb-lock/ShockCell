#ifndef __TimeFunctions_h__

#define __TimeFunctions_h__

#define TIME_MAX 4102444799L

#include "Arduino.h"

class TimeFunctions
{
private:
  int dstOffset = 1;

public:
  String Time2String(const long seconds, bool duration=false);
  String Time2StringNoDays(const long seconds, bool duration=false);
  String Time2StringNoDaysCompact(const long seconds, bool duration=false);
  unsigned long GetTimeInSeconds();
  int GetHours();
  int GetMinutes();
  unsigned long GetMidnightToday();
  int GetDayOfWeek() { return (((GetTimeInSeconds() / 86400L) + 3) % 7); }
  int UpdateDSTOffset();
  int GetNumberOfDays(unsigned long duration) { return (duration / 86400); }
  bool IsWeekend();
  bool IsSleepingTime();
  unsigned long WakeUpTime();
  unsigned long SleepTime();
  bool SleepingTimeJustChanged(bool started);
  String GetTimeString(unsigned long t = 0);
  void SetClock();
  void ProcessSleepTime();
};

#endif

//
