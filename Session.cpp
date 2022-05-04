#include "Defs.h"
#include "EServer.h"
#include "Session.h"
#include "Message.h"
#include "Oled.h"
#include "TimeF.h"
#include "User.h"
#include <ArduinoJson.h>


extern EmlaServer emlaServer;
extern TimeFunctions timeFunc;
extern UserSet users;
extern Message message;
extern OledDisplay oledDisplay;
extern Session session;
extern Verification verification;
extern Task tasks;



// ------------------------------------------------------------------------
String Task::GetStatusMessage()
{
  String statusStr;
  Serial.println(ESP.getFreeHeap());
  if (IsActive() && (! IsFulfilled()))
  {
    // active task
    if (timeFunc.GetTimeInSeconds() > GetTimeOfBegin())
    {
      // task is due
      if (timeFunc.GetTimeInSeconds() < GetTimeOfEnd())
      {
        // task is due
        statusStr = "due task";
      }
      else
      {
        // task is overdue
        statusStr = "overdue";
      }
    }
    else
    {
      // pending task
      statusStr = "pending";
    }
    Serial.println("  . return");
    return "[" + String(GetIndex(), DEC) + "]  " + GetMessage() + " (" + statusStr + ")\n";
  }
}


// ------------------------------------------------------------------------
void Task::Print()
{
  switch (tType)
  {
    default:
    case TASK_GENERAL:
      Serial.print("Task");
      break;
    case TASK_CONFIRMATION:
      Serial.print("Confirmation");
      break;
  }
  Serial.print(" index: " + String(GetIndex(), DEC));
  Serial.println("- message: " + GetMessage());
  if (IsActive())
    Serial.print(" - is active, ");
  else
    Serial.print(" - is not active, ");
  if (IsFulfilled())
    Serial.println("is fulfilled, ");
  else
    Serial.println("is not fulfilled, ");
  Serial.print("- begin: " + timeFunc.Time2StringNoDays(GetTimeOfBegin()));
  if (IsAnnouncedBegin())
    Serial.println(" (notified)");
  else
    Serial.println(" (not notified)");
  Serial.print("- end: " + timeFunc.Time2StringNoDays(GetTimeOfEnd()));
  if (IsAnnouncedEnd())
    Serial.println(" (notified)");
  else
    Serial.println(" (not notified)");
}


// ------------------------------------------------------------------------
void Tasklist::Init()
{
  Serial.println("*** Tasklist::Init()");
  for (int i = 0; i < MAX_TASKS; i++)
  {
    task[i].SetIndex(i);
    task[i].Reset();
  }
}


// ------------------------------------------------------------------------
void Tasklist::Reset()
{
  Serial.println("*** Tasklist::Reset()");
  for (int i = 0; i < MAX_TASKS; i++)
  {
    Serial.println("- index: " + String(i, DEC));
    task[i].Reset();
    task[i].SetAnnouncedBegin(false);
    if (task[i].IsAnnouncedBegin())
      Serial.println("- notification begin: true");
    else
      Serial.println("- notification begin: false");
    if (task[i].IsAnnouncedEnd())
      Serial.println("- notification end: true");
    else
      Serial.println("- notification end: false");
  }
}


// ------------------------------------------------------------------------
int Tasklist::FindFreeIndex()
{
  int i = 0;
  while (i < MAX_TASKS)
  {
    if (! task[i].IsActive())
      return i;
    i++;
  }
  return TASK_NONE;
}


// ------------------------------------------------------------------------
int Tasklist::Add(String msg, int tType, unsigned long tBegin, unsigned long tEnd)
{
  
}


// ------------------------------------------------------------------------
const String Tasklist::GetTaskList()
{
  String str = "List of open tasks:\n";
  /*
  Serial.println("*** Tasklist::GetTaskList()");
  for (int i = 0; i < MAX_TASKS; i++)
  {
    Serial.println("- index:" + String(i, DEC));
    str += task[i].GetStatusMessage();
  }
  Print();
*/

  return str;
}


// ------------------------------------------------------------------------
void Tasklist::Print()
{
  Serial.println("*** Tasklist::Print()");
  Serial.println("- active tasks: " + String(GetActiveTaskCount(), DEC));
  for (int i = 0; i < MAX_TASKS; i++)
  {
    task[i].Print();
  }
}


