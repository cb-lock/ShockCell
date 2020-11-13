#ifndef __TimeFunctions_h__

#define __TimeFunctions_h__

#include "Arduino.h"

class TimeFunctions
{
private:
public:
  String Time2String(const long seconds);
  long GetTimeInSeconds();
  int GetHours();
  int GetMinutes();
  bool IsWeekend();
  bool IsSleepingTime();
  bool SleepingTimeJustChanged(bool started);
  String GetTimeString(unsigned long t = 0);
  void SetClock();
};

#endif

//
