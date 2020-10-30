#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "Message.h"
#include "EServer.h"
#include "TimeF.h"
#include "User.h"
#include "Session.h"


extern EmlaServer emlaServer;
extern TimeFunctions timeFunc;
extern OledDisplay oledDisplay;
extern int coverState;
extern int oldCoverState;
extern UserSet users;
extern Session session;



// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you

WiFiClientSecure clientsec;
UniversalTelegramBot bot(BOT_TOKEN, clientsec);

// Checks for new messages every 1 second.

#define BOT_COMMANDS_GENERAL "/start - Start communication\n/state - Report the cover state\n/roles - List roles\n/users - List users in chat\n"
#define BOT_COMMANDS_SHOCKS "/shock1 - Shock for 1 seconds\n/shock3 - Shock for 3 seconds\n/shock5 - Shock for 5 seconds\n/shock10 - Shock for 10 seconds\n/shock30 - Shock for 30 seconds\n"
#define BOT_COMMANDS_UNLOCK "/unlock - Unlock key safe\n"
#define BOT_COMMANDS_ROLES "/holder - Adopt holder role\n/teaser - Adopt teaser role\n/guest - Adopt guest role\n"
#define BOT_COMMANDS_WAITING "/waiting - Make wearer waiting to be collared by the holder\n/free - Make wearer free again (stops waiting for collaring)\n"
#define BOT_COMMANDS_COLLAR "/collar - Collar and capture wearer as a sub\n/release - Release wearer as a sub (open collar)"

String botCommandsAll =    BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES BOT_COMMANDS_WAITING BOT_COMMANDS_COLLAR ;
String botCommandsWearer = BOT_COMMANDS_GENERAL                     BOT_COMMANDS_UNLOCK                    BOT_COMMANDS_WAITING                     ;
String botCommandsHolder = BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES                      BOT_COMMANDS_COLLAR ;
String botCommandsGuest  = BOT_COMMANDS_GENERAL                                         BOT_COMMANDS_ROLES                      BOT_COMMANDS_COLLAR ;
String botCommandsTeaser = BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS                     BOT_COMMANDS_ROLES                      BOT_COMMANDS_COLLAR ;


/*
start - Start communication
roles - List roles
shock1 - Shock for 1 seconds
shock3 - Shock for 3 seconds
shock5 - Shock for 5 seconds
shock10 - Shock for 10 seconds
shock30 - Shock for 30 seconds
state - Report the cover state
unlock - Unlock key safe
users - Lists users in chat
holder - Adopt holder role
teaser - Adopt teaser role
guest - Adopt guest role
waiting - Make wearer waiting to be collared by the holder
free - Make wearer free again (stops waiting for collaring)
collar - Collar and capture wearer as a sub
release - Release wearer as a sub (open collar)
 */



// ------------------------------------------------------------------------
Message::Message()
{
}


