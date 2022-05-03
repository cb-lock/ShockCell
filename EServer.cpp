#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
//#define PROXY_ENABLED
//#include <WiFi.h>
#include "HTTPSP.h"
//#include <WiFiClientSecure.h>
//#include <StreamString.h>
#include <ESP32Ping.h>
#include "EServer.h"
#include "Defs.h"
#include "Oled.h"


//extern StreamString sString;



// ------------------------------------------------------------------------
bool EmlaServer::Connect2WiFi(const char *ssid, const char *password, int timeout)
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
#ifdef ESP8266
  WiFi.setInsecure();
#endif
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  int attempts = 0;
  while ((WiFi.status() != WL_CONNECTED) && (attempts < timeout))
  {
    ++attempts;
    oledDisplay.PrintInitDisplay(ssid, 0, attempts);
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to ");
    Serial.println(ssid);
    oledDisplay.PrintInitDisplay(ssid, 1, 0);
    return true;
  }
  else
  {
    Serial.print("Connection to ");
    Serial.print(ssid);
    Serial.println(" failed!");
    oledDisplay.PrintInitDisplay(ssid, 2, 0);
//    lcd.setCursor(0, 0);
//    lcd.print(EMPTY_LINE);
    return false;
  }
}

// ------------------------------------------------------------------------
void EmlaServer::SetCookieFromHeader(String rawCookie)
{
  sessionCookie = rawCookie;
  Serial.print("Received cookie: ");
  Serial.println(sessionCookie);
  sessionCookie = sessionCookie.substring(sessionCookie.indexOf("=") + 1);
  sessionCookie.remove(sessionCookie.indexOf(";"));
  Serial.print("Extracted cookie: ");
  Serial.println(sessionCookie);
}

// ------------------------------------------------------------------------
String EmlaServer::GetCookieHeader()
{
  Serial.print("- use cookie: ");
  Serial.println(sessionCookie);

  return "ELKEY=" + sessionCookie + ";";
}

// ------------------------------------------------------------------------
bool EmlaServer::WGet(const String url, String & payload)
{
  bool result = false;
  WiFiClientSecure *client = new WiFiClientSecure;
  payload = "";

  Serial.print("*** EmlaServer::WGet: ");
  Serial.println(url);
  if (client)
  {
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("GET ");
      Serial.print(url);
      Serial.print(" ");
      if (https.begin(*client, url.c_str()))
      { // HTTPS
        // start connection and send HTTP header
        https.addHeader("Cookie", GetCookieHeader());
        int retry = 0;
        int httpCode = 0;
        while ((httpCode <= 0) && (retry < 4))
        {
          httpCode = https.GET();

          // httpCode will be negative on error
          if (httpCode > 0)
          {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
//              https.getSize();
//              https.getString(sString);
//              Serial.println(payload2);
              String payload2 = https.getString();
              payload = payload2;
//              Serial.println(sString);
//              payload = sString;
              Serial.println(payload);
              result = true;
            }
          }
          else
          {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
          retry++;
        }

        https.end();
      }
      else
      {
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }

    //client->stop();
    delete client;
  }
  else
  {
    Serial.println("Unable to create client");
  }
  return result;
}

