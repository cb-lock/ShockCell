#ifndef __Chat_h__

#define __Chat_h__

#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"



#define KEEP_TIMESTAMP false
#define UPDATE_TIMESTAMP true

#define CHAT_NONE -1


class ChatSet;


class Chat
{
protected:
  ChatSet *chatSet;
  int wearerIndex;
  int holderIndex;
  int botIndex;
  String name;
  String telegramId;
  unsigned long lastMessageTime;
  bool isHolderChat;

public:
  Chat() : chatSet(NULL), wearerIndex(-1), holderIndex(-1), botIndex(-1), lastMessageTime(0), isHolderChat(false) {}
  void SetChatSet(ChatSet *cs) { chatSet = cs; }
  String GetName() { return name; }
  void SetName(String n) { name = n; }
  String GetId() { return telegramId; }
  void SetId(String i) { telegramId = i; }
  bool IsHolderChat() { return isHolderChat; }
  String IsHolderChatStr() { return isHolderChat ? "true" : "false"; }
  void SetHolderChat(bool is=true) { isHolderChat = is; }
  void SetHolderChatStr(String isHChat) { isHolderChat = (isHChat == "true"); }
  unsigned long LastMessageTime() { return lastMessageTime; }
  void SetLastMessageTime(unsigned int now) { lastMessageTime = now; }
};



class ChatGroup : public Chat
{
public:
  ChatGroup() {}
};



class ChatSet
{
protected:
  int count;
  int holderChatIndex;
  Chat chat[CHAT_CACHE_SIZE];
  unsigned long lastMessageTime;

public:
  ChatSet() : count(0), holderChatIndex(CHAT_NONE) {}
  int GetCount() { return count; }
  Chat * GetChat(int i) { return &chat[i]; }
  Chat * GetChatFromIndex(int i);
  Chat * GetChatFromId(String id);
  Chat * SearchHolderChat();
  int GetIndexFromId(String id);
  Chat * GetHolderChat() { if (holderChatIndex >= 0) return GetChatFromIndex(holderChatIndex); else return SearchHolderChat(); }
  int GetHolderChatIndex() { return holderChatIndex; }
  String GetHolderChatName() { return chat[holderChatIndex].GetName(); }
  void SetHolderChatIndex(int i) { holderChatIndex = i; }
  int AddChat(String id, String name="", bool isHolderChat=false);
  unsigned long LastMessageTime() { return lastMessageTime; }
  void SetLastMessageTime(unsigned long now) { lastMessageTime = now; }
  String GetChatsInfo();
};

#endif

//
