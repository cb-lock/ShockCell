#include <U8g2lib.h>
#include "Oled.h"
#include "TimeF.h"
#include "User.h"


extern TimeFunctions timeFunc;
extern UserSet users;



// ------------
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
/*
#define LCD_RS     19
#define LCD_Enable  4
#define LCD_D4     16
#define LCD_D5     17
#define LCD_D6      5
#define LCD_D7     18
#define EMPTY_LINE "                "
//#define LED_ACTIVE 15
// initialize the library with the numbers of the interface pins

//                RS, E, D4, D5, D6, D7
LiquidCrystal lcd(LCD_RS, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
*/

// ------------------------------------------------------------------------
OledDisplay::OledDisplay()
{
}

void OledDisplay::Init()
{
  u8g2.begin();
}



// ------------------------------------------------------------------------
void OledDisplay::Show(String *line)
{
  Serial.println("*** OledDisplay::Show()");
  u8g2.firstPage();
  do
  {
    // u8g2_font_prospero_bold_nbp_tf
    // u8g2_font_tenthinnerguys_t_all
    // u8g2_font_tenthinguys_tf
    // u8g2_font_Born2bSportyV2_tf
    // u8g2_font_8x13B_tf
    // u8g2_font_9x18B_tf
    // u8g2_font_t0_15b_tf
    u8g2.setFont(u8g2_font_profont10_mf);
//    u8g2.setFont(u8g2_font_prospero_bold_nbp_tf);

    const int zeilenAbstand = 9;
    for (int i = 0; i < 7; i++)
    {
      u8g2.drawStr(0,(i+1)*zeilenAbstand, line[i].c_str());
      Serial.println(line[i]);
    }
  }
  while (u8g2.nextPage());
}



// ------------------------------------------------------------------------
void OledDisplay::PrintInitDisplay(String ssid, int connection_status, int attempts)
{
  String line[MAX_LINES];

  if (connection_status == 0)
  {
    line[0] = "Connecting: " + String(attempts);
    line[1] = "-> " + ssid;
  }
  else if (connection_status == 1)
  {
    line[0] = "CONNECTED:";
    line[1] = ssid;
  }
  else if (connection_status == 2)
  {
    line[0] = "ERROR!";
    line[1] = ssid;
  }
  line[2] = "";

  Show(line);
}



// ------------------------------------------------------------------------
void OledDisplay::PrintDisplay(String statusMsg)
{
  String line[20];

  for (int i = 0; i < 6; i++)
  {
    if (users.GetUser(i)->GetName().length() > 0)
      line[i] = users.GetUser(i)->GetName() + " (" + users.GetUser(i)->GetRoleStr() + ")";
    else
      line[i] = "";
  }
//  line[5] = statusMsg;
  line[6] = timeFunc.GetTimeString(WITH_DATE);

  Show(line);
}


//