// ------------------------------------------------------------------------
void VerificationEvent::Activate(unsigned long windowStart, unsigned long windowEnd)
{
  SetRandom(true);
  SetNeedsCode(true);
  SetRandomCode();
  SetTimeOfBegin(windowStart);
  SetTimeOfEnd(windowEnd);
  Print();
}


// ------------------------------------------------------------------------
void VerificationEvent::Print()
{
  Serial.print("Verification event index: " + String(GetIndex(), DEC));
  if (IsFulfilled())
    Serial.print(" - is fulfilled, ");
  else
    Serial.print(" - is not fulfilled, ");
  if (IsRandom())
    Serial.println("random");
  else
    Serial.println("not random");
  Serial.print("- begin: " + timeFunc.Time2StringNoDays(GetTimeOfBegin()));
  if (IsAnnouncedBegin())
    Serial.println(" (notified)");
  else
    Serial.println(" (not notified)");
  Serial.print("- end: " + timeFunc.Time2StringNoDays(GetTimeOfEnd()));
  if (IsAnnouncedEnd())
    Serial.println(" (notified)");
  else
    Serial.println(" (not notified)");
  Serial.print("- code: " + GetCode());
  if (NeedsCode())
    Serial.println(" (needed)");
  else
    Serial.println(" (not needed)");
}


// ------------------------------------------------------------------------
void Verification::Init()
{
  Serial.println("*** Verification::Init()");
  for (int i = 0; i < MAX_VERIFICATIONS; i++)
  {
    event[i].SetIndex(i);
    event[i].Reset();
  }
  needsSchedule = true;
}


// ------------------------------------------------------------------------
void Verification::Reset()
{
  Serial.println("*** Verification::Reset()");
  currentIndex = 0;
  dayIsCompleted = false;
  String vCode;
  for (int i = 0; i < MAX_VERIFICATIONS; i++)
  {
    Serial.println("- index: " + String(i, DEC));
    event[i].Reset();
    event[i].SetAnnouncedBegin(false);
    if (event[i].IsAnnouncedBegin())
      Serial.println("- notification begin: true");
    else
      Serial.println("- notification begin: false");
    if (event[i].IsAnnouncedEnd())
      Serial.println("- notification end: true");
    else
      Serial.println("- notification end: false");
    event[i].SetRandomCode();
    Serial.println("- code: " + event[i].GetCode());
  }
}


// ------------------------------------------------------------------------
String Verification::GetHash()
{
  return String("864") + GetCurrentEvent()->GetCode() + timeFunc.Time2StringNoDaysCompact(GetCurrentEvent()->GetTimeOfBegin()) + timeFunc.Time2StringNoDaysCompact(GetCurrentEvent()->GetTimeOfEnd());
}


// ------------------------------------------------------------------------
void Verification::Print()
{
  Serial.println("*** Verification::Print()");
  if (IsEnabled())
    Serial.println("- is enabled");
  else
    Serial.println("- is disabled");
  if (IsDayCompleted())
    Serial.println("- day is completed");
  else
    Serial.println("- day is not completed");
  Serial.println("- current index: " + String(GetCurrentIndex(), DEC));
  Serial.println("- required count per day: " + String(GetRequiredCountPerDay(), DEC));
  Serial.println("- actual count per day: " + String(GetActualToday(), DEC));
  Serial.println("- day of week: " + String(GetDayOfWeek(), DEC));
  for (int i = 0; i < MAX_VERIFICATIONS; i++)
    event[i].Print();
}


// ------------------------------------------------------------------------
int Verification::GetActualToday()
{
  int c = 0;
  int i = 0;
  while (i < requiredCountPerDay)
  {
    if (event[i].IsFulfilled())
      c++;
    i++;
  }
  return c;
}


// ------------------------------------------------------------------------
void Verification::SetActualToday(int a)
{
  for (int i = 0; i < a; i++)
  {
    event[i].SetFulfilled(true);
  }
}


