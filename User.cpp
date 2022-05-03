#include <EEPROM.h>

#include "TimeF.h"
#include "User.h"
#include "Defs.h"
#include "Message.h"
#include "Session.h"


RoleSet roles;

extern TimeFunctions timeFunc;
extern UniversalTelegramBot bot;
extern Message message;
extern Session session;
extern UserSet users;



// ------------------------------------------------------------------------
bool User::SetRoleId(int r, bool force)
{
  Serial.println("*** SetRoleId()");
  User *wearer = users.GetWearer();
  User *holder = users.GetHolder();
  switch(r)
  {
    case ROLE_WEARER_FREE:
    case ROLE_WEARER_WAITING:
    case ROLE_WEARER_CAPTURED:
      // check other users
      // if current user is the wearer or if current user may be turned into one...
      if (force ||
          (wearer && (wearer->GetId() == GetId()) || MayBecomeWearer()))
      {
        roleId = r;
        return true;
      }
      else
        return false;
      break;
    case ROLE_HOLDER:
      // check other users
      if (force ||
          (holder && (holder->GetId() == GetId()) || MayBecomeHolder()))
      {
        roleId = r;
        // get index of current user
        userSet->SetHolderIndex(userSet->GetIndexFromId(GetId()));
        return true;
      }
      else
        return false;
      break;
    case ROLE_SHOCKCELL:
      roleId = r;
      userSet->SetBotIndex(userSet->GetIndexFromId(GetId()));
      return true;
      break;
    case ROLE_TEASER:
    case ROLE_GUEST:
    default:
      roleId = r;
      return true;
      break;
  }
}


// ------------------------------------------------------------------------
bool User::UpdateRoleId(int r, bool force)
{
  Serial.println("*** UpdateRoleId()");
  // save the current roleId in order to manage the dependent states accordingly
  int oldRoleId = roleId;

  SetRoleId(r, force);

  if (roleId != oldRoleId)
    userSet->Update();
}


// ------------------------------------------------------------------------
void User::SetLastMessageTime(unsigned int now)
{
  Serial.println("*** SetLastMessageTime(" + String(now, DEC) + ")");
  Serial.println("- before: " + GetName() + " (" + GetRoleStr() + ") [" + timeFunc.GetTimeString(WITH_DATE, GetLastMessageTime()) + "]");
  if (now == 0)
    lastMessageTime = timeFunc.GetTimeInSeconds();
  else
    lastMessageTime = now;
  Serial.println("- after: " + GetName() + " (" + GetRoleStr() + ") [" + timeFunc.GetTimeString(WITH_DATE, GetLastMessageTime()) + "]");
}


// ------------------------------------------------------------------------
bool User::MayBecomeHolder()
{
  return ((IsTeaser() || IsGuest()) &&
          (! userSet->GetHolder()) &&
          (! userSet->GetWearer()->IsFreeWearer()));
}


// ------------------------------------------------------------------------
void User::SetBot(bool is)
{
  isBot = is;
  bot.getMe();
  name = bot.GetName();
  roleId = ROLE_SHOCKCELL;
}


// ------------------------------------------------------------------------
bool UserSet::IdIsHolder(String id)
{
  User *u = GetUserFromId(id);
  if (u)
    return u->IsHolder();
  else
    return false;
}


// ------------------------------------------------------------------------
int UserSet::AddUser(String id, String name, int roleId, bool isBot)
{
  Serial.println("*** UserSet::AddUser()");
  int i = 0;

  if (id.length() == 0)
  {
    Serial.println("- error: empty id string");
    return USER_NONE;
  }

  while (i < USER_CACHE_SIZE)
  {
    if (user[i].GetId().length() == 0)
    {
      // create new user
      user[i].SetUserSet(this);
      user[i].SetId(id);
      user[i].SetName(name);
      user[i].SetRoleId(roleId, true);
      Serial.print("- add ");
      Serial.print(id);
      Serial.print(", ");
      Serial.print(name);
      Serial.print(", ");
      Serial.print(roleId);
      Serial.print(", ");
      Serial.print(isBot);
      Serial.println();
      count = i + 1;
      // we need to make sure that the description is changed with every request and not identical to the existing one.
      return i;
    }
    else if (user[i].GetId() == id)
    {
      // update existing user
      user[i].SetName(name);
      user[i].UpgradeRoleId(roleId);
      Serial.print(id);
      Serial.print(", ");
      Serial.print(name);
      Serial.print(", ");
      Serial.print(roleId);
      Serial.println();
      return i;
    }
    ++i;
  }
  return USER_NONE;
}


// ------------------------------------------------------------------------
User * UserSet::GetUserFromIndex(int i)
{
  if (i >= 0)
    return &user[i];
  else
    return NULL;
}


// ------------------------------------------------------------------------
User * UserSet::GetUserFromId(String id)
{
  int i = 0;
  while (i < USER_CACHE_SIZE)
  {
    if (user[i].GetId() == id)
      return &user[i];
    ++i;
  }
  return NULL;
}


// ------------------------------------------------------------------------
User * UserSet::GetUserFromRole(int roleId)
{
  int i = 0;
  while (i < USER_CACHE_SIZE)
  {
    // only return those matches of valid users
    if ((user[i].GetId().length() > 0) &&
        (user[i].GetRoleId() == roleId))
      return &user[i];
    ++i;
  }
  return NULL;
}


// ------------------------------------------------------------------------
int UserSet::GetIndexFromId(String id)
{
  int i = 0;
  while (i < USER_CACHE_SIZE)
  {
    if (user[i].GetId() == id)
      return i;
    ++i;
  }
  return USER_NONE;
}


// ------------------------------------------------------------------------
String UserSet::GetUsersInfo()
{
  int i = 0;
  String info;
  while (i < USER_CACHE_SIZE)
  {
    if (user[i].GetId().length() > 0)
    {
      info += user[i].GetName() + ":" + user[i].GetId() + ":" + user[i].GetRoleStr() + "; ";
    }
    ++i;
  }
  return info;
}


// ------------------------------------------------------------------------
bool UserSet::MayShock(String fromId)
{
  bool may;
  User *u = users.GetUserFromId(fromId);
  if (u)
  {
    if (u->IsHolder() ||
        u->IsBot() ||
        (u->IsTeaser() && session.IsTeasingMode()) ||
        (u->IsWearer() && session.IsTeasingMode()) ||
        (users.GetWearer()->IsFreeWearer()))
      return true;
    else
      return false;
  }
  else
    return false;
}


// ------------------------------------------------------------------------
void UserSet::Update()
{
  int i = 0;
  String info;

  // clear the indices
  wearerIndex = USER_NONE;
  holderIndex = USER_NONE;
  botIndex = USER_NONE;

  // re-build the indices
  while (i < USER_CACHE_SIZE)
  {
    if (user[i].GetId().length() > 0)
    {
      if (user[i].IsWearer())
	wearerIndex = i;
      else if (user[i].IsHolder())
	holderIndex = i;
      else if (user[i].IsBot())
	botIndex = i;
    }
    ++i;
  }
  
}


//
