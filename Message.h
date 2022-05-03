#ifndef __Message_h__

#define __Message_h__

// ---------------------------------
// Persisted values
#define LAST_OPENING_TAG "LAOP"
//#define LAST_OPENING_TAG "LastOpening"
#define LAST_CLOSING_TAG "LACL"
#define LAST_SHOCK_TAG "LASH"
//#define LAST_SHOCK_TAG "LastShock"
#define LOCK_TIMER_END_TAG "LOTE"
#define RANDOM_MODE_START_TAG "RAMS"
//#define RANDOM_MODE_START_TAG "RandomModeStart"
#define NEXT_VERIFICATION_BEGIN_TAG "VSTA"
#define NEXT_VERIFICATION_END_TAG "VEND"
#define DAY_OF_WEEK_TAG "DOWE"
#define TEASING_MODE_TAG "TEMO"
//#define TEASING_MODE_TAG "TeasingMode"
#define RANDOM_MODE_TAG "RAMO"
//#define RANDOM_MODE_TAG "RandomMode"
#define VERIFICATION_MODE_TAG "VEMO"
#define ACTUAL_VERIFICATIONS_TAG "ACTV"
#define CREDITS_TAG "CRED"
//#define CREDITS_TAG "Credits"
#define VOUCHER_TAG "VOUC"
#define DEVIATIONS_TAG "DEVN"
#define FAILURES_TAG "FAIL"
#define SWVERSION_TAG "SWVERSION"
#define USERS_PREFIX "USERS:"
#define CHATS_PREFIX "CHATS:"
#define REFERENCE_PREFIX "REFERENCE:"
#define AWAY_TAG "AWAY:"
#define IPADDRESS_TAG "IPCLIENT:"

// general
#define SYMBOL_WARNING "\xe2\x9a\xa0"
#define SYMBOL_FINGER_UP "\xe2\x98\x9d"
// locking
#define SYMBOL_LOCK_CLOSED "\xf0\x9f\x94\x92"
#define SYMBOL_LOCK_OPEN "\xf0\x9f\x94\x93"
#define SYMBOL_LOCK_KEY "\xf0\x9f\x94\x90"
#define SYMBOL_KEY "\xf0\x9f\x94\x91"
#define SYMBOL_TWO_CHAIN_LINKS "\xf0\x9f\x94\x97"
#define SYMBOL_CHAINS "\xe2\x9b\x93"
#define SYMBOL_LOCKED_LUGGAGE "\xf0\x9f\x9b\x85"
// time
#define SYMBOL_MOON "\xf0\x9f\x8c\x99"
#define SYMBOL_SUN1 "\xe2\x98\x80"
#define SYMBOL_SUN2 "\xf0\x9f\x8c\x9e"
#define SYMBOL_SLEEPING "\xf0\x9f\x92\xa4"
// control
#define SYMBOL_WATCHING_EYES "\xf0\x9f\x91\x80"
#define SYMBOL_POLICEMAN "\xf0\x9f\x91\xae"
#define SYMBOL_PASSPORT_CONTROL "\xf0\x9f\x9b\x83"
#define SYMBOL_CUSTOMS "\xf0\x9f\x9b\x82"
#define SYMBOL_EYE "\xf0\x9f\x91\x81"
// holder
#define SYMBOL_QUEEN "\xf0\x9f\x91\xb8"
#define SYMBOL_DEVIL_SMILE "\xf0\x9f\x98\x88"
#define SYMBOL_DEVIL_ANGRY "\xf0\x9f\x91\xbf"
#define SYMBOL_BELL "\xf0\x9f\x94\x94"
#define SYMBOL_CLOCK_3 "\xf0\x9f\x95\x92"
#define SYMBOL_SMILY_SMILE "\xf0\x9f\x98\x8a"
#define SYMBOL_SMILY_WINK "\xf0\x9f\x98\x89"
#define SYMBOL_SMILE_OHOH "\xf0\x9f\x98\xb3"
#define SYMBOL_FORBIDDEN "\xf0\x9f\x9a\xab"
#define SYMBOL_STOP "\xf0\x9f\x9b\x91"
#define SYMBOL_OK "\xf0\x9f\x86\x97"
#define SYMBOL_NO_ENTRY "\xe2\x9b\x94"
// goodies
#define SYMBOL_STAR "\xe2\xad\x90"
#define SYMBOL_STAR_GLOWING "\xf0\x9f\x8c\x9f"
#define SYMBOL_GEM "\xf0\x9f\x92\x8e"
// games
#define SYMBOL_DIRECT_HIT "\xf0\x9f\x8e\xaf"
#define SYMBOL_DICE "\xf0\x9f\x8e\xb2"
#define SYMBOL_DIGIT0 "\x30\xe2\x83\xa3"
#define SYMBOL_DIGIT1 "\x31\xe2\x83\xa3"
#define SYMBOL_DIGIT2 "\x32\xe2\x83\xa3"
#define SYMBOL_DIGIT3 "\x33\xe2\x83\xa3"
#define SYMBOL_DIGIT4 "\x34\xe2\x83\xa3"
#define SYMBOL_DIGIT5 "\x35\xe2\x83\xa3"
#define SYMBOL_DIGIT6 "\x36\xe2\x83\xa3"
#define SYMBOL_DIGIT7 "\x37\xe2\x83\xa3"
#define SYMBOL_DIGIT8 "\x38\xe2\x83\xa3"
#define SYMBOL_DIGIT9 "\x39\xe2\x83\xa3"
#define SYMBOL_DRUM "\xf0\x9f\xa5\x81"
// naughty
#define SYMBOL_CHERRIES "\xf0\x9f\x8d\x92"
#define SYMBOL_AUBERGINE "\xf0\x9f\x8d\x86"
// rewards
#define SYMBOL_SPLASHING_SWEAT "\xf0\x9f\x92\xa6"
// punishments
#define SYMBOL_COLLISION "\xf0\x9f\x92\xa5"
#define SYMBOL_ICE_CUBE "\xf0\x9f\xa7\x8a"
#define SYMBOL_DOUGHNUT "\xf0\x9f\x8d\xa9"
#define SYMBOL_CRICKET_BAT "\xf0\x9f\x8f\x8f"