// ------------------------------------------------------------------------
void Verification::WindowCompleted()
{
  Serial.println("*** Verification::WindowCompleted()");
  Serial.println("- verification window: " + String(GetCurrentEvent()->GetIndex(), DEC));
  GetCurrentEvent()->SetAnnouncedBegin(true);
  GetCurrentEvent()->SetAnnouncedEnd(true);
  if (GetCurrentIndex() < GetRequiredCountPerDay())
  {
    SetCurrentIndex(GetCurrentIndex() + 1);
  }
  else
  {
    SetDayCompleted(true);
  }
}


// ------------------------------------------------------------------------
void Verification::SetVerificationMode(bool onOff, int count)
{
  Serial.println("*** Verification::SetVerificationMode()");
  Serial.print("- onOff: ");
  Serial.println(onOff);
  Serial.print("- count: ");
  Serial.println(count);
  SetEnabled(onOff);
  requiredCountPerDay = count;
  Schedule(true);
  message.WriteCommandsAndSettings("Verification-SetVerificationMode()");
}


// ------------------------------------------------------------------------
void Verification::Schedule(bool force)
{
  Serial.println("*** Verification::Schedule()");
  Serial.println("- verifications per day: " + String(GetRequiredCountPerDay(), DEC));
  unsigned long wakeUpTime = timeFunc.WakeUpTime();
  unsigned long sleepTime = timeFunc.SleepTime();
  unsigned long verificationWindow = 0;
  unsigned long windowBeginTime = 0;  // used to determine intermediate values when defining windows

  if (IsEnabled())
  {
    Serial.println("- IsEnabled");
    // schedule new verification events at 5:00
    if (force ||
        (needsSchedule && (timeFunc.GetHours() == 5)))
    {
      Serial.println("");
      // new day --> update day of week
      SetDayOfWeek(timeFunc.GetDayOfWeek());
      // reset all fulfillments first
      Reset();
      // check for the number of verifications
      switch (GetRequiredCountPerDay())
      {
      case 1:
        Serial.println("1:");
	      event[0].Activate(wakeUpTime, sleepTime);
        break;
      case 2:
        Serial.println("2:");
      	event[0].Activate(wakeUpTime, wakeUpTime + DURATION_EARLY_VERIFICATION);
      	event[1].Activate(sleepTime - DURATION_LATE_VERIFICATION, sleepTime);
        break;
      case 3:
        Serial.println("3:");
	      event[0].Activate(wakeUpTime, wakeUpTime + DURATION_EARLY_VERIFICATION);
	      verificationWindow = (sleepTime - DURATION_LATE_VERIFICATION) - (wakeUpTime + DURATION_EARLY_VERIFICATION) - 3600;
	      windowBeginTime = (wakeUpTime + DURATION_EARLY_VERIFICATION + random(verificationWindow*100) / 100) + 1800;
	      event[1].Activate(windowBeginTime, windowBeginTime + DURATION_RANDOM_VERIFICATION);
	      event[2].Activate(sleepTime - DURATION_LATE_VERIFICATION, sleepTime);
        break;
      case 4:
        Serial.println("4:");
	      event[0].Activate(wakeUpTime, wakeUpTime + DURATION_EARLY_VERIFICATION);
        verificationWindow = ((sleepTime - DURATION_LATE_VERIFICATION) - (wakeUpTime + DURATION_EARLY_VERIFICATION)) / 2 - 1800;
	      windowBeginTime = (wakeUpTime + DURATION_EARLY_VERIFICATION + random(verificationWindow*100) / 100) + 900;
	      event[1].Activate(windowBeginTime, windowBeginTime + DURATION_RANDOM_VERIFICATION);
	      windowBeginTime = (event[1].GetTimeOfEnd() + random(verificationWindow*100) / 100) + 900;
	      event[2].Activate(windowBeginTime, windowBeginTime + DURATION_RANDOM_VERIFICATION);
	      event[3].Activate(sleepTime - DURATION_LATE_VERIFICATION, sleepTime);
        break;
      case 0:
      default:
        Serial.println("0:");
        break;
      }
      needsSchedule = false;
      message.WriteCommandsAndSettings("Verification-Schedule()");
      Print();
    }
  }
}


