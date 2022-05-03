#ifndef __Session_h__

#define __Session_h__

#define MAX_TASKS 10
#define TASK_GENERAL 0
#define TASK_CONFIRMATION 1
#define TASK_NONE -1

#define MAX_VERIFICATIONS 5
#define VERIFICATION_STATUS_BEFORE 0
#define VERIFICATION_STATUS_WAITING 1
#define VERIFICATION_STATUS_RECEIVED 2
#define DURATION_EARLY_VERIFICATION 3*3600L
#define DURATION_LATE_VERIFICATION 4*3600L
#define DURATION_RANDOM_VERIFICATION 900
#define VERIFICATION_BONUS_TIME 60

#define SHOCKS_PER_SESSION 50
#define SHOCK_BACKFILL_PROBABILITY 100
#define SHOCK_NEEDINESS_THRESHOLD 60.0
#define SHOCK_COUNT_MAXMIMUM 30
#define SHOCK_DURATION_MINIMUM 1000
#define SHOCK_DURATION_DEFAULT 2000
#define SHOCK_DURATION_MAXIMUM 20000
#define SHOCK_BREAK_DURATION 2000


#include "Arduino.h"
#include "Defs.h"
#include "TimeF.h"
#include "EServer.h"
#include "User.h"


class Task
{
private:
  int index = 0;
  bool active = false; // if not active, task slot is empty and available for new tasks
  int tType = TASK_GENERAL;
  String message;
  unsigned long timeOfBegin = 0;
  unsigned long timeOfEnd = 0;
  bool announcedBegin = false;
  bool announcedEnd = false;
  bool fulfilled = false;
  // Confirmation
  bool holderConfirmed = false;
  bool wearerConfirmed = false;

public:
  Task() {}
  int GetIndex() { return index; }
  void SetIndex(int i) { index = i; }
  bool IsActive() { return active; }
  void SetActive(bool a) { active = a; }
  int GetType() { return tType; }
  void SetType(int t) { tType = t; }
  String GetMessage() { return message; }
  void SetMessage(String msg) { message = msg; }
  unsigned long GetTimeOfBegin() { return timeOfBegin; }
  void SetTimeOfBegin(unsigned long theTime) { timeOfBegin = theTime; }
  unsigned long GetTimeOfEnd() { return timeOfEnd; }
  void SetTimeOfEnd(unsigned long theTime) { timeOfEnd = theTime; }
  bool IsAnnouncedBegin() { return announcedBegin; }
  void SetAnnouncedBegin(bool a) { announcedBegin = a; }
  bool IsAnnouncedEnd() { return announcedEnd; }
  void SetAnnouncedEnd(bool a) { announcedEnd = a; }
  bool IsFulfilled() { return fulfilled; }
  void SetFulfilled(bool f) { fulfilled = f; }
  void Reset() { active = false; tType = TASK_GENERAL; message = ""; timeOfBegin = 0; timeOfEnd = 0; announcedBegin = false; announcedEnd = false; fulfilled = false; holderConfirmed = false; wearerConfirmed = false; }
  void Print();
  // Confirmation
  bool HasHolderConfirmed() { return holderConfirmed; }
  void SetHolderConfirmed(bool c) { holderConfirmed = c; }
  bool HasWearerConfirmed() { return wearerConfirmed; }
  void SetWearerConfirmed(bool c) { wearerConfirmed = c; }

  void Create(String msg) { SetActive(true); SetMessage(msg); }
  String GetStatusMessage();
  void Complete() { SetActive(false); SetFulfilled(true); }
};


class Tasklist
{
private:
  Task task[MAX_TASKS];

public:
  Tasklist() {}

  int GetActiveTaskCount() { int aCount = 0; for(int i = 0; i < MAX_TASKS; i++) { if (task[i].IsActive()) aCount++; } return aCount; }
  Task * GetTask(int i) { if (i < MAX_TASKS) return &task[i]; else return NULL; }
  int FindFreeIndex();

  void Init();
  void Reset();
  int Add(String msg, int tType=TASK_GENERAL, unsigned long tBegin=0, unsigned long tEnd=TIME_MAX);
  const String GetTaskList();
  void Print();
};


class VerificationEvent : public Task
{
private:
  bool isRandom = false;
  bool needsCode = false;
  String code;

public:
  VerificationEvent() {}
  bool IsRandom() { return isRandom; }
  void SetRandom(bool a) { isRandom = a; }
  bool NeedsCode() { return needsCode; }
  void SetNeedsCode(bool c) { needsCode = c; }
  String GetCode() { return code; }
  void SetCode(String c) { code = c; }
  void SetRandomCode() { String vCode = String(random(1000), DEC); while (vCode.length() < 3) vCode = "0" + vCode; SetCode(vCode); }
  void Reset() { Task::Reset(); isRandom = false; needsCode = false; }
  void Activate(unsigned long windowStart, unsigned long windowEnd);
  void Print();
};


