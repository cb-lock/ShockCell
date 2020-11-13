#ifndef __EmlaServer_h__

#define __EmlaServer_h__

#include "Arduino.h"
#include "Oled.h"


class EmlaServer
{
private:
  OledDisplay & oledDisplay;
  String emlaserver;
  String emlaurl;
  String sessionCookie;
  
public:
  EmlaServer(OledDisplay & o) : oledDisplay(o)
  {
    emlaserver = "www.emlalock.com";
    emlaurl = "https://" + emlaserver;
  }
  String GetServer() { return emlaserver; }
  bool Connect2WiFi(const char *ssid, const char *password, int timeout);
  String GetUrl() { return emlaurl; }
  String GetSessionCookie() { return sessionCookie; }
  String GetCookieHeader();
  void SetCookieFromHeader(String rawCookie);
  bool WGet(const String url, String & payload);
  bool WPost(const String url, const String request, String & payload, bool getSessionCookie = false);
  bool WDelete(const String url, const String request, String & payload);
  String UrlEncode(const char *src);
};

#endif

//