// ------------------------------------------------------------------------
void Verification::CheckIn(String chatId)
{
  Serial.println("*** CheckVerification()");
  Print();
  if (IsEnabled())
  {
    Serial.println("- verification mode: true");
    if (timeFunc.GetTimeInSeconds() > GetTimeOfNextBegin())
    {
      // 60 seconds bonus time on check
      if (timeFunc.GetTimeInSeconds() < GetTimeOfNextEnd() + VERIFICATION_BONUS_TIME)
      {
        // verification is accepted, it comes in time
        GetCurrentEvent()->SetFulfilled(true);
        message.SendMessage("Verification accepted " + timeFunc.Time2String(GetTimeOfNextEnd() - timeFunc.GetTimeInSeconds() + VERIFICATION_BONUS_TIME) + " before the deadline.", chatId);
        WindowCompleted();
        Serial.print("- actual verifications after check-in: ");
        Serial.println(GetActualToday());
      }
      else
      {
        // verification is late
        session.SetDeviations(session.GetDeviations() + 1);
        message.SendMessage(SYMBOL_DEVIL_ANGRY " Verification is not acceptable. It comes late.", chatId);
      }
    }
    else
    {
      // verification comes early
      session.SetDeviations(session.GetDeviations() + 1);
      message.SendMessage(SYMBOL_DEVIL_SMILE " Verification is not acceptable. It comes early.", chatId);
    }
    message.WriteCommandsAndSettings("Verification-CheckIn()");
  }
}


// ------------------------------------------------------------------------
void Verification::ProcessVerification(String chatId)
{
  Serial.println("*** Session::ProcessVerification()");
  Print();
  if (IsEnabled() && session.IsActiveSession() && (! IsDayCompleted()))
  {
    // verification is enabled today
    Serial.println("- (IsEnabled() && session.IsActiveSession() && (! IsDayCompleted()))");
    if (GetCurrentEvent()->IsFulfilled())
    {
      Serial.println("- (GetCurrentEvent()->IsFulfilled())");
      WindowCompleted();
    }
    else if (timeFunc.GetTimeInSeconds() > GetTimeOfNextEnd())
    {
      // the end of the current period has arrived
      Serial.println("- (timeFunc.GetTimeInSeconds() > GetTimeOfNextEnd())");
      if (! GetCurrentEvent()->IsAnnouncedEnd())
      {
        Serial.println("- (! GetCurrentEvent()->IsAnnouncedEnd())");
        // check, if we are just starting and already beyond the verification window
        if (! GetCurrentEvent()->IsAnnouncedBegin())
        {
          Serial.println("- verification window " + String(GetCurrentEvent()->GetIndex(), DEC) + " is skipped");
          WindowCompleted();
        }
        else
        {
          Serial.println("- verification window " + String(GetCurrentEvent()->GetIndex(), DEC) + " ends");
          // angry smiling symbols
          session.SetFailures(session.GetFailures() + 1);
          message.SendMessage(SYMBOL_DEVIL_ANGRY SYMBOL_DEVIL_ANGRY SYMBOL_DEVIL_ANGRY " Verification request has expired!!! Wearer " + users.GetWearer()->GetName() + " has failed to provide a verification in time!", chatId);
          WindowCompleted();
          message.WriteCommandsAndSettings("Verification-ProcessVerification() verification expired");
        }
      }
    }
    else if (timeFunc.GetTimeInSeconds() > GetTimeOfNextBegin())
    {
      Serial.println("- (timeFunc.GetTimeInSeconds() > GetTimeOfNextBegin())");
      if (! GetCurrentEvent()->IsAnnouncedBegin())
      {
        Serial.println("- verification window " + String(GetCurrentEvent()->GetIndex(), DEC) + " begins");
        // devil smiling symbol
        message.SendMessage(SYMBOL_DEVIL_SMILE " Verification request - wearer " + users.GetWearer()->GetName() + " must provide a verification showing the verification code " + GetCurrentEvent()->GetCode() + " within " + 
                            timeFunc.Time2String(GetTimeOfNextEnd() - timeFunc.GetTimeInSeconds()) + " from now!", chatId);
        GetCurrentEvent()->SetAnnouncedBegin(true);
        message.WriteCommandsAndSettings("Verification-ProcessVerification() verification request");
      }
    }
  }
}