class Verification
{
private:
  VerificationEvent event[MAX_VERIFICATIONS];
  bool enabled = true;
  bool needsSchedule = true;  // indicates that a new set of verification requests have to be scheduled (for the next day)
  bool dayIsCompleted = false;
  int requiredCountPerDay = 0;
  int currentIndex = 0;
  int dayOfWeek = 0;

public:
  void Init();
  bool IsEnabled() { return enabled; }
  void SetEnabled(bool e) { enabled = e; }
  bool NeedsSchedule() { return needsSchedule; }
  void SetNeedsSchedule(bool n) { needsSchedule = n; }
  void SetVerificationMode(bool onOff, int count=1);
  // only for settings exchange:
  void SetVerificationModeInt(int mode) { enabled = (mode > 0); requiredCountPerDay = mode; }
  int GetVerificationModeInt() { return enabled ? requiredCountPerDay : 0; }
  int GetRequiredCountPerDay() { return requiredCountPerDay; }
  void SetActualToday(int a);
  int GetActualToday();
  bool IsDayCompleted() { return dayIsCompleted; }
  void SetDayCompleted(bool i) { dayIsCompleted = i; needsSchedule = true; }
  //
  int GetCurrentIndex() { return currentIndex; }
  void SetCurrentIndex(int c) { currentIndex = c; }
  int GetDayOfWeek() { return dayOfWeek; }
  void SetDayOfWeek(int d) { dayOfWeek = d; }
  //
  VerificationEvent * GetEvent(int i) { if (i < MAX_VERIFICATIONS) return &event[i]; else return NULL; }
  VerificationEvent * GetCurrentEvent() { if (currentIndex < MAX_VERIFICATIONS) return &event[currentIndex]; else return NULL; }
  unsigned long GetTimeOfNextBegin() { return event[currentIndex].GetTimeOfBegin(); }
  unsigned long GetTimeOfNextEnd() { return event[currentIndex].GetTimeOfEnd(); }
  String GetHash();

  void Reset();
  void WindowCompleted();
  void Print();
  void Schedule(bool force=false);
  void CheckIn(String chatId);
  void ProcessVerification(String chatId);
};



class Session
{
private:
  String id;
  String chatId;
  bool randomShockMode = false;
  bool ramdomShockOffMessage30 = false;
  bool ramdomShockOffMessage10 = false;
  unsigned long randomShocksPerHour = 1;
  bool teasingMode = true;
  unsigned long awayCounter = 0;
//  int credits = 0;
//  int unlockVouchers = 0;
//  int creditFractions = 0;
  int deviations = 0;
  int failures = 0;
  unsigned int wearerPresence = 0;
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
  unsigned long timeOfRandomModeStart = 0;
  unsigned long timeOfLast5sInterval = 0;
  unsigned long timeOfLast1minInterval = 0;
  unsigned long timeOfLast5minInterval = 0;
  unsigned long lockTimerEnd = 0;
  bool lockTimerWasActive = false;

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
  bool IsActiveSession();

  unsigned long GetTimeOfLastUnlock() { return timeOfLastUnlock; }
  void SetTimeOfLastUnlock(unsigned long t) { timeOfLastUnlock = t; }

  unsigned long GetTimeOfLastOpening() { return timeOfLastOpening; }
  void SetTimeOfLastOpening(unsigned long t) { timeOfLastOpening = t; }

  unsigned long GetTimeOfLastClosing() { return timeOfLastClosing; }
  void SetTimeOfLastClosing(unsigned long t) { timeOfLastClosing = t; }

  unsigned long GetTimeOfLastShock() { return timeOfLastShock; }
  void SetTimeOfLastShock(unsigned long t) { timeOfLastShock = t; }

  unsigned long GetTimeOfRandomModeStart() { return timeOfRandomModeStart; }
  void SetTimeOfRandomModeStart(unsigned long t) { timeOfRandomModeStart = t; }

  unsigned long GetLockTimerEnd() { return lockTimerEnd; }
  void SetLockTimerEnd(unsigned long l) { lockTimerEnd = l; }
  bool AddLockTimerEnd(unsigned long l);
  void SubLockTimerEnd(unsigned long l);
  unsigned long GetLockTimerRemaining();
  bool IsLockTimerActive() { return (GetLockTimerRemaining() > 0); }

  void SetTimeOfLast5sInterval(unsigned long setTime) { timeOfLast5sInterval = setTime; }
  unsigned long GetTimeOfLast5sInterval() { return timeOfLast5sInterval; }
  void SetTimeOfLast1minInterval(unsigned long setTime) { timeOfLast1minInterval = setTime; }
  unsigned long GetTimeOfLast1minInterval() { return timeOfLast1minInterval; }
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
  void SetTeasingModeInt(int mode) { teasingMode = (mode > 0); }
  int GetTeasingModeInt() { return teasingMode ? 1 : 0; }
  int SetRandomMode(bool onOff, int shocksPerHour=1);
  bool IsRandomMode() { return randomShockMode; }
  int GetRandomModeShocksPerHour() { return randomShocksPerHour; }
  void SetRandomModeInt(int mode) { randomShockMode = (mode > 0); randomShocksPerHour = mode; }
  int GetRandomModeInt() { return randomShockMode ? randomShocksPerHour : 0; }

  void SetAwayCounter(unsigned long newVal) { awayCounter = newVal; }
  unsigned long GetAwayCounter() { return awayCounter; }
  void SetDeviations(int newVal) { deviations = newVal; }
  int GetDeviations() { return deviations; }
  void SetFailures(int newVal) { failures = newVal; }
  int GetFailures() { return failures; }

  void SetEmergencyReleaseCounter(int newVal) { emergencyReleaseCounter = newVal; emergencyReleaseCounterRequest = false; }
  int GetEmergencyReleaseCounter() { return emergencyReleaseCounter; }
  void SetEmergencyReleaseCounterRequest(bool req) { emergencyReleaseCounterRequest = req; if (! req) emergencyReleaseCounter = 0; }
  bool GetEmergencyReleaseCounterRequest() { return emergencyReleaseCounterRequest; }

  void Punishment(int level);
  void Shock(int count, long milliseconds, int level=30);
  void Lock();
  void ForcedUnlock();
  void Unlock();

  void ProcessRandomShocks();
  void ScheduleNextRandomShock();
};

#endif

//
