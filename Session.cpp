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



// ------------------------------------------------------------------------
void Verification::Init()
{
  Serial.println("*** Verification::Init()");
  for (int i = 0; i < MAX_VERIFICATIONS; i++)
  {
    event[i].SetIndex(i);
    event[i].Reset();
  }
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
  message.WriteCommandsAndSettings();
}


// ------------------------------------------------------------------------
void Verification::Schedule()
{
  Serial.println("*** Verification::Schedule()");
  Serial.println("- verifications per day: " + String(GetRequiredCountPerDay(), DEC));
  unsigned long wakeUpTime = timeFunc.WakeUpTime();
  unsigned long sleepTime = timeFunc.SleepTime();
  unsigned long verificationWindow = 0;

  if (IsEnabled())
  {
    // schedule new verification events if there is a new day
    if (timeFunc.GetDayOfWeek() != GetDayOfWeek())
    {
      // new day --> update day of week
      SetDayOfWeek(timeFunc.GetDayOfWeek());
      // reset all fulfillments first
      Reset();
      // check for the number of verifications
      switch (GetRequiredCountPerDay())
      {
      case 1:
        Serial.println("1:");
        event[0].SetTimeOfBegin(wakeUpTime);
        event[0].SetTimeOfEnd(sleepTime);
        break;
      case 2:
        Serial.println("2:");
        event[0].SetTimeOfBegin(wakeUpTime);
        event[0].SetTimeOfEnd(wakeUpTime + DURATION_EARLY_VERIFICATION);
        event[1].SetTimeOfBegin(sleepTime - DURATION_LATE_VERIFICATION);
        event[1].SetTimeOfEnd(sleepTime);
        break;
      case 3:
        Serial.println("3:");
        event[0].SetTimeOfBegin(wakeUpTime);
        event[0].SetTimeOfEnd(wakeUpTime + DURATION_EARLY_VERIFICATION);
        // determine random verification window
        verificationWindow = (sleepTime - DURATION_LATE_VERIFICATION) - (wakeUpTime + DURATION_EARLY_VERIFICATION) - 3600;
        event[1].SetTimeOfBegin((wakeUpTime + DURATION_EARLY_VERIFICATION + random(verificationWindow*100) / 100) + 1800);
        event[1].SetTimeOfEnd(event[1].GetTimeOfBegin() + DURATION_RANDOM_VERIFICATION);
        event[2].SetTimeOfBegin(sleepTime - DURATION_LATE_VERIFICATION);
        event[2].SetTimeOfEnd(sleepTime);
        break;
      case 4:
        Serial.println("4:");
        event[0].SetTimeOfBegin(wakeUpTime);
        event[0].SetTimeOfEnd(wakeUpTime + DURATION_EARLY_VERIFICATION);
        // determine random verification window 1
        verificationWindow = ((sleepTime - DURATION_LATE_VERIFICATION) - (wakeUpTime + DURATION_EARLY_VERIFICATION)) / 2 - 1800;
        event[1].SetTimeOfBegin((wakeUpTime + DURATION_EARLY_VERIFICATION + random(verificationWindow*100) / 100) + 900);
        event[1].SetTimeOfEnd(event[1].GetTimeOfBegin() + DURATION_RANDOM_VERIFICATION);
        // determine random verification window 2
        event[2].SetTimeOfBegin((event[1].GetTimeOfEnd() + random(verificationWindow*100) / 100) + 900);
        event[2].SetTimeOfEnd(event[2].GetTimeOfBegin() + DURATION_RANDOM_VERIFICATION);
        event[3].SetTimeOfBegin(sleepTime - DURATION_LATE_VERIFICATION);
        event[3].SetTimeOfEnd(sleepTime);
        break;
      case 0:
      default:
        Serial.println("0:");
        break;
      }
      message.WriteCommandsAndSettings();
      for (int i = 0; i < MAX_VERIFICATIONS; i++)
      {
        Serial.println("- timeOfNextVerificationBegin " + String(i, DEC) + ": " + timeFunc.Time2StringNoDays(event[i].GetTimeOfBegin()));
        Serial.println("- timeOfNextVerificationEnd " + String(i, DEC) + ": " + timeFunc.Time2StringNoDays(event[i].GetTimeOfEnd()));
      }
    }
  }
}


// ------------------------------------------------------------------------
void Verification::CheckIn(String chatId)
{
  Serial.println("*** CheckVerification()");
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
    message.WriteCommandsAndSettings();
  }
}


// ------------------------------------------------------------------------
void Verification::ProcessVerification(String chatId)
{
  Serial.println("*** Session::ProcessVerification()");
  Serial.println("- verification mode: " + String(IsEnabled(), DEC));
  Serial.println("- verification index: " + String(GetCurrentIndex(), DEC));
  Serial.println("- begin: " + timeFunc.Time2StringNoDays(GetCurrentEvent()->GetTimeOfBegin()));
  Serial.println("- end: " + timeFunc.Time2StringNoDays(GetCurrentEvent()->GetTimeOfEnd()));
  if (IsEnabled() && session.IsActiveSession() && (! IsDayCompleted()))
  {
    if (! GetCurrentEvent()->IsFulfilled())
    {
      if (timeFunc.GetTimeInSeconds() > GetTimeOfNextEnd())
      {
        if (! GetCurrentEvent()->IsAnnouncedEnd())
        {
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
            message.WriteCommandsAndSettings();
          }
        }
      }
      else if (timeFunc.GetTimeInSeconds() > GetTimeOfNextBegin())
      {
        if (! GetCurrentEvent()->IsAnnouncedBegin())
        {
          Serial.println("- verification window " + String(GetCurrentEvent()->GetIndex(), DEC) + " begins");
          // devil smiling symbol
          message.SendMessage(SYMBOL_DEVIL_SMILE " Verification request - wearer " + users.GetWearer()->GetName() + " must provide a verification within " + 
                              timeFunc.Time2String(GetTimeOfNextEnd() - GetTimeOfNextBegin()) + " from now!", chatId);
          verification.GetCurrentEvent()->SetAnnouncedBegin(true);
          message.WriteCommandsAndSettings();
        }
      }
    }
  }
}



