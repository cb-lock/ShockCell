#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "Defs.h"
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
#define BOT_COMMANDS_RANDOM "/random_5 - Switch on random shock mode with 5 shocks per hour (other intervals work with corresponding numbers)\n/random_off - Switch off random shock mode\n"
#define BOT_COMMANDS_TEASING "/teasing_on - Enable teasing\n/teasing_off - Disable teasing\n"
#define BOT_COMMANDS_UNLOCK "/unlock - Unlock key safe\n"
#define BOT_COMMANDS_ROLES "/holder - Adopt holder role\n/teaser - Adopt teaser role\n/guest - Adopt guest role\n"
#define BOT_COMMANDS_WAITING "/waiting - Make wearer waiting to be captured by the holder\n/free - Make wearer free again (stops waiting for capture)\n"
#define BOT_COMMANDS_CAPTURE "/capture - Capture wearer as a sub\n/release - Release wearer as a sub"
#define BOT_COMMANDS_EMERGENCY "/thisisanemergency - Release the wearer in case of an emergency\n"

String botCommandsAll =     BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS                                          BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES BOT_COMMANDS_WAITING BOT_COMMANDS_CAPTURE ;
String botCommandsWearer =  BOT_COMMANDS_GENERAL                                                              BOT_COMMANDS_UNLOCK                    BOT_COMMANDS_WAITING                      ;
String botCommandsHolder =  BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_RANDOM BOT_COMMANDS_TEASING BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsGuest =   BOT_COMMANDS_GENERAL                                                                                  BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsTeaser =  BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS                                                              BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsAllHelp = BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS                                          BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES BOT_COMMANDS_WAITING BOT_COMMANDS_CAPTURE BOT_COMMANDS_EMERGENCY ;


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
waiting - Make wearer waiting to be captured by the holder
free - Make wearer free again (stops waiting for capturing by the holder)
capture - Capture wearer as a sub
release - Release wearer as a sub
 */



// ------------------------------------------------------------------------
Message::Message()
{
}