// ------------------------------------------------------------------------
void Message::Init()
{
  // UserSet::Init() must have already be run!
  // populate values with defaults...
  session.SetTimeOfLastShock(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastUnlock(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastOpening(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastClosing(timeFunc.GetTimeInSeconds());

  int index;
  index = users.AddUser(USER_ID_CHARLY, "Charly", ROLE_WEARER_FREE);
  users.SetWearerIndex(index);

  index = users.AddUser(USER_ID_BOT, "Ruler", ROLE_SHOCKCELL);
  users.GetUser(index)->SetBot(true);

  String description;
  if (bot.getChatDescription(GROUP_CHAT_ID, description))
  {
    AdoptChatDescription(description);
  }
  else
  {
    // this is only needed for the initial setup of a chat group
    UpdateChatDescription();
  }

  bot.setMyCommands(botCommandsAll);
}


// ------------------------------------------------------------------------
void Message::MessageLastShock(String chatId)
{
  SendMessage(chatId, "Last shock was " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastShock()) + " ago.");
}


// ------------------------------------------------------------------------
void Message::MessageIsClosed(String chatId)
{
  SendMessage(chatId, "Key safe is safely closed and locked since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastClosing()) + ".");
}


// ------------------------------------------------------------------------
void Message::MessageIsOpen(String chatId)
{
  SendMessage(chatId, "Key safe is open since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastOpening()) + ".");
}


// ------------------------------------------------------------------------
void Message::MessageCoverStateChange(String chatId)
{
  if (coverState == COVER_CLOSED)
  {
    session.SetTimeOfLastClosing(timeFunc.GetTimeInSeconds());
    UpdateChatDescription();
    SendMessage(chatId, "Key safe has just been safely closed.");
  }
  else
  {
    session.SetTimeOfLastOpening(timeFunc.GetTimeInSeconds());
    UpdateChatDescription();
    SendMessage(chatId, "Key safe has just been opened.");
  }
}


// ------------------------------------------------------------------------
void Message::MessageCoverState(String chatId)
{
  coverState = digitalRead(COVER_OPEN_PIN);
  if (coverState == COVER_CLOSED)
    MessageIsClosed(chatId);
  else
    MessageIsOpen(chatId);
}


// ------------------------------------------------------------------------
void Message::MessageUsers(String chatId)
{
  int i = 0;
  String msg = "Users in chat:\n";
  while (i < USER_CACHE_SIZE)
  {
    String name = users.GetUser(i)->GetName();
    if (name.length() > 0)
      msg += "- " + name + " (" + users.GetUser(i)->GetRoleStr() + ")\n";
    ++i;
  }
  SendMessage(chatId, msg);
//  SendRichMessage(chatId, "inline_keyboard: [\n[{ text: 'Some button text 1', callback_data: '1' }],\n[{ text: 'Some button text 2', callback_data: '2' }],\n[{ text: 'Some button text 3', callback_data: '3' }]\n]");
}


// ------------------------------------------------------------------------
void Message::MessageRoles(String chatId)
{
  Serial.println("*** Message::MessageRoles()");
  String msg = "Assigned roles:\n";
  User *user;
  for (int r = 0; r < ROLE_COUNT; r++)
  {
    Serial.println(r);
    user = users.GetUserFromRole(r);
    if (user)
    {
      String name = user->GetName();
      msg += "- " + user->GetRoleStr() + ": " + name + "\n";
      Serial.println(msg);
    }
  }
  SendMessage(chatId, msg);
}


// ------------------------------------------------------------------------
void Message::MessageChastikeyState(String chatId)
{
  String msg;
  if (session.IsActiveChastikeySession())
  {
    msg = "Wearer " + users.GetWearer()->GetName() + " is locked in a ChastiKey session and controlled by holder " + session.GetChastikeyHolder() + ". The key safe remains securely locked as long as this session is active.";
  }
  else
  {
    msg = "Wearer " + users.GetWearer()->GetName() + " is not locked in a ChastiKey session.";
  }
  SendMessage(chatId, msg);
}


// ------------------------------------------------------------------------
void Message::MessageState(String chatId)
{
  MessageLastShock(chatId);
  MessageCoverState(chatId);
  MessageUsers(chatId);
  MessageChastikeyState(chatId);
}


// ------------------------------------------------------------------------
void Message::ShockAction(String chatId, int count, long milliseconds)
{
  String msg = "Shock processing for " + String((milliseconds / 1000), DEC) + " s begins...";
  session.SetTimeOfLastShock(timeFunc.GetTimeInSeconds());
  UpdateChatDescription();
  SendMessage(chatId, msg);

  session.Shock(count, milliseconds);  

  msg = "Shock processing completed. :-)";
  SendMessage(chatId, msg);
  UpdateChatDescription();
}



#define COMMON_MSG_WAITING "The holder can capture him with the /collar command."
#define COMMON_MSG_COLLAR "He is now securely locked and lost his permissions to open the key safe. The key safe can only be operated by the holder using the /unlock command.\nFurthermore, the wearer can now be punished with shocks."


// ------------------------------------------------------------------------
void Message::WaitingAction(String fromId, String chatId)
{
  String msg;

  // does request comes from free wearer?
  if (users.GetUserFromId(fromId)->IsFreeWearer())
  {
    users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
    msg = "User " + users.GetWearerName() + " is now exposed and waiting to be collared by the holder." COMMON_MSG_WAITING;
    SendMessage(chatId, msg);
    UpdateChatDescription();
  }
  // does request comes from waiting wearer?
  else if (users.GetUserFromId(fromId)->IsWaitingWearer())
  {
    msg = "User " + users.GetWearerName() + " is already waiting for capture by the holder." COMMON_MSG_WAITING;
    SendMessage(chatId, msg);
  }
  // does request comes from collared wearer?
  else if (users.GetUserFromId(fromId)->IsCollaredWearer())
  {
    msg = "User " + users.GetWearerName() + " is already collared and captured by the holder.";
    SendMessage(chatId, msg);
  }
  else
  {
    msg = "Only the free wearer can use the command /waiting to make himself available for collaring by a holder.";
    SendMessage(chatId, msg);
  }
}


// ------------------------------------------------------------------------
void Message::CollarAction(String fromId, String chatId)
{
  String msg;
  if (users.GetUserFromId(fromId)->IsHolder() ||
      (!users.GetHolder() && users.GetUserFromId(fromId)->MayBecomeHolder()))
  {
    // request is from holder
    // or from other user when there is no holder present yet
    if (users.GetWearer()->IsWaitingWearer())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_COLLARED);
      msg = "Collaring wearer " + users.GetWearerName() + " now. " COMMON_MSG_COLLAR;
      // Make requestor a holder if necessary
      if (! users.GetUserFromId(fromId)->IsHolder())
      {
        msg += " User " + users.GetUserFromId(fromId)->GetName() + " is now holder.";
        users.GetUserFromId(fromId)->SetHolder();
      }
      UpdateChatDescription();
    }
    else if (users.GetWearer()->IsCollaredWearer())
    {
      msg = "Wearer " + users.GetWearerName() + " is already collared. " COMMON_MSG_COLLAR;
    }
    else
    {
      msg = "Wearer " + users.GetWearerName() + " is currently not waiting to get collared. The wearer must send the /waiting command to await collaring and getting captured.";
    }
    SendMessage(chatId, msg);
  }
  else
  {
    SendMessage(chatId, "You currently have no permission to collar the wearer. Guests may collar the wearer only, if there is no current holder.");
  }
}


// ------------------------------------------------------------------------
void Message::ReleaseAction(String fromId, String chatId)
{
  String msg;
  if (users.GetUserFromId(fromId)->IsHolder())
  {
    // request is from holder
    if (users.GetWearer()->IsCollaredWearer())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
      msg = "Releasing wearer " + users.GetWearerName() + ". He is now released and received back his permissions to open the key safe.\n";
      UpdateChatDescription();
    }
    else
    {
      msg = "Wearer " + users.GetWearerName() + " is currently not collared and there is no need to release him.";
    }
    SendMessage(chatId, msg);
  }
  else
  {
    if (users.GetUserFromId(fromId)->IsCollaredWearer())
      SendMessage(chatId, "The wearer cannot free himself. Only the holder may release the wearer.");
    else
      SendMessage(chatId, "You currently have no permission to release the wearer. Only the holder may release the wearer.");
  }
}


// ------------------------------------------------------------------------
void Message::UnlockAction(String fromId, String chatId)
{
  Serial.println("*** Message::UnlockAction()");
  Serial.print("- free: ");
  Serial.println(users.GetUserFromId(fromId)->IsFreeWearer());
  Serial.print("- waiting: ");
  Serial.println(users.GetUserFromId(fromId)->IsWaitingWearer());
  Serial.print("- holder: ");
  Serial.println(users.GetUserFromId(fromId)->IsHolder());

  // from wearer & wearer is free
  // from holder
  if (session.IsActiveChastikeySession())
  {
    SendMessage(chatId, "Unlock request is denied! Wearer is locked in an active ChastiKey session.");
  }
  else if (users.GetUserFromId(fromId)->MayUnlock())
  {
    SendMessage(chatId, "Key safe is unlocking now for 4 seconds and may be opened during this period.");
    session.Unlock();
    // check state of the safe afterwards
    coverState = digitalRead(COVER_OPEN_PIN);
    if (coverState == COVER_OPEN)
    {
      SendMessage(chatId, "Key safe has been opened.");
      UpdateChatDescription();
    }
    else
      SendMessage(chatId, "Key safe is locked again and has not been opened.");
  }
  else
  {
    SendMessage(chatId, "You have no permission to unlock. Only the free wearer or the holder may unlock the key safe.");
  }
}


// ------------------------------------------------------------------------
void Message::HolderAction(String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u && u->MayBecomeHolder())
  {
    u->SetRoleId(ROLE_HOLDER);
    msg = "User " + users.GetUserFromId(fromId)->GetName() + " has now holder permissions.";
    SendMessage(chatId, msg);
    UpdateChatDescription();
  }
  else
  {
    if (u->IsHolder())
      msg = "User " + users.GetUserFromId(fromId)->GetName() + " is already holder.";
    else
      msg = "User " + users.GetUserFromId(fromId)->GetName() + " has no permission to become holder.";
    if (u->IsWearer())
      msg += " The wearer is generally not allowed to adopt the holder role.";
    SendMessage(chatId, msg);
  }
}


