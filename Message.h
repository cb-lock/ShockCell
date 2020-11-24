#ifndef __Message_h__

#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"
#include "User.h"

#define USERS_PREFIX "USERS:"
#define CHATS_PREFIX "CHATS:"

extern String knownUserId[USER_CACHE_SIZE];
extern String knownUserName[USER_CACHE_SIZE];
extern int knownUserCount;

#define __Message_h__

class Message
{
private:
  String lastChatDescription;
  bool mutedWearer;
  bool requestRestart = false;
  String requestChatId = "";

public:
  Message();
  void Init();
  void ProcessChatMessage(String msg, String fromId, String chatId=GROUP_CHAT_ID);
  void ProcessNewMessages();
  void SendMessage(String msg, String chatId=GROUP_CHAT_ID);
  void SendMessageAll(String msg, String chatId=GROUP_CHAT_ID);
  void SendRichMessage(String id, String msg);
  void MessageLastShock(String chatId);
  void MessageIsClosed(String chatId);
  void MessageIsOpen(String chatId);
  void MessageCoverStateChange(String chatId=GROUP_CHAT_ID);
  void MessageCoverState(String chatId=GROUP_CHAT_ID);
  void MessageSendEarnedCredits(int creditsEarned, String chatId=GROUP_CHAT_ID);
  void MessageModes(String chatId=GROUP_CHAT_ID);
  void MessageUsers(String chatId=GROUP_CHAT_ID);
  void MessageRoles(String chatId=GROUP_CHAT_ID);
  void MessageChastikeyState(String chatId=GROUP_CHAT_ID);
  void MessageState(String chatId=GROUP_CHAT_ID);
  void ShockAction(String durationStr, int count, String fromId, String chatId=GROUP_CHAT_ID);
  void ShockAction(unsigned long milliseconds, int count, String fromId, String chatId=GROUP_CHAT_ID);
  void HolderAction(String fromId, String chatId=GROUP_CHAT_ID);
  void TeaserAction(String fromId, String chatId=GROUP_CHAT_ID);
  void TeasingModeAction(bool mode, String fromId, String chatId=GROUP_CHAT_ID);
  void GuestAction(String fromId, String chatId=GROUP_CHAT_ID);
  void WaitingAction(String fromId, String chatId=GROUP_CHAT_ID);
  void FreeAction(String fromId, String chatId=GROUP_CHAT_ID);
  void CaptureAction(String fromId, String chatId=GROUP_CHAT_ID);
  void UnlockAction(String fromId, String chatId=GROUP_CHAT_ID, bool force=false);
  void ReleaseAction(String fromId, String chatId=GROUP_CHAT_ID);
  void RestrictUserAction(String fromId, String chatId=GROUP_CHAT_ID);
  void RandomShockModeAction(String commandParameter, String fromId, String chatId=GROUP_CHAT_ID, bool force=false);
  void VerificationModeAction(String commandParameter, String fromId, String chatId=GROUP_CHAT_ID, bool force=false);
  void CheckVerificationAction(String caption, String fromId, String chatId=GROUP_CHAT_ID);
  void EmergencyAction(String fromId, String chatId=GROUP_CHAT_ID);
  void RestartRequest(String fromId, String chatId=GROUP_CHAT_ID);
  void RestartAction(String fromId, String chatId=GROUP_CHAT_ID);
  void ResetRequests();
  void ProcessPendingRequests();
  void SetChatDescription(String chatId, String descr);
  unsigned long ReadParamLong(String text, String id);
  void AdoptUserInfos(String text);
  void AdoptChatDescription();
  void UnknownCommand(String chatId=GROUP_CHAT_ID);
  void WriteCommandsAndSettings();
};

#endif

//