// ------------------------------------------------------------------------
void Message::Init()
{
  Serial.println("*** Message::Init()");
  // UserSet::Init() must have already be run!
  // populate values with defaults...
  session.SetTimeOfLastShock(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastUnlock(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastOpening(timeFunc.GetTimeInSeconds());
  session.SetTimeOfLastClosing(timeFunc.GetTimeInSeconds());

  int index;
  index = users.AddUser(USER_ID_WEARER, USER_NAME_WEARER, ROLE_WEARER_FREE);
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

  String commands = botCommandsAll;
  Serial.println(commands);
  int pos = 0;
  while ((pos = commands.indexOf('/')) != -1)
    commands.remove(pos, 1);
  bot.setMyCommands(commands);
}


// ------------------------------------------------------------------------
void Message::MessageLastShock(String chatId)
{
  SendMessage("Last shock was " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastShock()) + " ago.", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageIsClosed(String chatId)
{
  SendMessage("Key safe is safely closed and locked since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastClosing()) + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageIsOpen(String chatId)
{
  SendMessage("Key safe is open since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastOpening()) + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageCoverStateChange(String chatId)
{
  if (coverState == COVER_CLOSED)
  {
    session.SetTimeOfLastClosing(timeFunc.GetTimeInSeconds());
    UpdateChatDescription();
    SendMessage("Key safe has just been safely closed.", chatId);
  }
  else
  {
    session.SetTimeOfLastOpening(timeFunc.GetTimeInSeconds());
    UpdateChatDescription();
    SendMessage("Key safe has just been opened.", chatId);
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
  SendMessage(msg, chatId);
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
  SendMessage(msg, chatId);
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
  SendMessage(msg, chatId);
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
void Message::ShockAction(String fromId, String chatId, int count, long milliseconds)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u)
  {
    if (u->IsHolder() ||
        u->IsBot() ||
        (u->IsTeaser() && session.IsTeasingMode()))
    {
      String msg = "Shock processing for " + String((milliseconds / 1000), DEC) + " s begins...";
      SendMessage(msg, chatId);
      session.SetTimeOfLastShock(timeFunc.GetTimeInSeconds());
      UpdateChatDescription();

      session.Shock(count, milliseconds);  

      msg = "Shock processing completed. :-)";
      SendMessage(msg, chatId);
      UpdateChatDescription();
    }
    else
    {
      String msg = "Request denied. Only the holder";
      if (session.IsTeasingMode())
        msg += " and the teasers";
      msg += " are allowed to execute shock punishments.";
      SendMessage(msg, chatId);
    }
  }
}



#define COMMON_MSG_WAITING " The holder can capture him with the /capture command."
#define COMMON_MSG_CAPTURE " He is now securely locked and lost his permissions to open the key safe. The key safe can only be operated by the holder using the /unlock command.\nFurthermore, the wearer can now be punished with shocks."


// ------------------------------------------------------------------------
void Message::WaitingAction(String fromId, String chatId)
{
  String msg;

  // does request comes from free wearer?
  if (users.GetUserFromId(fromId)->IsFreeWearer())
  {
    users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
    msg = "Wearer " + users.GetWearerName() + " is now exposed and waiting to be captured by the holder." COMMON_MSG_WAITING;
    SendMessage(msg, chatId);
    UpdateChatDescription();
  }
  // does request comes from waiting wearer?
  else if (users.GetUserFromId(fromId)->IsWaitingWearer())
  {
    msg = "Wearer " + users.GetWearerName() + " is already waiting for capture by the holder." COMMON_MSG_WAITING;
    SendMessage(msg, chatId);
  }
  // does request comes from captured wearer?
  else if (users.GetUserFromId(fromId)->IsCapturedWearer())
  {
    msg = "Wearer " + users.GetWearerName() + " is already captured by the holder.";
    SendMessage(msg, chatId);
  }
  else
  {
    msg = "Only the free wearer can use the command /waiting to make himself available for capturing by a holder.";
    SendMessage(msg, chatId);
  }
}


// ------------------------------------------------------------------------
void Message::FreeAction(String fromId, String chatId)
{
  String msg;

  // does request comes from waiting wearer?
  if (users.GetUserFromId(fromId)->IsWaitingWearer())
  {
    users.GetWearer()->SetRoleId(ROLE_WEARER_FREE);
    msg = "Wearer " + users.GetWearerName() + " is now free and cannot be captured by the holder.";
    SendMessage(msg, chatId);
    UpdateChatDescription();
  }
  // does request comes from free wearer?
  else if (users.GetUserFromId(fromId)->IsFreeWearer())
  {
    msg = "Wearer " + users.GetWearerName() + " is already free.";
    SendMessage(msg, chatId);
  }
  // does request comes from captured wearer?
  else if (users.GetUserFromId(fromId)->IsCapturedWearer())
  {
    msg = "Request is denied! Wearer " + users.GetWearerName() + " is captured by the holder. He must be released by the holder to get free.";
    SendMessage(msg, chatId);
  }
  else
  {
    msg = "Only the waiting wearer can use the command /free to be safe from capturing by the holder.";
    SendMessage(msg, chatId);
  }
}


// ------------------------------------------------------------------------
void Message::CaptureAction(String fromId, String chatId)
{
  String msg;
  if (users.GetUserFromId(fromId)->IsHolder() ||
      (! users.GetHolder() && users.GetUserFromId(fromId)->MayBecomeHolder()))
  {
    // request is from holder
    // or from other user when there is no holder present yet
    if (users.GetWearer()->IsWaitingWearer())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_CAPTURED);
      msg = "Capturing wearer " + users.GetWearerName() + " now." COMMON_MSG_CAPTURE;
      // Make requestor a holder if necessary
      if (! users.GetUserFromId(fromId)->IsHolder())
      {
        msg += " User " + users.GetUserFromId(fromId)->GetName() + " is now holder.";
        users.GetUserFromId(fromId)->SetHolder();
      }
      UpdateChatDescription();
    }
    else if (users.GetWearer()->IsCapturedWearer())
    {
      msg = "Wearer " + users.GetWearerName() + " is already captured." COMMON_MSG_CAPTURE;
    }
    else
    {
      msg = "Wearer " + users.GetWearerName() + " is currently not waiting to get captured. The wearer must send the /waiting command to await capture by the holder.";
    }
    SendMessage(msg, chatId);
  }
  else
  {
    if (users.GetWearer()->IsFreeWearer())
      SendMessage("You currently cannot capture the wearer, because he has not made himself available. The wearer must be waiting for capture using the /waiting command.", chatId);
    else
      SendMessage("You currently have no permission to capture the wearer. Guests may capture the wearer only, if there is no current holder.", chatId);
  }
}


// ------------------------------------------------------------------------
void Message::ReleaseAction(String fromId, String chatId)
{
  String msg;
  if (users.GetUserFromId(fromId)->IsHolder())
  {
    // request is from holder
    if (users.GetWearer()->IsCapturedWearer())
    {
      users.GetWearer()->SetRoleId(ROLE_WEARER_WAITING);
      msg = "Releasing wearer " + users.GetWearerName() + ". He is now released and received back his permissions to open the key safe.\n";
      UpdateChatDescription();
    }
    else
    {
      msg = "Wearer " + users.GetWearerName() + " is currently not captured and there is no need to release him.";
    }
    SendMessage(msg, chatId);
  }
  else
  {
    if (users.GetUserFromId(fromId)->IsCapturedWearer())
      SendMessage("The wearer cannot free himself. Only the holder may release the wearer.", chatId);
    else
      SendMessage("You currently have no permission to release the wearer. Only the holder may release the wearer.", chatId);
  }
}


// ------------------------------------------------------------------------
void Message::UnlockAction(String fromId, String chatId, bool force)
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
  if (session.IsActiveChastikeySession() && ! force)
  {
    SendMessageAll("Unlock request is denied! Wearer is locked in an active ChastiKey session.", chatId);
  }
  else if (users.GetUserFromId(fromId)->MayUnlock() || force)
  {
    if (force)
      SendMessageAll("An emergency release request has been granted.", chatId);
    SendMessageAll("Key safe is unlocking now for 4 seconds and may be opened during this period.", chatId);
    session.Unlock();
    // check state of the safe afterwards
    coverState = digitalRead(COVER_OPEN_PIN);
    if (coverState == COVER_OPEN)
    {
      SendMessageAll("Key safe has been opened.", chatId);
      UpdateChatDescription();
    }
    else
      SendMessageAll("Key safe is locked again and has not been opened.", chatId);
  }
  else
  {
    SendMessageAll("You have no permission to unlock. Only the free wearer or the holder may unlock the key safe.", chatId);
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
    msg += "The holder can use the following commands:\n" + botCommandsHolder;
    SendMessage(msg, chatId);
    UpdateChatDescription();
  }
  else
  {
    if (u->IsHolder())
    {
      msg = "User " + users.GetUserFromId(fromId)->GetName() + " is already holder.";
      msg += "The holder can use the following commands:\n" + botCommandsHolder;
    }
    else
      msg = "User " + users.GetUserFromId(fromId)->GetName() + " has no permission to become holder.";
    if (u->IsWearer())
      msg += " The wearer is generally not allowed to adopt the holder role.";
    SendMessage(msg, chatId);
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
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer captured. He is now released and received back his permissions to open the key safe.";
    }
    u->SetRoleId(ROLE_TEASER);
    SendMessage(msg, chatId);
    UpdateChatDescription();
  }
  else
  {
    msg = "User " + users.GetUserFromId(fromId)->GetName() + " has no permission to become teaser.";
    SendMessage(msg, chatId);
  }
}


// ------------------------------------------------------------------------
void Message::TeasingModeAction(bool mode, String fromId, String chatId)
{
  if (users.GetHolder()->GetId() == fromId)
  {
    session.SetTeasingMode(mode);
    if (mode)
      SendMessage("Teasing mode is now activated. Users with teaser role can now support the holder with treatments like /shock_5.", chatId);
    else
      SendMessage("Teasing mode is now switched off. Only the holder has the permission to treat the wearer with shocks.", chatId);
  }
  else
  {
    SendMessage("Request denied. Only the holder has the permission to control the rights for users with the teaser role.", chatId);
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
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer captured. He is now released and received back his permissions to open the key safe.";
    }
    u->SetRoleId(ROLE_GUEST);
    SendMessage(msg, chatId);
    UpdateChatDescription();
  }
}


// ------------------------------------------------------------------------
void Message::RestrictUserAction(String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u)
  {
    if (u->IsHolder())
    {
      long now = timeFunc.GetTimeInSeconds();
      String untilDate = timeFunc.Time2String(now + 3600);
      msg = "Restricting user " + users.GetWearer()->GetName() + " in chat (" + (!mutedWearer ? "unmuted" : "muted") + ") until " + untilDate + ".";
      bot.restrictChatMember(chatId, users.GetWearer()->GetId(), ! mutedWearer, untilDate);
      mutedWearer = ! mutedWearer;
    }
    else
    {
      msg = "Only the holder may apply restrictions.";
    }
    SendMessage(msg, chatId);
  }
}


// ------------------------------------------------------------------------
void Message::RandomShockModeAction(String commandParameter, String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u)
  {
    if (u->IsHolder())
    {
      int shocksPerHour = commandParameter.toInt();
      if (commandParameter == "off")
      {
        session.SetRandomMode(false);
        SendMessage("Switching random mode off.", chatId);
      }
      else
      {
        if (shocksPerHour > 0)
        {
          session.SetRandomMode(true, shocksPerHour);
          SendMessage("Switching random mode on with " + commandParameter + " shocks per hour on average. This remains active until it is switched off or sleeping time of wearer " + users.GetWearer()->GetName() + " begins.", chatId);
        }
        else
        {
          SendMessage("Error: invalid number of shocks per hour provided: " + commandParameter, chatId);
        }
      }
    }
    else
    {
      SendMessage("User " + u->GetName() + " has no rights to set the random mode. Only the holder is allowed to do this.");
    }
  }
}


