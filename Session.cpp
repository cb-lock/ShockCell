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
bool Session::IsActiveSession()
{
  return (! users.GetWearer()->IsFreeWearer()) || activeChastikeySession;
}


// ------------------------------------------------------------------------
void Session::InfoChastikey()
{
  String payload;

  Serial.print("*** Session::InfoChastikey()");

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
int Session::SetVerificationMode(bool onOff, int count)
{
  Serial.println("*** Session::SetVerificationMode()");
  Serial.print("- onOff: ");
  Serial.println(onOff);
  Serial.print("- count: ");
  Serial.println(count);

  verificationMode = onOff;
  verificationsPerDay = count;
  message.WriteCommandsAndSettings();
}


// ------------------------------------------------------------------------
void Session::ScheduleNextVerification()
{
  Serial.println("*** ScheduleNextVerification()");
  Serial.print("- verifications per day: ");
  Serial.println(GetVerificationCountPerDay());
  // postpone scheduling until tomorrow if today's verifications have been completed
  if (GetVerificationsToday() < GetVerificationCountPerDay())
  {
    switch (GetVerificationCountPerDay())
    {
      case 0:
      case 1:
        Serial.println("0, 1:");
        timeOfNextVerificationBegin = timeFunc.WakeUpTime();
        timeOfNextVerificationEnd = timeFunc.SleepTime();
        verificationStatus = VERIFICATION_STATUS_BEFORE;
        break;
      case 2:
        Serial.println("2:");
        if ((GetVerificationsToday() >= 1) ||
            (timeFunc.GetTimeInSeconds() >= timeFunc.SleepTime() + DURATION_EARLY_VERIFICATION))
        {
          // late verification
          timeOfNextVerificationBegin = timeFunc.SleepTime() - DURATION_LATE_VERIFICATION;
          timeOfNextVerificationEnd = timeFunc.SleepTime();
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
        else
        {
          // early verification
          timeOfNextVerificationBegin = timeFunc.WakeUpTime();
          timeOfNextVerificationEnd = timeFunc.WakeUpTime() + DURATION_EARLY_VERIFICATION;
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
        break;
      case 3:
      default:
        Serial.println("3:");
        if ((GetVerificationsToday() >= 2) ||
            (timeFunc.GetTimeInSeconds() >= timeFunc.SleepTime() - DURATION_LATE_VERIFICATION))
        {
          // late verification
          timeOfNextVerificationBegin = timeFunc.SleepTime() - DURATION_LATE_VERIFICATION;
          timeOfNextVerificationEnd = timeFunc.SleepTime();
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
        else if ((GetVerificationsToday() >= 1) ||
                 (timeFunc.GetTimeInSeconds() >= timeFunc.SleepTime() + DURATION_EARLY_VERIFICATION))
        {
          // random verification
          unsigned long verificationWindow = (timeFunc.SleepTime() - DURATION_LATE_VERIFICATION) - (timeFunc.WakeUpTime() + DURATION_EARLY_VERIFICATION) - 3600;
          timeOfNextVerificationBegin = (timeFunc.GetTimeInSeconds() + random(verificationWindow*100) / 100) + 1800;
          // 15 minute window
          timeOfNextVerificationEnd = timeOfNextVerificationBegin + 900;
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
        else
        {
          // early verification
          timeOfNextVerificationBegin = timeFunc.WakeUpTime();
          timeOfNextVerificationEnd = timeFunc.WakeUpTime() + DURATION_EARLY_VERIFICATION;
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
        break;
    }
  }
}


// ------------------------------------------------------------------------
void Session::CheckVerification()
{
  Serial.println("*** CheckVerification()");
  if (verificationMode)
  {
    Serial.println("- verification mode: true");
    if (timeFunc.GetTimeInSeconds() > timeOfNextVerificationBegin)
    {
      if (timeFunc.GetTimeInSeconds() < timeOfNextVerificationEnd)
      {
        // verification is accepted, it comes in time
        message.SendMessage("Verification accepted " + timeFunc.Time2String(timeOfNextVerificationEnd - timeFunc.GetTimeInSeconds()) + " before the deadline.");
        Serial.print("- actual verifications before check-in: ");
        Serial.println(GetVerificationsToday());
        SetVerificationsToday(GetVerificationsToday() + 1);
        Serial.print("- actual verifications after check-in: ");
        Serial.println(GetVerificationsToday());
      }
      else
      {
        // verification is late
        message.SendMessage("Verification is not acceptable. It comes late by " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - timeOfNextVerificationEnd) + ".");
      }
    }
    else
    {
      // verification comes early
      message.SendMessage("Verification is not acceptable. It comes early by " + timeFunc.Time2String(timeOfNextVerificationBegin - timeFunc.GetTimeInSeconds()) + ".");
    }
  }
}


// ------------------------------------------------------------------------
void Session::ProcessVerification()
{
  Serial.println("*** Session::ProcessVerification()");
  Serial.print("- verificationMode: ");
  Serial.println(verificationMode);
  if (verificationMode && IsActiveSession())
  {
    // honor need for sleep
    Serial.print("- IsSleeping: ");
    Serial.println(users.GetWearer()->IsSleeping());

    // schedule first verification at 6:00
    if ((timeFunc.GetHours() == 6) && (timeFunc.GetMinutes() < 6))
    {
      Serial.println("- schedule first verification");
      SetVerificationsToday(0);
      ScheduleNextVerification();
    }
    else if (! users.GetWearer()->IsSleeping())
    {
      Serial.println("- wearer is awake");
      if (timeFunc.GetTimeInSeconds() > timeOfNextVerificationEnd)
      {
        Serial.println("- verification window ends");
        if (verificationStatus == VERIFICATION_STATUS_WAITING)
        {
          // angry smiling symbols
          message.SendMessage("\xf0\x9f\x98\xa1\xf0\x9f\x98\xa1\xf0\x9f\x98\xa1 Verification request has expired!!! Wearer " + users.GetWearer()->GetName() + " has failed to provide a verification in time!");
          ScheduleNextVerification();
          verificationStatus = VERIFICATION_STATUS_BEFORE;
        }
      }
      else if (timeFunc.GetTimeInSeconds() > timeOfNextVerificationBegin)
      {
        Serial.println("- verification window begins");
        // devil smiling symbol
        if (verificationStatus != VERIFICATION_STATUS_WAITING)
          message.SendMessage("\xf0\x9f\x98\x88 Verification request - wearer " + users.GetWearer()->GetName() + " must provide a verification within " + 
                              timeFunc.Time2String(timeOfNextVerificationEnd - timeOfNextVerificationBegin) + " from now!");
        verificationStatus = VERIFICATION_STATUS_WAITING;
      }
    }
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
