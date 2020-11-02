#include "esp32-hal-ledc.h"
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

void PrintDisplay(String statusMsg = "");




// -------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(200);

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
  //------------
  unsigned long milliseconds = millis();
  String msg;

  coverState = digitalRead(COVER_OPEN_PIN);
  if (coverState != oldCoverState)
  {
    oledDisplay.PrintDisplay();
    message.MessageCoverStateChange();
    message.UpdateChatDescription();
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

    // has the sleeping period just begun?
    if (timeFunc.SleepingTimeJustChanged(true) && ! users.GetWearer()->IsSleeping())
    {
      users.GetWearer()->SetSleeping(true);
      msg = "Good night "  + users.GetWearer()->GetName() + ", sleep well and frustrated.";
      message.SendMessage(msg);
    }
    // has the sleeping period just ended?
    if (timeFunc.SleepingTimeJustChanged(false) && users.GetWearer()->IsSleeping())
    {
      users.GetWearer()->SetSleeping(false);
      msg = "Good morning "  + users.GetWearer()->GetName() + ", wake up boy!";
      message.SendMessage(msg);
    }
    session.SetTimeOfLast5minInterval(timeFunc.GetTimeInSeconds());
  }
}

//