// ------------------------------------------------------------------------
void Session::Shock(int count, long milliseconds, int level)
{
  Serial.print("*** Shock() milliseconds=");
  Serial.print(milliseconds);
  Serial.print(", count=");
  Serial.print(count);
  String prefix = "#=" + String(count, DEC) + " t=" + String((milliseconds / 1000), DEC) + " l=" + String(level, DEC);

  // IFTTT webhook handling
  {
    String payload;
    String request = "value1=" + String(count, DEC) + "&value2=" + String(milliseconds, DEC) + "," + String(level, DEC) + "&value3=Telegram";
    Serial.print("*** IFTTT webhook");
    Serial.println(request);

    int trial = 0;
    bool success = false;
    while ((trial < 10) && ! success)
    {
      success = emlaServer.WPost("https://maker.ifttt.com/trigger/s_received/with/key/" IFTTT_API_KEY, request, payload, true);
      trial++;
      if (trial > 1)
      {
        Serial.print("Retry: ");
        Serial.println(trial);
      }
      if (! success)
        delay(1000);
    }
  }

  int firstBurst = 0;
  long now = timeFunc.GetTimeInSeconds();
  long delivered = 0;

  Serial.print("last shock ago: ");
  Serial.println(now - timeOfLastShock);
  if ((now - timeOfLastShock) > 250)
    firstBurst = 2500;

  int shockPin = SHOCK_PIN;
  bool turnOn, turnOff;
  level = 10;
  if (level < 20)
  {
    shockPin = SHOCK1_PIN;
    turnOn = LOW;
    turnOff = HIGH;
  }
  else if (level < 40)
  {
    shockPin = SHOCK_PIN;
    turnOn = HIGH;
    turnOff = LOW;
  }
  else if (level < 40)
  {
    shockPin = SHOCK2_PIN;
    turnOn = LOW;
    turnOff = HIGH;
  }
  else if (level < 40)
  {
    shockPin = SHOCK3_PIN;
    turnOn = LOW;
    turnOff = HIGH;
  }
  else
  {
    shockPin = SHOCK4_PIN;
    turnOn = LOW;
    turnOff = HIGH;
  }

  for (int i = 0; i < count; i++)
  {
    Serial.println("*** Shock processing: ");
    Serial.print(i);
    Serial.print(", ");
    Serial.print(milliseconds);
    Serial.println(" ms");

//    digitalWrite(shockPin, turnOn);
    digitalWrite(4, HIGH);
    delay(100);
//    digitalWrite(shockPin, turnOff);
    digitalWrite(4, LOW);
    delay(200);

//    digitalWrite(shockPin, turnOn);
    digitalWrite(4, HIGH);
    delay(firstBurst);
//    digitalWrite(shockPin, turnOff);
    digitalWrite(4, LOW);
    delay(50);

    while(milliseconds > delivered)
    {
//    digitalWrite(shockPin, turnOn);
      digitalWrite(4, HIGH);
      if ((milliseconds - delivered) > 10000L)
      {
        delay(10000L);
        delivered += 10000L;
      }
      else
      {
        delay(milliseconds - delivered);
        delivered += (milliseconds - delivered);
      }
//    digitalWrite(shockPin, turnOff);
      digitalWrite(4, LOW);
      delay(50);
    }

    delay(SHOCK_BREAK_DURATION);
  }

  timeOfLastShock = now;
}



// ------------------------------------------------------------------------
void Session::Lock()
{
}



// ------------------------------------------------------------------------
void Session::ForcedUnlock()
{
  oledDisplay.PrintDisplay("Unlock GRANTED!");
  ledcAttachPin(SERVO_PIN, 1);
  delay(50);
  ledcWrite(1, 2999);
//    myservo.write(90);
  delay(UNLOCK_DURATION);
//    myservo.write(0);
  ledcWrite(1, 999);
  delay(500);
  ledcDetachPin(SERVO_PIN);
}



// ------------------------------------------------------------------------
void Session::Unlock()
{
  ForcedUnlock();
}


//  TRANSITIONS
//             Holder    Teaser    Guest     Wearer
//  Holder       +         +         +
//  Teaser       +         +         +
//  Guest        +         +         +
//  Wearer
//
//  Wearer/free  
//
//



