#ifndef __User_h__

#define __User_h__

#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"


#define USER_ID_BOT "1264046045"
#define USER_ID_CHARLY "1157999292"
#define GROUP_CHAT_ID "-1001300311525"

// Initialize Telegram bot
// your bot token (from Botfather)
#define BOT_TOKEN "1264046045:AAEk49lP6viWTAkZ0jItDas5Y_03ZjoPPMg"

#define KEEP_TIMESTAMP false
#define UPDATE_TIMESTAMP true

#define ROLE_WEARER_COLLARED 0
#define ROLE_WEARER_WAITING 1
#define ROLE_WEARER_FREE 2
#define ROLE_HOLDER 3
#define ROLE_SHOCKCELL 4
#define ROLE_TEASER 5
#define ROLE_GUEST 6
#define ROLE_COUNT 7

#define USER_NONE -1


class UserSet;
class RoleSet;


extern RoleSet roles;



class Role
{
protected:
  int id;
  String name;

public:
  Role() { id = 0; name = ""; }
  Role(int i, const char *n) { id = i; name = n; }
  int GetId() { return id; }
  String GetName() { return name; }
};

class RoleSet
{
protected:
  Role roleInfo[ROLE_COUNT] = { { 0, "Wearer/collared" }, { 1, "Wearer/waiting" }, { 2, "Wearer/free" }, { 3, "Holder" }, { 4, "ShockCell" }, { 5, "Teaser" }, { 6, "Guest" } };

public:
  RoleSet() {}
  Role GetRole(int i) { return roleInfo[i]; }
  String GetRoleStr(int i) { return roleInfo[i].GetName(); }
  int GetRoleId(String s) { for (int i = 0; i < ROLE_COUNT; i++) { if (roleInfo[i].GetName() == s) return i; } return -1; }
};


class User
{
protected:
  UserSet *userSet;
  String name;
  String telegramId;
  unsigned long lastMessageTime;
  int roleId;
  bool isBot;
  bool sleeping = false;

public:
  User() : userSet(NULL), lastMessageTime(0), isBot(false), sleeping(false) {}
  void SetUserSet(UserSet *us) { userSet = us; }
  String GetName() { return name; }
  void SetName(String n) { name = n; }
  String GetId() { return telegramId; }
  void SetId(String i) { telegramId = i; }
  Role GetRole() { return roles.GetRole(roleId); }
  bool SetRole(Role r) { return SetRoleId(roleId); }
  int GetRoleId() { return roleId; }
  bool SetRoleId(int r, bool force=false);
  String GetRoleStr() { return roles.GetRoleStr(roleId); }
  bool UpgradeRoleId(int r) { if (r < roleId) return SetRoleId(r); else return false; }
  bool IsBot() { return isBot; }
  void SetBot(bool is=true);
  bool IsHolder() { return roleId == ROLE_HOLDER; }
  bool SetHolder() { SetRoleId(ROLE_HOLDER); }
  bool IsWearer() { return (roleId >= ROLE_WEARER_COLLARED) && (roleId <= ROLE_WEARER_FREE); }
  bool IsTeaser() { return roleId == ROLE_TEASER; }
  bool IsGuest() { return roleId == ROLE_GUEST; }
  bool IsFreeWearer() { return roleId == ROLE_WEARER_FREE; }
  bool IsWaitingWearer() { return roleId == ROLE_WEARER_WAITING; }
  bool IsCollaredWearer() { return roleId == ROLE_WEARER_COLLARED; }
  bool MayBecomeWearer() { return (IsWearer() || IsTeaser() || IsGuest()); }
  bool MayBecomeTeaser() { return (IsHolder() || IsTeaser() || IsGuest()); }
  bool MayBecomeHolder();
  bool MayUnlock() { return (IsFreeWearer() || IsWaitingWearer() || IsHolder()); }
  bool IsSleeping();
  void SetSleeping(bool s) { sleeping = s; }
  unsigned long LastMessageTime() { return lastMessageTime; }
  void SetLastMessageTime(unsigned int now) { lastMessageTime = now; }
};



class ChatGroup : public User
{
public:
  ChatGroup() {}
};



class UserSet
{
protected:
  int count;
  int wearerIndex;
  int holderIndex;
  User user[USER_CACHE_SIZE];
  unsigned long lastMessageTime;

public:
  UserSet() : count(0), wearerIndex(-1), holderIndex(-1) {}
  int GetCount() { return count; }
  User * GetUser(int i) { return &user[i]; }
  User * GetUserFromIndex(int i);
  User * GetUserFromId(String id);
  User * GetUserFromRole(int roleId);
  int GetIndexFromId(String id);
  User * GetWearer() { return GetUserFromIndex(wearerIndex); }
  User * GetHolder() { return GetUserFromIndex(holderIndex); }
  int GetWearerIndex() { return wearerIndex; }
  String GetWearerName() { return user[wearerIndex].GetName(); }
  void SetWearerIndex(int i) { wearerIndex = i; }
  int GetHolderIndex() { return holderIndex; }
  String GetHolderName() { return user[holderIndex].GetName(); }
  void SetHolderIndex(int i) { holderIndex = i; }
  int AddUser(String id, String name="", int role=ROLE_GUEST, bool isBot=false);
  unsigned long LastMessageTime() { return lastMessageTime; }
  void SetLastMessageTime(unsigned long now) { lastMessageTime = now; }
  String GetUsersInfo();
};

#endif

//