// ------------------------------------------------------------------------
void Session::Shock(int count, long milliseconds)
{
  Serial.print("*** Shock() milliseconds=");
  Serial.print(milliseconds);
  Serial.print(", count=");
  Serial.print(count);
  String prefix = "#=" + String(count, DEC) + " t=" + String((milliseconds / 1000), DEC);

  // IFTTT webhook handling
  {
    String payload;
    String request = "value1=" + String(count, DEC) + "&value2=" + String(milliseconds, DEC) + "&value3=Telegram";
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

  Serial.print("last shock ago: ");
  Serial.println(now - timeOfLastShock);
  if ((now - timeOfLastShock) > 250)
    firstBurst = 2500;

  for (int i = 0; i < count; i++)
  {
    Serial.println("*** Shock processing: ");
    Serial.print(i);
    Serial.print(", ");
    Serial.print(milliseconds);
    Serial.println(" ms");

    digitalWrite(SHOCK_PIN, HIGH);
    delay(100);
    digitalWrite(SHOCK_PIN, LOW);
    delay(200);

    digitalWrite(SHOCK_PIN, HIGH);
    delay(milliseconds + firstBurst);
    digitalWrite(SHOCK_PIN, LOW);

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
void Session::SetCredits(int newVal, String chatId)
{
  int creditCount = GetCredits();
  SetCredits(newVal);
  if (GetCredits() > creditCount)
    message.SendMessage(String(SYMBOL_CREDIT) + " Wearer received " + (GetCredits() > creditCount) + " credits.", chatId);
}


// ------------------------------------------------------------------------
void Session::SetCreditFractions(int newVal)
{
  creditFractions = newVal;
  if (creditFractions >= 10)
  {
    int newCredits = creditFractions / 10;
    creditFractions = creditFractions % 10;
    SetCredits(GetCredits() + newCredits);
  }
}


// ------------------------------------------------------------------------
void Session::SetCreditFractions(int newVal, String chatId)
{
  int creditCount = GetCredits();
  SetCreditFractions(newVal);
  if (GetCredits() > creditCount)
    message.SendMessage(String(SYMBOL_CREDIT) + " Wearer received " + (GetCredits() > creditCount) + " credits.", chatId);
}


// ------------------------------------------------------------------------
bool Session::IsActiveSession()
{
  return (! users.GetWearer()->IsFreeWearer()) || activeChastikeySession;
}


// ------------------------------------------------------------------------
void Session::InfoChastikey()
{
  String payload;

  Serial.print("*** Session::InfoChastikey()");
/*
  int trial = 0;
  bool success = false;
  while ((trial < 10) && ! success)
  {
    success = emlaServer.WGet("https://api.chastikey.com/v0.3/listlocks.php?username=cblock", payload);
    trial++;
    if (trial > 1)
    {
      Serial.print("Retry: ");
      Serial.println(trial);
    }
    if (! success)
      delay(1000);
  }
  //  Serial.println(payload);

  if (payload.length() > 0)
  {
    // Extract values
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 10*JSON_OBJECT_SIZE(7) + 1260;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, payload);

    JsonObject response_0 = doc["response"][0];
    int response_0_status = response_0["status"]; // 200
    const char* response_0_message = response_0["message"]; // "Success"
    long response_0_timestampGenerated = response_0["timestampGenerated"]; // 1569859752

    JsonArray locks = doc["locks"];

    SetActiveChastikeySession(false);
    SetChastikeyHolder("");
    for (int i = 0; i < 25; i++)
    {
      JsonObject lockInfo = locks[i];
      String lockStatus = lockInfo["status"]; // "UnlockedReal"
      Serial.print("- Chastikey lock ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(lockStatus);
      if (lockStatus == "Locked")
      {
        SetActiveChastikeySession(true);
        SetChastikeyHolder(lockInfo["lockedBy"]);
      }
//      const char* locks_combination = locks["combination"]; // "84725435"
    }
  }
  Serial.print("- Chastikey session: ");
  Serial.println(IsActiveChastikeySession());
  */
}


// ------------------------------------------------------------------------
void Session::Punishment(int level)
{
  message.SendMessage("Wearer " + users.GetWearer()->GetName() + "needs to be punished.", chatId);
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
  int creditIncrement = duration / 3595;
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
    Serial.print("- credit increment: ");
    Serial.println(creditIncrement);
    SetCredits(GetCredits() + creditIncrement);
  }

  message.WriteCommandsAndSettings();

  return creditIncrement;
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
