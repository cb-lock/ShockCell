#include "esp32-hal-ledc.h"
// for the watchdog timers
#include "esp_system.h"
#include "Oled.h"
#include "Defs.h"
#include "TimeF.h"
#include "EServer.h"
#include "Message.h"
#include "Session.h"
#include "Chat.h"
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
Tasklist tasklist;
Verification verification;
Session session;

ChatSet chats;
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
  timerAlarmWrite(watchdogTimer, 1200000L, false); //set time in us
  // void timerAlarmEnable(hw_timer_t *timer)
  timerAlarmEnable(watchdogTimer); //enable interrupt

  // Pins
  pinMode(SHOCK_PIN, OUTPUT);
  pinMode(SHOCK1_PIN, OUTPUT);
  digitalWrite(SHOCK1_PIN, HIGH);
  pinMode(SHOCK2_PIN, OUTPUT);
  digitalWrite(SHOCK2_PIN, HIGH);
  pinMode(SHOCK3_PIN, OUTPUT);
  digitalWrite(SHOCK3_PIN, HIGH);
  pinMode(SHOCK4_PIN, OUTPUT);
  digitalWrite(SHOCK4_PIN, HIGH);
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
  timeFunc.UpdateDSTOffset();
  timeFunc.SetClock();
  randomSeed(timeFunc.GetTimeInSeconds());

  coverState = digitalRead(COVER_OPEN_PIN);
  oldCoverState = coverState;

  tasklist.Init();
  verification.Init();
  message.Init();
  session.SetTimeOfLast5sInterval(timeFunc.GetTimeInSeconds());
  //
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
    message.WriteCommandsAndSettings("loop() coverStateChanged");
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

  // every 1 minute
  if (timeFunc.GetTimeInSeconds() >= (session.GetTimeOfLast1minInterval() + 60))
  {
    if (ePing(""))
    {
      // Wearer is at home
      if (session.GetAwayCounter() > AWAY_LIMIT)
      {
        message.SendMessage("Wearer returned home.", GROUP_CHAT_ID);
        message.WriteCommandsAndSettings("loop() Wearer returned home");
      }
      session.SetAwayCounter(0);
    }
    else
    {
      // Wearer cannot be reached
      if (session.GetAwayCounter() == AWAY_LIMIT)
        message.SendMessage("Wearer is away from home since " + String(AWAY_LIMIT, DEC) + " minutes.", GROUP_CHAT_ID);
      if (session.GetAwayCounter() == (AWAY_LIMIT + 1))
        message.WriteCommandsAndSettings("loop() Wearer is away");
      session.SetAwayCounter(session.GetAwayCounter() + 1);
    }
    verification.ProcessVerification(GROUP_CHAT_ID);
    timeFunc.ProcessSleepTime();
    // this method calls tasklist.ProcessTasks() in the morning and evening

    session.SetTimeOfLast1minInterval(timeFunc.GetTimeInSeconds());
  }

  // every 5 minutes
  if (timeFunc.GetTimeInSeconds() >= (session.GetTimeOfLast5minInterval() + 5*60))
  {
    verification.Schedule();
//    mistress.CheckOffline();

    if (session.GetEmergencyReleaseCounterRequest())
    {
      session.SetEmergencyReleaseCounter(session.GetEmergencyReleaseCounter() + 1);
      if (session.GetEmergencyReleaseCounter() < 2)
        message.SendMessageAll("The first emergency release request has been accepted, but it needs to be raised again for confirmation. If the second request is raised successfully, the safe will be unlocked and the holder role will be removed.");
      else
      {
        message.UnlockAction(users.GetWearer()->GetId(), GROUP_CHAT_ID, FORCE);
        message.GuestAction(users.GetHolder()->GetId(), GROUP_CHAT_ID, FORCE);
        session.SetEmergencyReleaseCounterRequest(false);
      }
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
    timeFunc.UpdateDSTOffset();

    session.SetTimeOfLast5minInterval(timeFunc.GetTimeInSeconds());
  }
}

//
