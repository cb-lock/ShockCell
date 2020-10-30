#ifndef __Oled_h__

#define __Oled_h__

#include "Arduino.h"

#define DISPLAY_LINE_COUNT 7
#define MAX_LINES 10

class OledDisplay
{
private:

public:
  OledDisplay();
  void Init();
  void Show(String *line);
  void PrintInitDisplay(String ssid, int connection_status, int attempts);
  void PrintDisplay(String statusMsg = "");
};

#endif

//
