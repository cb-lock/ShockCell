#ifndef __User_h__

#define __User_h__

#include "Arduino.h"
#include <UniversalTelegramBot.h>
#include "Defs.h"



#define USER_CACHE_SIZE 30

#define KEEP_TIMESTAMP false
#define UPDATE_TIMESTAMP true

#define ROLE_WEARER_CAPTURED 0
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
  Role roleInfo[ROLE_COUNT] = { { 0, "Wearer/captured" }, { 1, "Wearer/waiting" }, { 2, "Wearer/free" }, { 3, "Holder" }, { 4, "ShockCell" }, { 5, "Teaser" }, { 6, "Guest" } };

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
  //  Role GetRole() { return roles.GetRole(roleId); }
  //  bool SetRole(Role r) { return SetRoleId(r.GetId()); }
  int GetRoleId() { return roleId; }
  bool SetRoleId(int r, bool force=false);
  String GetRoleStr() { return roles.GetRoleStr(roleId); }
  // this method is the standard method to manage all the side effects of role changes
  bool UpdateRoleId(int r, bool force=false);
  // this is method is used in case that the privileges shall only be upgraded
  bool UpgradeRoleId(int r) { if (r < roleId) return UpdateRoleId(r); else return false; }
  bool IsBot() { return isBot; }
  void SetBot(bool is=true);
  bool IsHolder() { return roleId == ROLE_HOLDER; }
  bool SetHolder() { UpdateRoleId(ROLE_HOLDER); }
  bool IsWearer() { return (roleId >= ROLE_WEARER_CAPTURED) && (roleId <= ROLE_WEARER_FREE); }
  bool IsTeaser() { return roleId == ROLE_TEASER; }
  bool IsGuest() { return roleId == ROLE_GUEST; }
  bool IsFreeWearer() { return roleId == ROLE_WEARER_FREE; }
  bool IsWaitingWearer() { return roleId == ROLE_WEARER_WAITING; }
  bool IsCapturedWearer() { return roleId == ROLE_WEARER_CAPTURED; }
  bool MayBecomeWearer() { return (IsWearer() || IsTeaser() || IsGuest()); }
  bool MayBecomeTeaser() { return (IsHolder() || IsTeaser() || IsGuest()); }
  bool MayBecomeHolder();
  bool MayUnlock() { return (IsFreeWearer() || IsWaitingWearer() || IsHolder()); }
  bool IsSleeping() { return sleeping; }
  void SetSleeping(bool s) { sleeping = s; }
  unsigned long GetLastMessageTime() { return lastMessageTime; }
  void SetLastMessageTime(unsigned int now=0);
};



class UserSet
{
protected:
  int count;
  int wearerIndex;
  int holderIndex;
  int botIndex;
  User user[USER_CACHE_SIZE];
//  unsigned long lastMessageTime;

public:
  UserSet() : count(0), wearerIndex(-1), holderIndex(-1), botIndex(-1) {}
  int GetCount() { return count; }
  User * GetUser(int i) { return &user[i]; }
  User * GetUserFromIndex(int i);
  User * GetUserFromId(String id);
  User * GetUserFromRole(int roleId);
  int GetIndexFromId(String id);
  User * GetWearer() { return GetUserFromIndex(wearerIndex); }
  User * GetHolder() { return GetUserFromIndex(holderIndex); }
  User * GetBot() { return GetUserFromIndex(botIndex); }
  int GetWearerIndex() { return wearerIndex; }
  String GetWearerName() { return user[wearerIndex].GetName(); }
  void SetWearerIndex(int i) { wearerIndex = i; }
  int GetHolderIndex() { return holderIndex; }
  String GetHolderName() { return user[holderIndex].GetName(); }
  void SetHolderIndex(int i) { holderIndex = i; }
  bool IdIsHolder(String id);
  void SetBotIndex(int i) { botIndex = i; }
  int AddUser(String id, String name="", int role=ROLE_GUEST, bool isBot=false);
//  unsigned long GetLastMessageTime() { return lastMessageTime; }
//  void SetLastMessageTime(unsigned long now) { lastMessageTime = now; }
  String GetUsersInfo();
  bool MayShock(String fromId);
  void Update();
};

#endif

//