// ------------------------------------------------------------------------
void Message::EmergencyAction(String fromId, String chatId)
{
  if (users.GetWearer()->GetId() == fromId)
  {
    session.SetEmergencyReleaseCounterRequest(true);
    SendMessageAll("An emergency release request has been received and will be processed within 5 minutes.", chatId);
  }
  else
    SendMessage("Only the wearer may request for emergency release.", chatId);
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
          case ROLE_WEARER_CAPTURED:
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
        SendMessage(welcome, chat_id);
      }
      else if (text == "/shock1")
        ShockAction(from_id, chat_id, 1, 1000);
      else if (text == "/shock3")
        ShockAction(from_id, chat_id, 1, 3000);
      else if (text == "/shock5")
        ShockAction(from_id, chat_id, 1, 5000);
      else if (text == "/shock10")
        ShockAction(from_id, chat_id, 1, 10000);
      else if (text == "/shock30")
        ShockAction(from_id, chat_id, 1, 30000);
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
      else if (text == "/teasing_on")
        TeasingModeAction(true, from_id, chat_id);
      else if (text == "/teasing_off")
        TeasingModeAction(false, from_id, chat_id);
      else if (text == "/guest")
        GuestAction(from_id, chat_id);
      else if (text == "/waiting")
        WaitingAction(from_id, chat_id);
      else if (text == "/capture")
        CaptureAction(from_id, chat_id);
      else if (text == "/release")
        ReleaseAction(from_id, chat_id);
      else if (text == "/free")
        FreeAction(from_id, chat_id);
      else if (text.substring(0, 7) == "/random")
        RandomShockModeAction(text.substring(8), from_id, chat_id);
//      else if (text == "/restrict")
//        RestrictUserAction(from_id, chat_id);
      else if (text == "/thisisanemergency")
        EmergencyAction(from_id, chat_id);
      else if (text.substring(0, 1) == "/")
        UnknownCommand(chat_id);
      else
        ProcessChatMessage(text, from_id, chat_id);
    }
    newMessageCount = bot.getUpdates(bot.last_message_received + 1);
  }
}



// -------------------------------------------------
void Message::SendMessage(String msg, String chatId)
{
  Serial.print(chatId);
  Serial.print("> ");
  Serial.println(msg);
  bot.sendMessage(chatId, msg, "");
}


// -------------------------------------------------
void Message::SendMessageAll(String msg, String chatId)
{
  SendMessage(msg, chatId);
  if (chatId != GROUP_CHAT_ID)
    SendMessage(msg, GROUP_CHAT_ID);
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