// ------------------------------------------------------------------------
void Message::TeaserAction(String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u && u->MayBecomeTeaser())
  {
    msg = "User " + users.GetUserFromId(fromId)->GetName() + " has now teaser permissions.";
    if (u->IsHolder())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer collared. He is now released and received back his permissions to open the key safe.";
    }
    u->SetRoleId(ROLE_TEASER);
    SendMessage(chatId, msg);
    UpdateChatDescription();
  }
  else
  {
    msg = "User " + users.GetUserFromId(fromId)->GetName() + " has no permission to become teaser.";
    SendMessage(chatId, msg);
  }
}


// ------------------------------------------------------------------------
void Message::GuestAction(String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u)
  {
    msg = "User " + users.GetUserFromId(fromId)->GetName() + " has now guest permissions.";
    if (u->IsHolder())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer collared. He is now released and received back his permissions to open the key safe.";
    }
    u->SetRoleId(ROLE_GUEST);
    SendMessage(chatId, msg);
    UpdateChatDescription();
  }
}


// ------------------------------------------------------------------------
void Message::RestrictUserAction(String fromId, String chatId)
{
  long now = timeFunc.GetTimeInSeconds();
  String untilDate = timeFunc.Time2String(now + 3600);
  String msg = "Restricting user " + users.GetWearer()->GetName() + " in chat (" + (!mutedWearer ? "unmuted" : "muted") + ") until " + untilDate + ".";
  bot.restrictChatMember(chatId, users.GetWearer()->GetId(), ! mutedWearer, untilDate);
  mutedWearer = ! mutedWearer;
  SendMessage(chatId, msg);
}


