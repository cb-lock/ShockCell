#include <EEPROM.h>

#include "TimeF.h"
#include "Chat.h"
#include "Defs.h"
#include "Message.h"


extern TimeFunctions timeFunc;
extern UniversalTelegramBot bot;
extern Message message;
extern ChatSet chats;



// ------------------------------------------------------------------------
int ChatSet::AddChat(String id, String name, bool isHChat)
{
  Serial.println("*** ChatSet::AddChat()");
  int i = 0;
  while (i < CHAT_CACHE_SIZE)
  {
    if (chat[i].GetId().length() == 0)
    {
      // create new chat
      chat[i].SetChatSet(this);
      chat[i].SetId(id);
      chat[i].SetName(name);
      chat[i].SetHolderChat(isHChat);
      Serial.print("- add ");
      Serial.print(id);
      Serial.print(", ");
      Serial.print(name);
      Serial.print(", ");
      Serial.print(isHChat);
      Serial.println();
      count = i + 1;
      // we need to make sure that the description is changed with every request and not identical to the existing one.
      return i;
    }
    else if (chat[i].GetId() == id)
    {
      // update existing chat
      chat[i].SetName(name);
      Serial.print(id);
      Serial.print(", ");
      Serial.print(name);
      Serial.println();
      return i;
    }
    ++i;
  }
  return CHAT_NONE;
}


// ------------------------------------------------------------------------
Chat * ChatSet::GetChatFromIndex(int i)
{
  if (i >= 0)
    return &chat[i];
  else
    return NULL;
}


// ------------------------------------------------------------------------
Chat * ChatSet::GetChatFromId(String id)
{
  int i = 0;
  while (i < CHAT_CACHE_SIZE)
  {
    if (chat[i].GetId() == id)
      return &chat[i];
    ++i;
  }
  return NULL;
}


// ------------------------------------------------------------------------
Chat * ChatSet::SearchHolderChat()
{
  int i = 0;
  while (i < CHAT_CACHE_SIZE)
  {
    // only return those matches of valid chats
    if ((chat[i].GetId().length() > 0) &&
        chat[i].IsHolderChat())
    {
      holderChatIndex = i;
      return &chat[i];
    }
    ++i;
  }
  return NULL;
}


// ------------------------------------------------------------------------
int ChatSet::GetIndexFromId(String id)
{
  int i = 0;
  while (i < CHAT_CACHE_SIZE)
  {
    if (chat[i].GetId() == id)
      return i;
    ++i;
  }
  return CHAT_NONE;
}

 
// ------------------------------------------------------------------------
String ChatSet::GetChatsInfo()
{
  int i = 0;
  String info;
  while (i < CHAT_CACHE_SIZE)
  {
    if (chat[i].GetId().length() > 0)
    {
      info += chat[i].GetName() + ":" + chat[i].GetId() + ":" + (chat[i].IsHolderChat() ? "true" : "false") + "; ";
    }
    ++i;
  }
  return info;
}


//
