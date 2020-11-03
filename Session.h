#ifndef __Session_h__

#define __Session_h__

#include "Arduino.h"

#include "Defs.h"
#include "TimeF.h"
#include "EServer.h"
#include "User.h"


class Session
{
private:
  String id;
  String chatId;
//  User wearer;
//  User holder;
//  User supervisor;
  bool activeChastikeySession = false;
  bool randomShockMode = false;
  bool teasingMode = false;
  unsigned long randomShocksPerHour = 1;
  String chastikeyHolder;
  unsigned long endTime;
  String endTimeStr;
  bool elapsedTimeDisplay;
  String releaseCode;
  int emergencyReleaseCounter = 0;
  bool emergencyReleaseCounterRequest = false;
  unsigned long timeOfLastUnlock = 0;
  unsigned long timeOfLastOpening = 0;
  unsigned long timeOfLastClosing = 0;
  unsigned long timeOfLastShock = 0;
  unsigned long timeOfNextScheduledShock = 0;
  unsigned long timeOfLast5sInterval = 0;
  unsigned long timeOfLast5minInterval = 0;
//  String lastShockOwner;
//  String lastShockReason;

public:
  Session() {}
  String GetId() { return id; }
  void SetId(String i) { id = i; Serial.print("Set session id="); Serial.println(i); }

  String GetChatId() { return chatId; }
  void SetChatId(String i) { chatId = i; Serial.print("Set chat id="); Serial.println(i); }

//  User & GetWearer() { return wearer; }
//  void SetWearerId(String i) { wearer.SetId(i); }
//  void SetWearerApikey(String a) { wearer.SetApikey(a); }
//  User & GetHolder() { return holder; }
//  void SetHolderApikey(String a) { holder.SetApikey(a); }
//  User & GetSupervisor() { return supervisor; }
//  bool HasSupervisor() { return (supervisor.GetId().length() > 0); }
  bool IsActiveChastikeySession() { return activeChastikeySession; }
  void SetActiveChastikeySession(bool is) { activeChastikeySession = is; }

  String GetChastikeyHolder() { return chastikeyHolder; }
  void SetChastikeyHolder(String hname) { chastikeyHolder = hname; }

  unsigned long GetTimeOfLastUnlock() { return timeOfLastUnlock; }
  void SetTimeOfLastUnlock(unsigned long t) { timeOfLastUnlock = t; }

  unsigned long GetTimeOfLastOpening() { return timeOfLastOpening; }
  void SetTimeOfLastOpening(unsigned long t) { timeOfLastOpening = t; }

  unsigned long GetTimeOfLastClosing() { return timeOfLastClosing; }
  void SetTimeOfLastClosing(unsigned long t) { timeOfLastClosing = t; }

  unsigned long GetTimeOfLastShock() { return timeOfLastShock; }
  void SetTimeOfLastShock(unsigned long t) { timeOfLastShock = t; }

  void SetTimeOfLast5sInterval(unsigned long setTime) { timeOfLast5sInterval = setTime; }
  unsigned long GetTimeOfLast5sInterval() { return timeOfLast5sInterval; }
  void SetTimeOfLast5minInterval(unsigned long setTime) { timeOfLast5minInterval = setTime; }
  unsigned long GetTimeOfLast5minInterval() { return timeOfLast5minInterval; }

  unsigned long GetRemainingTime(bool forDisplay = false);
  unsigned long GetEndTime() { return endTime; }
  void SetEndTime(unsigned long t) { endTime = t; }
  String GetEndTimeStr() { return endTimeStr; }
  void SetEndTimeStr(String t) { endTimeStr = t; }
  String GetReleaseCode() { return releaseCode; }
  void SetReleaseCode(String c) { releaseCode = c; }

  void SetTeasingMode(bool mode) { teasingMode = mode; }
  bool IsTeasingMode() { return teasingMode; }

  void SetEmergencyReleaseCounter(int newVal) { emergencyReleaseCounter = newVal; emergencyReleaseCounterRequest = false; }
  int GetEmergencyReleaseCounter() { return emergencyReleaseCounter; }
  void SetEmergencyReleaseCounterRequest(bool req) { emergencyReleaseCounterRequest = req; }
  bool GetEmergencyReleaseCounterRequest() { return emergencyReleaseCounterRequest; }

  void Punishment(int level);
  void Shock(int count, long milliseconds);
  void SetRandomMode(bool onOff, int shocksPerHour=1);
  void Lock();
  void ForcedUnlock();
  void Unlock();
  void InfoChastikey();

  void ProcessRandomShocks();
  void ScheduleNextRandomShock();
};

#endif

//