// ------------------------------------------------------------------------
unsigned long Session::GetRemainingTime(bool forDisplay)
{
  unsigned long now = timeFunc.GetTimeInSeconds();

  if (forDisplay)
    return endTime - now;
  else if (endTime == 0)
    return 1;
  else if (endTime < now)
    return 1;
  else
    return endTime - now;
}


// ------------------------------------------------------------------------
unsigned long Session::GetLockTimerRemaining()
{
  if (timeFunc.GetTimeInSeconds() > lockTimerEnd)
    return 0;
  else
    return lockTimerEnd - timeFunc.GetTimeInSeconds();
}


// ------------------------------------------------------------------------
bool Session::AddLockTimerEnd(unsigned long lockTime)
{
  bool limitReached = false;
  // limit extension to 7 days
  if (lockTime > 7*86400L)
  {
    lockTime = 7*86400L;
    limitReached = true;
  }

  if (IsLockTimerActive())
  {
    // lock timer may not exceed 7 days
    if ((GetLockTimerEnd() + lockTime) > (timeFunc.GetTimeInSeconds() + 7*86400L))
    {
      SetLockTimerEnd(timeFunc.GetTimeInSeconds() + 7*86400L);
      limitReached = true;
    }
    else
      SetLockTimerEnd(GetLockTimerEnd() + lockTime);
  }
  else
    SetLockTimerEnd(timeFunc.GetTimeInSeconds() + lockTime);

  return limitReached;
}


// ------------------------------------------------------------------------
void Session::SubLockTimerEnd(unsigned long lockTime)
{
  if (IsLockTimerActive())
  {
    if (GetLockTimerEnd() > lockTime)
      SetLockTimerEnd(GetLockTimerEnd() - lockTime);
    else
      // we want the lockTimerEnd to be set to the current time if cleared to avoid underrun issues
      SetLockTimerEnd(timeFunc.GetTimeInSeconds());
  }
  else
    SetLockTimerEnd(timeFunc.GetTimeInSeconds() - lockTime);
}


// ------------------------------------------------------------------------
bool Session::IsActiveSession()
{
  return (! users.GetWearer()->IsFreeWearer());
}


// ------------------------------------------------------------------------
void Session::Punishment(int level)
{
  message.SendMessage("Wearer " + users.GetWearer()->GetName() + "needs to be treated.", chatId);
  unsigned long rnd = random(10000);
  switch (rnd * level / 10000)
  {
    case 0:
      Shock(1, 1000);
      break;
    case 1:
      Shock(1, 3000);
      break;
    case 2:
      Shock(1, 5000);
      break;
    case 3:
      Shock(3, 1000);
      break;
    case 4:
      Shock(3, 3000);
      break;
    case 5:
      Shock(3, 5000);
      break;
    case 6:
      Shock(5, 3000);
      break;
    case 7:
      Shock(5, 5000);
      break;
    case 8:
      Shock(10, 5000);
      break;
    case 9:
      Shock(10, 10000);
      break;
  }
}


// ------------------------------------------------------------------------
int Session::SetRandomMode(bool onOff, int shocksPerHour)
{
  Serial.println("*** Session::SetRandomMode()");
  Serial.print("- onOff: ");
  Serial.println(onOff);
  Serial.print("- shocksPerHour: ");
  Serial.println(shocksPerHour);
  unsigned long now = timeFunc.GetTimeInSeconds();
  unsigned long duration = now - GetTimeOfRandomModeStart();
  // credit is given for each hours minus 5 seconds in order to prevent small deviations from the 1 hour duration
//  int creditIncrement = duration / 3595;
  bool oldRandomShockMode = randomShockMode;
  randomShockMode = onOff;

  if (randomShockMode)
  {
    // randomShocksPerHour must be greater zero!
    if (shocksPerHour > 0)
      randomShocksPerHour = shocksPerHour;
    else
      randomShocksPerHour = 1;
    ramdomShockOffMessage10 = false;
    ramdomShockOffMessage30 = false;
    SetTimeOfRandomModeStart(now);
    ScheduleNextRandomShock();
  }
  else if (oldRandomShockMode)
  {
    // if randomShockMode is switched off, but oldRandomShockMode was on...
//    Serial.print("- credit increment: ");
//    Serial.println(creditIncrement);
//    SetCredits(GetCredits() + creditIncrement);
  }

  message.WriteCommandsAndSettings("Session-SetRandomMode()");

//  return creditIncrement;
  return 0;
}