// -------------------------------------------------
void Message::UnknownCommand(String chatId)
{
  bot.sendMessage(chatId, "Error: unknown command.");
}


// -------------------------------------------------
void Message::ProcessChatMessage(String msg, String fromId, String chatId)
{
  Serial.println("*** ProcessChatMessage()");
  Serial.println(msg);
}


// -------------------------------------------------
void Message::ProcessNewMessages()
{
  int newMessageCount = bot.getUpdates(bot.last_message_received + 1);

//  Serial.print("*** Message::ProcessNewMessages(");
//  Serial.print(String(newMessageCount));
//  Serial.println(")");
  while(newMessageCount)
  {
    Serial.println("Message received.");
    for (int i = 0; i < newMessageCount; i++)
    {
      // Chat id of the requester
      String chat_id = String(bot.messages[i].chat_id);
      Serial.print("Chat ID: ");
      Serial.println(chat_id);

      // Print the received message
      String from_name = bot.messages[i].from_name;
      String from_id = bot.messages[i].from_id;
      Serial.print(from_name);
      Serial.print(": ");
      String text = bot.messages[i].text;
      Serial.println(text);

      if (text.substring(text.length() - 14) == "@shockcell_bot")
        text = text.substring(0, text.length() - 14);
      Serial.println(text);

      users.AddUser(from_id, from_name);

      // execute commands
      if (text == "/start")
      {
        String welcome = "Welcome, " + from_name + ", use the following commands to control " + users.GetWearer()->GetName() + "'s ShockCell.\n\n";
        welcome += "Commands for the ";
        switch (users.GetUserFromId(from_id)->GetRoleId())
        {
          case ROLE_WEARER_COLLARED:
          case ROLE_WEARER_WAITING:
          case ROLE_WEARER_FREE:
            welcome += "wearer:\n" + botCommandsWearer;
            break;
          case ROLE_HOLDER:
            welcome += "holder:\n" + botCommandsHolder;
            break;
          case ROLE_TEASER:
            welcome += "teaser:\n" + botCommandsTeaser;
            break;
          case ROLE_GUEST:
            welcome += "guest:\n" + botCommandsGuest;
            break;
        }
        SendMessage(chat_id, welcome);
      }
      else if (text == "/shock1")
        ShockAction(chat_id, 1, 1000);
      else if (text == "/shock3")
        ShockAction(chat_id, 1, 3000);
      else if (text == "/shock5")
        ShockAction(chat_id, 1, 5000);
      else if (text == "/shock10")
        ShockAction(chat_id, 1, 10000);
      else if (text == "/shock30")
        ShockAction(chat_id, 1, 30000);
      else if (text == "/state")
      {
        session.InfoChastikey();
        MessageState(chat_id);
      }
      else if (text == "/unlock")
        UnlockAction(from_id, chat_id);
      else if (text == "/users")
        MessageUsers(chat_id);
      else if (text == "/roles")
        MessageRoles(chat_id);
      else if (text == "/holder")
        HolderAction(from_id, chat_id);
      else if (text == "/teaser")
        TeaserAction(from_id, chat_id);
      else if (text == "/guest")
        GuestAction(from_id, chat_id);
      else if (text == "/waiting")
        WaitingAction(from_id, chat_id);
      else if (text == "/collar")
        CollarAction(from_id, chat_id);
      else if ((text == "/release") || (text == "/free"))
        ReleaseAction(from_id, chat_id);
      else if (text == "/restrict")
        RestrictUserAction(from_id, chat_id);
      else if (text.substring(0, 1) == "/")
        UnknownCommand(chat_id);
      else
        ProcessChatMessage(text, from_id, chat_id);
    }
    newMessageCount = bot.getUpdates(bot.last_message_received + 1);
  }
}



