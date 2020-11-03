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
      success = emlaServer.WPost("https://maker.ifttt.com/trigger/s_received/with/key/2ONo1D18q28kdbCU-6zNT", request, payload, true);
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
    firstBurst = 2000;

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
  switch (rnd % level)
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
void Session::SetRandomMode(bool onOff, int shocksPerHour)
{
  Serial.println("*** Session::SetRandomMode()");
  Serial.print("- onOff: ");
  Serial.println(onOff);
  Serial.print("- shocksPerHour: ");
  Serial.println(shocksPerHour);
  randomShockMode = onOff;
  // randomShocksPerHour must be greater zero!
  if (shocksPerHour > 0)
    randomShocksPerHour = shocksPerHour;
  else
    randomShocksPerHour = 1;
  if (randomShockMode)
    ScheduleNextRandomShock();
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
    timeOfNextScheduledShock = timeFunc.GetTimeInSeconds() + random(86400*100) / (1200 * randomShocksPerHour);
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
    Serial.print("- IsSleeping: ");
    Serial.println(users.GetWearer()->IsSleeping());
    if (users.GetWearer()->IsSleeping())
    {
      // turn off random shock mode during sleeping time
      randomShockMode = false;
      return;
    }

    Serial.print("- timeOfNextScheduledShock: ");
    Serial.println(timeOfNextScheduledShock);
    Serial.print("- GetTimeInSeconds(): ");
    Serial.println(timeFunc.GetTimeInSeconds());
    if (timeOfNextScheduledShock < timeFunc.GetTimeInSeconds())
    {
      // execute random shock
      message.ShockAction(users.GetBot()->GetId(), GROUP_CHAT_ID, 1, 1000);
      // and schedule the next one
      ScheduleNextRandomShock();
    }
  }
}


//
