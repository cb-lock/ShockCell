#ifndef __Message_h__

#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"
#include "User.h"

#define USERS_PREFIX "USERS:"

extern String knownUserId[USER_CACHE_SIZE];
extern String knownUserName[USER_CACHE_SIZE];
extern int knownUserCount;

#define __Message_h__

class Message
{
private:
  String lastChatDescription;
  bool mutedWearer;

public:
  Message();
  void Init();
  void ProcessChatMessage(String msg, String fromId, String chatId=GROUP_CHAT_ID);
  void ProcessNewMessages();
  void SendMessage(String id, String msg);
  void SendRichMessage(String id, String msg);
  void MessageLastShock(String chatId);
  void MessageIsClosed(String chatId);
  void MessageIsOpen(String chatId);
  void MessageCoverStateChange(String chatId=GROUP_CHAT_ID);
  void MessageCoverState(String chatId=GROUP_CHAT_ID);
  void MessageUsers(String chatId=GROUP_CHAT_ID);
  void MessageRoles(String chatId=GROUP_CHAT_ID);
  void MessageChastikeyState(String chatId=GROUP_CHAT_ID);
  void MessageState(String chatId=GROUP_CHAT_ID);
  void ShockAction(String chatId, int count, long milliseconds);
  void HolderAction(String fromId, String chatId=GROUP_CHAT_ID);
  void TeaserAction(String fromId, String chatId=GROUP_CHAT_ID);
  void GuestAction(String fromId, String chatId=GROUP_CHAT_ID);
  void WaitingAction(String fromId, String chatId=GROUP_CHAT_ID);
  void FreeAction(String fromId, String chatId=GROUP_CHAT_ID);
  void CollarAction(String fromId, String chatId=GROUP_CHAT_ID);
  void UnlockAction(String fromId, String chatId=GROUP_CHAT_ID);
  void ReleaseAction(String fromId, String chatId=GROUP_CHAT_ID);
  void RestrictUserAction(String fromId, String chatId=GROUP_CHAT_ID);
  void SetChatDescription(String chatId, String descr);
  long ReadParamLong(String text, String id);
  void AdoptUserInfos(String text);
  void AdoptChatDescription(String descr);
  void UpdateChatDescription();
  void UnknownCommand(String chatId=GROUP_CHAT_ID);
};

#endif

//