// ------------------------------------------------------------------------
void Session::ScheduleNextRandomShock()
{
  Serial.println("*** Session::ScheduleNextRandomShock()");
  Serial.print("- randomShockMode: ");
  Serial.println(randomShockMode);
  Serial.print("- IsSleeping: ");
  Serial.println(users.GetWearer()->IsSleeping());
  if (users.GetWearer()->IsSleeping())
  {
    randomShockMode = false;
  }
  else
  {
    // 
    timeOfNextScheduledShock = timeFunc.GetTimeInSeconds() + random(86400*100) / (1200 * randomShocksPerHour);
    if (timeOfNextScheduledShock > 60)
      timeOfNextScheduledShock = timeOfNextScheduledShock - 15;
    Serial.print("- timeOfNextScheduledShock: ");
    Serial.println(timeOfNextScheduledShock);
  }
}


// ------------------------------------------------------------------------
void Session::ProcessRandomShocks()
{
  Serial.println("*** Session::ProcessRandomShocks()");
  Serial.print("- randomShockMode: ");
  Serial.println(randomShockMode);
  if (randomShockMode)
  {
    // process auto-off for random shocks
    unsigned long duration = timeFunc.GetTimeInSeconds() - GetTimeOfRandomModeStart();
    // auto-off random mode after 3 hours/RANDOM_SHOCK_AUTO_OFF_SECONDS
    if (duration > RANDOM_SHOCK_AUTO_OFF_SECONDS)
    {
      message.SendMessage("Random mode will be turned off now, because it ran for " + String(RANDOM_SHOCK_AUTO_OFF_SECONDS/3600, DEC) + " hours.");
      randomShockMode = false;
      message.WriteCommandsAndSettings("Session-ProcessRandomShocks()");
      message.RandomShockModeAction("off", USER_ID_BOT, GROUP_CHAT_ID, FORCE);
      return;
    }
    // message 10 minutes before auto-off
    else if ((duration > (RANDOM_SHOCK_AUTO_OFF_SECONDS - 600)) &&
             ( ! ramdomShockOffMessage10))
    {
      message.SendMessage("Random mode will be turned off automatically in 10 minutes. By triggering the random mode again, it will run again for " + String(RANDOM_SHOCK_AUTO_OFF_SECONDS/3600, DEC) + " hours before auto-off.");
      ramdomShockOffMessage10 = true;
    }
    // message 30 minutes before auto-off
    else if ((duration > (RANDOM_SHOCK_AUTO_OFF_SECONDS - 1800)) &&
             ( ! ramdomShockOffMessage30))
    {
      message.SendMessage("Random mode will be turned off automatically in 30 minutes. By triggering the random mode again, it will run again for " + String(RANDOM_SHOCK_AUTO_OFF_SECONDS/3600, DEC) + " hours before auto-off.");
      ramdomShockOffMessage30 = true;
    }

    // honor need for sleep
    Serial.print("- IsSleeping: ");
    Serial.println(users.GetWearer()->IsSleeping());
    if (users.GetWearer()->IsSleeping())
    {
      // turn off random shock mode during sleeping time
      message.SendMessage("Wearer is allowed to sleep now.");
      message.RandomShockModeAction("off", USER_ID_BOT, GROUP_CHAT_ID, FORCE);
      return;
    }

    Serial.print("- timeOfNextScheduledShock: ");
    Serial.println(timeOfNextScheduledShock);
    Serial.print("- GetTimeInSeconds(): ");
    Serial.println(timeFunc.GetTimeInSeconds());
    if (timeOfNextScheduledShock < timeFunc.GetTimeInSeconds())
    {
      // execute random shock
      unsigned long rnd = (random(9999) / 1000) + 1;
      message.ShockAction(rnd * 1000, 1, users.GetBot()->GetId());
      // and schedule the next one
      ScheduleNextRandomShock();
    }
  }
}


//
