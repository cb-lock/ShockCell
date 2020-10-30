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
  String chastikeyHolder;
  unsigned long endTime;
  String endTimeStr;
  bool elapsedTimeDisplay;
  String releaseCode;
  unsigned long timeOfLastUnlock;
  unsigned long timeOfLastOpening;
  unsigned long timeOfLastClosing;
  unsigned long timeOfLastShock;
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

  unsigned long GetRemainingTime(bool forDisplay = false);
  unsigned long GetEndTime() { return endTime; }
  void SetEndTime(unsigned long t) { endTime = t; }
  String GetEndTimeStr() { return endTimeStr; }
  void SetEndTimeStr(String t) { endTimeStr = t; }
  String GetReleaseCode() { return releaseCode; }
  void SetReleaseCode(String c) { releaseCode = c; }

  void Punishment(int level);
  void Shock(int count, long milliseconds);
  void Lock();
  void ForcedUnlock();
  void Unlock();
  void InfoChastikey();
};

#endif

//