// ------------------------------------------------------------------------
bool EmlaServer::WPost(const String url, const String request, String & payload, bool getSessionCookie)
{
  bool result = false;
  WiFiClientSecure *client = new WiFiClientSecure;
  payload = "";

  Serial.println("*** EmlaServer::WPost");
  if (client)
  {
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("POST ");
      Serial.print(url.c_str());
      Serial.print("; request=");
      Serial.println(request);
      if (https.begin(*client, url.c_str()))
      { // HTTPS
        const char * headerkeys[] = {"Set-Cookie", "Cookie", "ETag", "Date", "Content-Type", "Connection"};
        size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
        // start connection and send HTTP header
        https.setReuse(false);
        https.addHeader("Content-Type", "application/x-www-form-urlencoded");
        https.addHeader("Cookie", GetCookieHeader().c_str());
        https.collectHeaders(headerkeys, headerkeyssize);

        int retry = 0;
        int httpCode = 0;
        while ((httpCode <= 0) && (retry < 4))
        {
          httpCode = https.POST(request);

          // httpCode will be negative on error
          if (httpCode > 0)
          {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
  
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
              payload = https.getString();
              Serial.println(payload);
            }
            else
            {
              payload = https.getString();
              Serial.println(payload);
            }
            result = true;
          }
          else
          {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
          Serial.print("httpcode = ");
          Serial.println(httpCode);
          retry++;
          Serial.println(retry);
        }

        if (getSessionCookie)
        {
          Serial.println("Cookie requested");
          int c = 0;
          const char *headerKeys;
          while (c < https.headers())
          {
            Serial.println(https.header(c));
            c++;
          }
          if (https.hasHeader("Set-Cookie"))
          {
            SetCookieFromHeader(https.header("Set-Cookie"));
          }
          else
            Serial.print("No cookie received");
        }
        else
          Serial.println("No cookie requested");

        https.end();
      }
      else
      {
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }

    client->stop();
    delete client;
  }
  else
  {
    Serial.println("Unable to create client");
  }
  return result;
}

/*
// ------------------------------------------------------------------------
bool EmlaServer::WDelete(const String url, const String request, String & payload)
{
  bool result = false;
  WiFiClientSecure *client = new WiFiClientSecure;
  payload = "";

  Serial.println("*** EmlaServer::WDelete");
  if (client)
  {
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;

      Serial.print("DELETE ");
      Serial.print(url.c_str());
      Serial.print("; request=");
      Serial.println(request);
      if (https.begin(*client, url.c_str()))
      { // HTTPS
        const char * headerkeys[] = {"Set-Cookie", "Cookie", "ETag", "Date", "Content-Type", "Connection"};
        size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
        // start connection and send HTTP header
        https.setReuse(false);
        https.addHeader("Content-Type", "application/x-www-form-urlencoded");
        https.addHeader("Cookie", GetCookieHeader().c_str());
        https.collectHeaders(headerkeys, headerkeyssize);

        int retry = 0;
        int httpCode = 0;
        while ((httpCode <= 0) && (retry < 4))
        {
          httpCode = https.DELETE(request);

          // httpCode will be negative on error
          if (httpCode > 0)
          {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
  
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
              payload = https.getString();
              Serial.println(payload);
            }
            else
            {
              payload = https.getString();
              Serial.println(payload);
            }
            result = true;
          }
          else
          {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
          Serial.print("httpcode = ");
          Serial.println(httpCode);
          retry++;
          Serial.println(retry);
        }

        https.end();
      }
      else
      {
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }

    client->stop();
    delete client;
  }
  else
  {
    Serial.println("Unable to create client");
  }
  return result;
}
*/


// ------------------------------------------------------------------------
String EmlaServer::UrlEncode(const char *src)
{
  Serial.print("*** UrlEncode: ");
  Serial.print(src);
  Serial.print(" -> ");
  char specials[] = "$&+,/:;=?@ <>#%{}|\\^~[]`"; /* String containing chars you want encoded */
  char c;
  char buffer[MESSAGE_HISTORY_SIZE * MESSAGE_SUMMARY_SIZE];
  char *d = &buffer[0];
  while (c = *src++)
  {
    if (strchr(specials, c))
    {
      *d++ = '%';
      *d++ = "01234567890ABCDEF"[(c >> 4) & 0x0F];
      *d++ = "01234567890ABCDEF"[(c) & 0x0F];
    }
    else *d++ = c;
  }
  *d = 0;
  d = &buffer[0];

  Serial.println(d);
  return String(d);
}


// ------------------------------------------------------------------------
bool ePing(const char *hostname)
{
  bool success = Ping.ping("192.168.1.67", 10);
  return success;
}

//