#define SYMBOL_CREDIT SYMBOL_STAR
#define SYMBOL_VOUCHER SYMBOL_LOCK_KEY

#define PARSE_MODE ""
#define RESIZE_KEYBOARD true
#define ONE_TIME_KEYBOARD true
#define MULTI_TIME_KEYBOARD false
#define SELECTIVE_KEYBOARD false
#define REMOVE_KEYBOARD true


#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"
#include "User.h"


extern String knownUserId[USER_CACHE_SIZE];
extern String knownUserName[USER_CACHE_SIZE];
extern int knownUserCount;



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
  void EditMessage(String msg, String chatId, int messageId);
  void SendMessageWithKeybaord(String chatId, String message, String keyboardJson);
  int  GetLastSentMessageId();
  void SendRichMessage(String id, String msg);
  void MessageWearerState(String chatId);
  void MessageLastShock(String chatId);
  void MessageIsClosed(String chatId);
  void MessageIsOpen(String chatId);
  void MessageCoverStateChange(String chatId=GROUP_CHAT_ID);
  void MessageCoverState(String chatId=GROUP_CHAT_ID);
  void MessageModes(String chatId=GROUP_CHAT_ID);
  void MessageUsers(String chatId=GROUP_CHAT_ID);
  void MessageRoles(String chatId=GROUP_CHAT_ID);
  void MessageState(String chatId=GROUP_CHAT_ID);
  void MessageTasks(String chatId=GROUP_CHAT_ID);
  void ShockAction(String durationStr, int count, String fromId, String chatId=GROUP_CHAT_ID);
  void ShockAction(unsigned long milliseconds, int count, String fromId, String chatId=GROUP_CHAT_ID);
  void HolderAction(String fromId, String chatId=GROUP_CHAT_ID);
  void TeaserAction(String fromId, String chatId=GROUP_CHAT_ID);
  void TeasingModeAction(bool mode, String fromId, String chatId=GROUP_CHAT_ID);
  void GuestAction(String fromId, String chatId=GROUP_CHAT_ID, bool force=false);
  void WaitingAction(String fromId, String chatId=GROUP_CHAT_ID);
  void FreeAction(String fromId, String chatId=GROUP_CHAT_ID);
  void CaptureAction(String fromId, String chatId=GROUP_CHAT_ID);
  void UnlockAction(String fromId, String chatId=GROUP_CHAT_ID, bool force=false);
  void LockTimerAction(String durationStr, String fromId, String chatId);
  void Play4UnlockAction(String fromId, String chatId=GROUP_CHAT_ID);
//  void PlayAction(String max, String fromId, String chatId=GROUP_CHAT_ID);
  void ReleaseAction(String fromId, String chatId=GROUP_CHAT_ID);
//  void RestrictUserAction(String fromId, String chatId=GROUP_CHAT_ID);
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
  void ParseSettings(String & descr);
  void AdoptChatDescription();
  void AdoptSettings();
  void UnknownCommand(String chatId=GROUP_CHAT_ID);
  void WriteCommandsAndSettings(String reference);
};

#endif

//