// -------------------------------------------------
void Message::SendMessage(String chatId, String msg)
{
  Serial.print(chatId);
  Serial.print("> ");
  Serial.println(msg);
  bot.sendMessage(chatId, msg, "");
}


// -------------------------------------------------
void Message::SendRichMessage(String chatId, String msg)
{
  Serial.print(chatId);
  Serial.print("> ");
  Serial.println(msg);
  bot.sendMessage(chatId, msg, "MarkdownV2");
}



// -------------------------------------------------
void Message::SetChatDescription(String chatId, String descr)
{
  bot.setChatDescription(chatId, descr);
}



// -------------------------------------------------
long Message::ReadParamLong(String text, String id)
{
/*
  Serial.println("ReadParamLong()");
  Serial.print(text);
  Serial.println();
  */
  int paramPos = text.indexOf(id);
  text = text.substring(paramPos);
//  Serial.print(text);
//  Serial.println();
  int nextColon = text.indexOf(':');
//  Serial.print(nextColon);
//  Serial.println();
  int nextSemi = text.indexOf(';');
//  Serial.print(nextSemi);
//  Serial.println();
  String value = text.substring(nextColon + 1, nextSemi);
/*
  Serial.println("=============");
  Serial.println(value);
  Serial.println("=============");
  */
  return atol(value.c_str());
}


// -------------------------------------------------
void Message::AdoptUserInfos(String text)
{
  // USERS: Charly:1157999292:Wearer; ShockCell:1264046045:ShockCell; S:919525040:Guest;
  Serial.println("AdoptUserInfos()");
  Serial.print(text);
  Serial.println();
  int userPos = text.indexOf(USERS_PREFIX) + String(USERS_PREFIX).length();
  Serial.print(userPos);
  Serial.println();
  int nextColon, nextSemi;
  int roleId;
  bool isBot;
  String name, id, role;
  while(userPos >= 0)
  {
    // skip space ' '
    userPos++;
    // name
    text = text.substring(userPos);
    nextColon = text.indexOf(':');
    name = text.substring(0, nextColon);
    Serial.print(name);
    Serial.println();
    // id
    text = text.substring(nextColon + 1);
    nextColon = text.indexOf(':');
    id = text.substring(0, nextColon);
    Serial.print(id);
    Serial.println();
    // role
    text = text.substring(nextColon + 1);
    nextSemi = text.indexOf(';');
    role = text.substring(0, nextSemi);
    Serial.print(role);
    Serial.println();

    isBot = (role == "ShockCell");
    roleId = roles.GetRoleId(role);
    users.AddUser(id, name, roleId, isBot);
    userPos = text.indexOf(' ');
  }
}


// -------------------------------------------------
void Message::AdoptChatDescription(String descr)
{
  // LastOpening:1598445166; LastClosing:1598445167; LastShock:1598445168; USERS: Charly:1157999292:Wearer; ShockCell:1264046045:ShockCell; S:919525040:Guest;
  session.SetTimeOfLastOpening(ReadParamLong(descr, "LastOpening"));
  session.SetTimeOfLastClosing(ReadParamLong(descr, "LastClosing"));
  session.SetTimeOfLastShock(ReadParamLong(descr, "LastShock"));
//  info += users.GetUsersInfo();
  AdoptUserInfos(descr);
}


// -------------------------------------------------
void Message::UpdateChatDescription()
{
  Serial.println("*** UpdateChatDescription()");
  String info;
  info = "LastOpening:" + String(session.GetTimeOfLastOpening(), DEC) + "; ";
  info += "LastClosing:" + String(session.GetTimeOfLastClosing(), DEC) + "; ";
  info += "LastShock:" + String(session.GetTimeOfLastShock(), DEC) + "; ";
  info += USERS_PREFIX " ";
  info += users.GetUsersInfo();

  // we need to make sure that the description is changed with every request and not identical to the existing one.
//  SetDescription(GROUP_CHAT_ID, "-");
  if (info != lastChatDescription)
  {
    SetChatDescription(GROUP_CHAT_ID, info);
    lastChatDescription = info;
  }
}


//
