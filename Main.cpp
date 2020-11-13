#include "esp32-hal-ledc.h"
// for the watchdog timers
#include "esp_system.h"
#include "Oled.h"
#include "Defs.h"
#include "TimeF.h"
#include "EServer.h"
#include "Message.h"
#include "Session.h"
#include "User.h"

// ---------------------------------

// Peripherals
OledDisplay oledDisplay;
int coverState = 0;
int oldCoverState = 0;

//StreamString sString;
TimeFunctions timeFunc;
EmlaServer emlaServer(oledDisplay);
Message message;
Session session;

ChatGroup chatGroup;
UserSet users;

unsigned long lastTimeBotRan;

hw_timer_t *watchdogTimer = NULL;




void PrintDisplay(String statusMsg = "");




// ------------------------------------------------------------------------
void IRAM_ATTR resetModule()
{
  Serial.println("********************* REBOOT ***********************");
  ESP.restart();
}


// -------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(200);

  //timer 0, div 40000 = 0,5 ms, 2000 ticks for 1 second
  //timer 0, div 8000 = 0,1 ms, 10000 ticks for 1 second
  // hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp)
  watchdogTimer = timerBegin(0, 8000, true);
  // void timerAttachInterrupt(hw_timer_t *timer, void (*fn)(void), bool edge)
  timerAttachInterrupt(watchdogTimer, &resetModule, true);
  // 60 seconds timeout for WD timer
  // 2000*60 = 120000
  // 10000*60 = 600000
  // void timerAlarmWrite(hw_timer_t *timer, uint64_t alarm_value, bool autoreload)
  timerAlarmWrite(watchdogTimer, 1200000, false); //set time in us
  // void timerAlarmEnable(hw_timer_t *timer)
  timerAlarmEnable(watchdogTimer); //enable interrupt

  // Pins
  pinMode(SHOCK_PIN, OUTPUT);
  // pin controlling the cover state
  pinMode(COVER_OPEN_PIN, INPUT_PULLUP);

  // OLED
  oledDisplay.Init();

  // servo
  // channel 1, 50 Hz, 16 bit width
  ledcSetup(1, 50, 16);
  // GPIO 2 attached to channel 1
  ledcAttachPin(SERVO_PIN, 1);

  // --------------------------------------------------------------------------
  // WiFi connection
  bool success = false;
  while (!success)
  {
    oledDisplay.PrintInitDisplay(SSID2, 0, 0);
    success = emlaServer.Connect2WiFi(SSID2, PASSWORD2, 3);
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID1, PASSWORD1, 3);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID3, PASSWORD3, 3);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID2, 0, 0);
      success = emlaServer.Connect2WiFi(SSID2, PASSWORD2, 3);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID1, PASSWORD1, 3);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID3, PASSWORD3, 3);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID2, 0, 0);
      success = emlaServer.Connect2WiFi(SSID2, PASSWORD2, 30);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID1, PASSWORD1, 30);
    }
    if (!success)
    {
      oledDisplay.PrintInitDisplay(SSID1, 0, 0);
      success = emlaServer.Connect2WiFi(SSID3, PASSWORD3, 30);
    }
  }

  // --------------------------------------------------------------------------
  timeFunc.SetClock();
  randomSeed(timeFunc.GetTimeInSeconds());

  coverState = digitalRead(COVER_OPEN_PIN);
  oldCoverState = coverState;

  message.Init();
  session.InfoChastikey();

  session.SetTimeOfLast5sInterval(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLast5sInterval(timeFunc.GetTimeInSeconds());
}



// -------------------------------------------------
void loop()
{
  timerWrite(watchdogTimer, 0);

  //------------
  unsigned long milliseconds = millis();
  String msg;

  coverState = digitalRead(COVER_OPEN_PIN);
  if (coverState != oldCoverState)
  {
    oledDisplay.PrintDisplay();
    message.MessageCoverStateChange();
    message.WriteCommandsAndSettings();
    oldCoverState = coverState;
  }

  if (milliseconds > lastTimeBotRan + BOT_REQUEST_INTERVAL)
  {
    message.ProcessNewMessages();
    lastTimeBotRan = millis();
  }

  // every 5 seconds
  if (timeFunc.GetTimeInSeconds() >= (session.GetTimeOfLast5sInterval() + 5))
  {
    oledDisplay.PrintDisplay();
    session.ProcessRandomShocks();
    session.SetTimeOfLast5sInterval(timeFunc.GetTimeInSeconds());
  }

  // every 5 minutes
  if (timeFunc.GetTimeInSeconds() >= (session.GetTimeOfLast5minInterval() + 5*60))
  {
//    mistress.CheckOffline();
    session.ProcessVerification();

    // has the sleeping period just begun?
    if (users.GetHolder())
    {
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
    }

    if (session.GetEmergencyReleaseCounterRequest())
    {
      session.SetEmergencyReleaseCounter(session.GetEmergencyReleaseCounter() + 1);
      if (session.GetEmergencyReleaseCounter() < 2)
        message.SendMessageAll("The first emergency release request has been accepted, but it needs to be raised again for confirmation.");
      else
        message.UnlockAction(users.GetWearer()->GetId(), GROUP_CHAT_ID, FORCE);
    }
    else
    {
      if (session.GetEmergencyReleaseCounter() > 0)
      {
        session.SetEmergencyReleaseCounter(0);
        message.SendMessage("The emergency request has not been confirmed and is cleared now.");
      }
    }
    session.SetEmergencyReleaseCounterRequest(false);

    session.SetTimeOfLast5minInterval(timeFunc.GetTimeInSeconds());
  }
}

//
