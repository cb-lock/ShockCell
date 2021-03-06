#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "Defs.h"
#include "Message.h"
#include "EServer.h"
#include "TimeF.h"
#include "User.h"
#include "Chat.h"
#include "Session.h"


extern EmlaServer emlaServer;
extern TimeFunctions timeFunc;
extern OledDisplay oledDisplay;
extern int coverState;
extern int oldCoverState;
extern ChatSet chats;
extern UserSet users;
extern Session session;
extern Verification verification;
extern Tasklist tasklist;



// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you

WiFiClientSecure clientsec;
UniversalTelegramBot bot(BOT_TOKEN, clientsec);

// Checks for new messages every 1 second.

#define BOT_COMMANDS_GENERAL "/start - Start communication\n/state - Report the cover state\n/roles - List roles\n/users - List users in chat\n/play_6 - Throw dice (any number of possibilities with corresponding number)\n"
#define BOT_COMMANDS_SHOCKS "/shock_1 - Shock for 1 second (other intervals work with corresponding numbers)\n/shock_5 - Shock for 5 seconds (other intervals work with corresponding numbers)\n/shock_10 - Shock for 10 seconds (other intervals work with corresponding numbers)\n/shock_30 - Shock for 30 seconds (other intervals work with corresponding numbers)\n"
#define BOT_COMMANDS_RANDOM "/random_5 - Switch on random shock mode with 5 shocks per hour (other intervals work with corresponding numbers)\n/random_off - Switch off random shock mode\n"
#define BOT_COMMANDS_TEASING "/teasing_on - Enable teasing\n/teasing_off - Disable teasing\n/verify_1 - Enable verification mode with 1 check-in per day\n/verify_2 - Enable verification mode with 2 check-ins per day\n/verify_off - Disable verification mode\n"
#define BOT_COMMANDS_TASKS ""
#define BOT_COMMANDS_UNLOCK "/unlock - Unlock key safe\n/play4unlock - Throw dice to determine if unlocking should be possible\n"
#define BOT_COMMANDS_ROLES "/holder - Adopt holder role\n/teaser - Adopt teaser role\n/guest - Adopt guest role\n"
#define BOT_COMMANDS_WAITING "/waiting - Make wearer waiting to be captured by the holder\n/free - Make wearer free again (stops waiting for capture)\n"
#define BOT_COMMANDS_CAPTURE "/capture - Capture wearer as a sub\n/release - Release wearer as a sub\n"
#define BOT_COMMANDS_EMERGENCY "/thisisanemergency - Release the wearer in case of an emergency\n"

String botCommandsAll =     BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_RANDOM BOT_COMMANDS_TEASING                    BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES BOT_COMMANDS_WAITING BOT_COMMANDS_CAPTURE ;
String botCommandsWearer =  BOT_COMMANDS_GENERAL                                                                                 BOT_COMMANDS_UNLOCK                    BOT_COMMANDS_WAITING                      ;
String botCommandsHolder =  BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_RANDOM BOT_COMMANDS_TEASING BOT_COMMANDS_TASKS BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsGuest =   BOT_COMMANDS_GENERAL                                                                                                     BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsTeaser =  BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS                                                                                 BOT_COMMANDS_ROLES                      BOT_COMMANDS_CAPTURE ;
String botCommandsAllHelp = BOT_COMMANDS_GENERAL BOT_COMMANDS_SHOCKS BOT_COMMANDS_RANDOM BOT_COMMANDS_TEASING                    BOT_COMMANDS_UNLOCK BOT_COMMANDS_ROLES BOT_COMMANDS_WAITING BOT_COMMANDS_CAPTURE BOT_COMMANDS_EMERGENCY ;


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


#define COMMON_MSG_WAITING " The holder can capture him with the /capture command."
#define COMMON_MSG_CAPTURE " He is now securely locked and lost his permissions to open the key safe. The key safe can only be operated by the holder using the /unlock command.\nFurthermore, the wearer can now be punished with shocks."
#define COMMON_MSG_TEASING_ON "Teasing mode is activated. Users with teaser role can now support the holder with treatments like /shock_5."
#define COMMON_MSG_TEASING_OFF "Teasing mode is switched off. Only the holder has the permission to treat the wearer with shocks."


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

  index = users.AddUser(USER_ID_BOT, "ShockCell", ROLE_SHOCKCELL);
  users.GetUser(index)->SetBot(true);

  AdoptSettings();
}


// ------------------------------------------------------------------------
void Message::MessageWearerState(String chatId)
{
  User *u = users.GetWearer();

  if (u)
  {
    if (u->GetRoleId() == ROLE_WEARER_CAPTURED)
      SendMessage(SYMBOL_LOCK_CLOSED " Wearer is securely captured and has no rights to free himself.", chatId);
    else if (u->GetRoleId() == ROLE_WEARER_WAITING)
      SendMessage(SYMBOL_FINGER_UP " Wearer is available to be captured by a holder with command: /holder!!!", chatId);
    else
      SendMessage("Wearer is free.", chatId);
  }
}


// ------------------------------------------------------------------------
void Message::MessageLastShock(String chatId)
{
  SendMessage("Last shock was " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastShock()) + " ago.", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageIsClosed(String chatId)
{
  SendMessage(SYMBOL_LOCK_CLOSED " Key safe is safely closed and locked since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastClosing()) + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageIsOpen(String chatId)
{
  SendMessage(SYMBOL_LOCK_OPEN " Key safe is open since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfLastOpening()) + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageCoverStateChange(String chatId)
{
  if (coverState == COVER_CLOSED)
  {
    session.SetTimeOfLastClosing(timeFunc.GetTimeInSeconds());
    WriteCommandsAndSettings();
    SendMessage(SYMBOL_LOCK_CLOSED " Key safe has just been safely closed.", chatId);
  }
  else
  {
    session.SetTimeOfLastOpening(timeFunc.GetTimeInSeconds());
    WriteCommandsAndSettings();
    SendMessage(SYMBOL_LOCK_OPEN " Key safe has just been opened.", chatId);
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
void Message::MessageSendEarnedCredits(int creditsEarned, String chatId)
{
  SendMessage(String(SYMBOL_CREDIT) + " Wearer " + users.GetWearer()->GetName() + " has earned " + creditsEarned + " credits. His new credit balance is " + session.GetCredits() + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageSendEarnedVouchers(int vouchersEarned, String chatId)
{
  SendMessage(String(SYMBOL_VOUCHER) + " Wearer " + users.GetWearer()->GetName() + " has earned " + vouchersEarned + " unlock vouchers. His new voucher balance is " + session.GetVouchers() + ".", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageModes(String chatId)
{
  String msg;
  if (session.IsTeasingMode())
    msg = COMMON_MSG_TEASING_ON;
  else
    msg = COMMON_MSG_TEASING_OFF;
  SendMessage(msg, chatId);

  if (session.IsRandomMode())
    msg = SYMBOL_DEVIL_SMILE " Random mode is activated. The wearer receives " + String(session.GetRandomModeShocksPerHour(), DEC) + 
          " random shocks per hour since " + timeFunc.Time2String(timeFunc.GetTimeInSeconds() - session.GetTimeOfRandomModeStart()) + 
          ". The random mode will be automatically deactivated after " + timeFunc.Time2String(RANDOM_SHOCK_AUTO_OFF_SECONDS - timeFunc.GetTimeInSeconds() + session.GetTimeOfRandomModeStart()) + ".";
  else
    msg = "Random mode is switched off.";
  SendMessage(msg, chatId);

  if (verification.IsEnabled())
  {
    msg = SYMBOL_WATCHING_EYES " Verification mode is activated. The wearer must send " + String(verification.GetRequiredCountPerDay(), DEC) + 
          " verification photos per day as requested. He has provided " + String(verification.GetActualToday(), DEC) + " photos today.";
  }
  else
    msg = "Verification mode is switched off.";
  SendMessage(msg, chatId);

  SendMessage(SYMBOL_CREDIT " Wearer has collected " + String(session.GetCredits(), DEC) + " credits from shock treatments.", chatId);
  if (session.GetVouchers() > 0)
    SendMessage(SYMBOL_VOUCHER " Wearer has collected " + String(session.GetVouchers(), DEC) + " unlock vouchers.", chatId);
  if (session.GetDeviations() > 0)
    SendMessage(SYMBOL_SMILE_OHOH " Wearer has missed expectations " + String(session.GetDeviations(), DEC) + " times.", chatId);
  if (session.GetFailures() > 0)
    SendMessage(SYMBOL_FORBIDDEN " Wearer has failed " + String(session.GetFailures(), DEC) + " times to complete his tasks.", chatId);

  if (users.GetWearer()->IsSleeping())
    SendMessage(SYMBOL_SLEEPING " Wearer is currently allowed to sleep and shocks are disabled during that time.", chatId);
}


// ------------------------------------------------------------------------
void Message::MessageUsers(String chatId)
{
  int i = 0;
  String msg = "Users in chat:\n";
  while ((i < users.GetCount()) &&
         (i < USER_CACHE_SIZE))
  {
    String name = users.GetUser(i)->GetName();
    Serial.println(String("\"") + name + String("\""));
    if (name.length() > 0)
      msg += "- " + name + " (" + users.GetUser(i)->GetRoleStr() + ")\n";
    ++i;
    Serial.println(msg);
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
    msg = "Wearer " + users.GetWearer()->GetName() + " is locked in a ChastiKey session and controlled by holder " + session.GetChastikeyHolder() + ". The key safe remains securely locked as long as this session is active.";
//  else
//    msg = "Wearer " + users.GetWearer()->GetName() + " is not locked in a ChastiKey session.";
  SendMessage(msg, chatId);
}


// ------------------------------------------------------------------------
void Message::MessageState(String chatId)
{
  MessageWearerState(chatId);
  MessageLastShock(chatId);
  MessageCoverState(chatId);
  MessageModes(chatId);
  MessageUsers(chatId);
  MessageChastikeyState(chatId);
}


// ------------------------------------------------------------------------
void Message::MessageTasks(String chatId)
{
  String msg = tasklist.GetTaskList();
  SendMessage(msg, chatId);
}


// ------------------------------------------------------------------------
void Message::ShockAction(String durationStr, int count, String fromId, String chatId)
{
  if (timeFunc.IsSleepingTime())
  {
    SendMessage("Wearer " + users.GetWearer()->GetName() + " is allowed to sleep and cannot be shocked now.", chatId);
  }
  else
  {
    unsigned long milliseconds;
    if (durationStr.length() > 0)
      milliseconds = durationStr.toInt() * 1000L;
    else
      milliseconds = 3000;

    ShockAction(milliseconds, count, fromId, chatId);
  }
}


//------------------------------------------------------------------------
void Message::ShockAction(unsigned long milliseconds, int count, String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u)
  {
    if (u->IsHolder() ||
        u->IsBot() ||
        (u->IsTeaser() && session.IsTeasingMode()) ||
        (u->IsWearer() && session.IsTeasingMode()))
    {
      String msg = SYMBOL_COLLISION SYMBOL_COLLISION SYMBOL_COLLISION " Shock processing for " + String((milliseconds / 1000L), DEC) + " s begins...";
      SendMessage(msg, chatId);
      SendMessage(msg, USER_ID_WEARER);
      session.SetTimeOfLastShock(timeFunc.GetTimeInSeconds());
      session.SetCreditFractions(session.GetCreditFractions() + (milliseconds / 1000), chatId);
      WriteCommandsAndSettings();

      session.Shock(count, milliseconds);

      msg = "Shock processing completed. :-)";
      SendMessage(msg, chatId);
      SendMessage(msg, USER_ID_BOT);
      WriteCommandsAndSettings();
    }
    else
    {
      String msg = SYMBOL_NO_ENTRY " Request denied. Only the holder";
      if (session.IsTeasingMode())
        msg += " and the teasers";
      msg += " are allowed to execute shock punishments.";
      SendMessage(msg, chatId);
    }
  }
}


// ------------------------------------------------------------------------
void Message::WaitingAction(String fromId, String chatId)
{
  String msg;
  User *u = users.GetUserFromId(fromId);

  if (! u)
    return;

  // does request comes from free wearer?
  if (u->IsFreeWearer())
  {
    users.GetWearer()->UpdateRoleId(ROLE_WEARER_WAITING);
    msg = SYMBOL_WATCHING_EYES " Wearer " + users.GetWearerName() + " is now exposed and waiting to be captured by the holder." COMMON_MSG_WAITING;
    SendMessage(msg, chatId);
    WriteCommandsAndSettings();
  }
  // does request comes from waiting wearer?
  else if (u->IsWaitingWearer())
  {
    msg = "Wearer " + users.GetWearerName() + " is already waiting for capture by the holder." COMMON_MSG_WAITING;
    SendMessage(msg, chatId);
  }
  // does request comes from captured wearer?
  else if (u->IsCapturedWearer())
  {
    msg = "Wearer " + users.GetWearerName() + " is already captured by the holder.";
    SendMessage(msg, chatId);
  }
  else
  {
    msg = SYMBOL_NO_ENTRY " Only the free wearer can use the command /waiting to make himself available for capturing by a holder.";
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
    users.GetWearer()->UpdateRoleId(ROLE_WEARER_FREE);
    msg = "Wearer " + users.GetWearerName() + " is now free and cannot be captured by the holder.";
    SendMessage(msg, chatId);
    WriteCommandsAndSettings();
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
    msg = SYMBOL_NO_ENTRY " Request is denied! Wearer " + users.GetWearerName() + " is captured by the holder. He must be released by the holder to get free.";
    SendMessage(msg, chatId);
  }
  else
  {
    msg = SYMBOL_NO_ENTRY " Only the waiting wearer can use the command /free to be safe from capturing by the holder.";
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
      users.GetWearer()->UpdateRoleId(ROLE_WEARER_CAPTURED);
      msg = SYMBOL_LOCK_CLOSED " Capturing wearer " + users.GetWearerName() + " now." COMMON_MSG_CAPTURE;
      // Make requestor a holder if necessary
      if (! users.GetUserFromId(fromId)->IsHolder())
      {
        msg += " User " + users.GetUserFromId(fromId)->GetName() + " is now holder.";
        users.GetUserFromId(fromId)->SetHolder();
      }
      WriteCommandsAndSettings();
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
      SendMessage(SYMBOL_NO_ENTRY " You currently have no permission to capture the wearer. Guests may capture the wearer only, if there is no current holder.", chatId);
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
      users.GetWearer()->UpdateRoleId(ROLE_WEARER_WAITING);
      msg = SYMBOL_LOCK_OPEN " Releasing wearer " + users.GetWearerName() + ". He is now released and received back his permissions to open the key safe.\n";
      WriteCommandsAndSettings();
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
      SendMessage(SYMBOL_NO_ENTRY " The wearer cannot free himself. Only the holder may release the wearer.", chatId);
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
  session.InfoChastikey();

  if (session.IsActiveChastikeySession() && ! force)
  {
    SendMessage(SYMBOL_NO_ENTRY " Unlock request is denied! Wearer is locked in an active ChastiKey session.", chatId);
  }
  else if (users.GetUserFromId(fromId)->MayUnlock() || force)
  {
    if (force)
      SendMessageAll("An emergency release request has been granted.", chatId);
    SendMessageAll(SYMBOL_LOCK_OPEN " Key safe is unlocking now for 4 seconds and may be opened during this period.", chatId);
    session.Unlock();
    // check state of the safe afterwards
    coverState = digitalRead(COVER_OPEN_PIN);
    if (coverState == COVER_OPEN)
    {
      SendMessageAll(SYMBOL_KEY " Key safe has been opened.", chatId);
      WriteCommandsAndSettings();
    }
    else
      SendMessageAll(SYMBOL_LOCK_CLOSED " Key safe is locked again and has not been opened.", chatId);
  }
  else
  {
    SendMessage(SYMBOL_NO_ENTRY " You have no permission to unlock. Only the free wearer or the holder may unlock the key safe.", chatId);
  }
}


// ------------------------------------------------------------------------
void Message::PlayAction(String max, String fromId, String chatId)
{
  int maxCount = max.toInt();
  if (maxCount > 100)
    maxCount = 100;
  else if (maxCount < 2)
    maxCount = 2;
  String num[10] = { SYMBOL_DIGIT0, SYMBOL_DIGIT1, SYMBOL_DIGIT2, SYMBOL_DIGIT3, SYMBOL_DIGIT4, SYMBOL_DIGIT5, SYMBOL_DIGIT6, SYMBOL_DIGIT7, SYMBOL_DIGIT8, SYMBOL_DIGIT9 };
  Serial.println("*** Message::PlayAction()");
  SendMessage(String(SYMBOL_DICE) + " Playing with dice (1.." + String(maxCount, DEC) + ").", chatId);
  SendMessage(SYMBOL_DIGIT0 SYMBOL_DIGIT0 " " SYMBOL_DRUM, chatId);
  delay(300);
  int lastMessageId = GetLastSentMessageId();
  int result = (random(90000) * maxCount / 90000) + 1;
  Serial.println("- value: " + String(result, DEC));
  int i = 0;
  String progress = SYMBOL_DRUM;
  while (i <= result)
  {
    progress += ".";
    EditMessage(num[i / 10] + num[i % 10] + " " + progress, String(lastMessageId, DEC), chatId);
    if (maxCount < 10)
      i++;
    else
      i = i + maxCount/10;
  }
  EditMessage(num[result / 10] + num[result % 10] + " " + SYMBOL_STAR_GLOWING, String(lastMessageId, DEC), chatId);
}


// ------------------------------------------------------------------------
void Message::Play4UnlockAction(String fromId, String chatId)
{
  String num[10] = { SYMBOL_DIGIT0, SYMBOL_DIGIT1, SYMBOL_DIGIT2, SYMBOL_DIGIT3, SYMBOL_DIGIT4, SYMBOL_DIGIT5, SYMBOL_DIGIT6, SYMBOL_DIGIT7, SYMBOL_DIGIT8, SYMBOL_DIGIT9 };
  Serial.println("*** Message::Play4UnlockAction()");
  unsigned long duration = timeFunc.GetTimeInSeconds() - session.GetTimeOfLastClosing();
  int winPoints = session.GetCredits() + timeFunc.GetNumberOfDays(duration);
  SendMessage(String(SYMBOL_DICE) + " Playing for unlock. Wearer wins for values smaller than: " + num[winPoints / 10] + num[winPoints % 10] +
              " (" + timeFunc.GetNumberOfDays(duration) + " days locked + " + session.GetCredits() + " credits)", chatId);
  SendMessage(SYMBOL_DIGIT0 SYMBOL_DIGIT0 " " SYMBOL_DRUM, chatId);
  delay(300);
  int lastMessageId = GetLastSentMessageId();
  int result = (random(90000) / 900);
  Serial.println("- value: " + String(result, DEC));
  int i = 0;
  String progress = SYMBOL_DRUM;
  while (i < result)
  {
    progress += ".";
    EditMessage(num[i / 10] + num[i % 10] + " " + progress, String(lastMessageId, DEC), chatId);
    i = i + 5;
  }
  EditMessage(num[result / 10] + num[result % 10] + " " + SYMBOL_STAR_GLOWING, String(lastMessageId, DEC), chatId);
  if (result <= winPoints)
    SendMessage(SYMBOL_LOCK_OPEN " Wearer has won! " SYMBOL_SMILY_SMILE SYMBOL_SMILY_SMILE SYMBOL_SMILY_SMILE, chatId);
  else
    SendMessage(SYMBOL_LOCK_CLOSED " Wearer has lost! " SYMBOL_DEVIL_SMILE SYMBOL_DEVIL_SMILE SYMBOL_DEVIL_SMILE, chatId);
}


// ------------------------------------------------------------------------
void Message::HolderAction(String fromId, String chatId)
{
  User *u = users.GetUserFromId(fromId);
  String msg;

  if (u && u->MayBecomeHolder())
  {
    u->UpdateRoleId(ROLE_HOLDER);
    msg = SYMBOL_QUEEN " User " + users.GetUserFromId(fromId)->GetName() + " has now holder permissions.";
    msg += "The holder can use the following commands:\n" + botCommandsHolder;
    SendMessage(msg, chatId);
    WriteCommandsAndSettings();
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
      users.GetWearer()->UpdateRoleId(ROLE_WEARER_WAITING);
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer captured. He is now released and received back his permissions to open the key safe.";
    }
    u->UpdateRoleId(ROLE_TEASER);
    SendMessage(msg, chatId);
    WriteCommandsAndSettings();
  }
  else
  {
    msg = SYMBOL_NO_ENTRY " User " + users.GetUserFromId(fromId)->GetName() + " has no permission to become teaser.";
    SendMessage(msg, chatId);
  }
}


// ------------------------------------------------------------------------
void Message::TeasingModeAction(bool mode, String fromId, String chatId)
{
  Serial.println("*** Message::TeasingModeAction()");
  if (users.GetHolder() && (users.GetHolder()->GetId() == fromId))
  {
    session.SetTeasingMode(mode);
    if (mode)
      SendMessage(COMMON_MSG_TEASING_ON, chatId);
    else
      SendMessage(COMMON_MSG_TEASING_OFF, chatId);
    WriteCommandsAndSettings();
  }
  else
  {
    SendMessage(SYMBOL_NO_ENTRY " Request denied. Only the holder has the permission to control the rights for users with the teaser role.", chatId);
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
      users.GetWearer()->UpdateRoleId(ROLE_WEARER_WAITING);
      msg += " Since user " + users.GetUserFromId(fromId)->GetName() + " has given up the holder role, the wearer " + users.GetWearer()->GetName() + " is no longer captured. He is now released and received back his permissions to open the key safe.";
    }
    u->UpdateRoleId(ROLE_GUEST);
    SendMessage(msg, chatId);
    WriteCommandsAndSettings();
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
void Message::VerificationModeAction(String commandParameter, String fromId, String chatId, bool force)
{
  User *u = users.GetUserFromId(fromId);
  String msg;
  bool wearerMayUseCommand = false;
  int minimumVerificationCount = 1;
  if (verification.IsEnabled())
    minimumVerificationCount = verification.GetRequiredCountPerDay() + 1;
  int verificationsPerDay = commandParameter.toInt();
  if (u->IsWearer() && (verificationsPerDay >= minimumVerificationCount))
    wearerMayUseCommand = true;
    
  if (u)
  {
    if (force ||
        (u->IsHolder()) ||
        (u->IsTeaser() && session.IsTeasingMode()) ||
        wearerMayUseCommand)
    {
      if (commandParameter == "off")
      {
        verification.SetVerificationMode(false, 0);
        SendMessage("Switching off verification mode.", chatId);
      }
      else
      {
        if (verificationsPerDay > MAX_VERIFICATIONS)
        {
          msg += "Maximum setting of required verifications reached.";
          verificationsPerDay = MAX_VERIFICATIONS;
        }
        msg += "Switching on verification mode with " + commandParameter + " photos per day. This regulation remains active until it is switched off by a privileged user.";
        verification.SetVerificationMode(true, verificationsPerDay);
        SendMessage(msg, chatId);
      }
      WriteCommandsAndSettings();
    }
    else
    {
      if (u->IsWearer() && ! wearerMayUseCommand)
        SendMessage("User " + u->GetName() + " has no rights to switch off the verification mode or set levels below " + verificationsPerDay + " verifications per day. Only the holder and teasers are allowed to do this.", chatId);
      else
        SendMessage("User " + u->GetName() + " has no rights to switch on the verification mode. Only the holder, teasers and the wearer are allowed to do this.", chatId);
    }
  }
}


// ------------------------------------------------------------------------
void Message::RandomShockModeAction(String commandParameter, String fromId, String chatId, bool force)
{
  User *u = users.GetUserFromId(fromId);
  String msg;
  bool wearerMayUseCommand = false;
  int wearerMinimumShockRate = 5;
  if (session.IsRandomMode())
    wearerMinimumShockRate = session.GetRandomModeShocksPerHour() + 1;
  int shocksPerHour = commandParameter.toInt();
  if (u->IsWearer() && (shocksPerHour >= wearerMinimumShockRate))
    wearerMayUseCommand = true;

  if (u)
  {
    if (force ||
        (u->IsHolder()) ||
        (u->IsTeaser() && session.IsTeasingMode()) ||
        wearerMayUseCommand)
    {
      if (commandParameter == "off")
      {
        int creditsEarned = session.SetRandomMode(false);
        SendMessage("Switching off random mode.", chatId);
        MessageSendEarnedCredits(creditsEarned, chatId);
        WriteCommandsAndSettings();
      }
      else
      {
        if (shocksPerHour > 0)
        {
          if (shocksPerHour > RANDOM_SHOCKS_PER_HOUR_MAX)
          {
            msg += "Maximum setting of random shocks per hour reached.";
            shocksPerHour = RANDOM_SHOCKS_PER_HOUR_MAX;
          }
          msg += SYMBOL_COLLISION " Switching on random punishment mode with " + commandParameter + " shocks per hour on average. "
                 "This punishment remains active until it is switched off by a privileged user, " + String(RANDOM_SHOCK_AUTO_OFF_SECONDS/3600, DEC) +
                 " hours have passed or sleeping time of the wearer " + users.GetWearer()->GetName() + " begins.";
          session.SetRandomMode(true, shocksPerHour);
          SendMessage(msg, chatId);
          WriteCommandsAndSettings();
        }
        else
        {
          SendMessage("Error: invalid number of shocks per hour provided: " + commandParameter, chatId);
        }
      }
    }
    else
    {
      if (u->IsWearer() && ! wearerMayUseCommand)
        SendMessage("User " + u->GetName() + " has no rights to switch off the random mode or set levels below " + wearerMinimumShockRate + " shocks per hour. Only the holder and teasers are allowed to do this.", chatId);
      else
        SendMessage("User " + u->GetName() + " has no rights to switch on the random mode. Only the holder, teasers and the wearer are allowed to do this.", chatId);
    }
  }
}


// ------------------------------------------------------------------------
void Message::CheckVerificationAction(String caption, String fromId, String chatId)
{
  if ((fromId == users.GetWearer()->GetId()) &&
      ((caption.indexOf("Verification") != -1) ||
       (caption.indexOf("verification") != -1) ||
       (caption.indexOf("verify") != -1) ||
       (caption.indexOf("proof") != -1)))
  {
    verification.CheckIn(chatId);
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
void Message::RestartRequest(String fromId, String chatId)
{
  requestRestart = true;
  requestChatId = chatId;
}


// -------------------------------------------------
void Message::RestartAction(String fromId, String chatId)
{
  Serial.println("******************************************************************************************************");
  Serial.println("*** RestartAction() ");
  SendMessage("Restarting key safe...", chatId);
  delay(3000);
  ESP.restart();
}


// -------------------------------------------------
void Message::UnknownCommand(String chatId)
{
  bot.sendMessage(chatId, "Error: unknown command.");
}


// -------------------------------------------------
void Message::ResetRequests()
{
  requestRestart = false;
  requestChatId = "";
}


// -------------------------------------------------
void Message::ProcessPendingRequests()
{
  if (requestRestart)
    RestartAction(USER_ID_BOT, requestChatId);
  ResetRequests();
}


// -------------------------------------------------
void Message::ProcessChatMessage(String msg, String fromId, String chatId)
{
  // This method is meant to look into the general communication beyond the direct commands for the bot.
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
  while (newMessageCount)
  {
    Serial.println("Message received.");
    for (int i = 0; i < newMessageCount; i++)
    {
      // clear previous requests
      ResetRequests();

      // Chat id of the requester
      String chat_id = String(bot.messages[i].chat_id);
      Serial.print("Chat ID: ");
      Serial.println(chat_id);

      // check for photo
      bool hasPhoto = bot.messages[i].hasPhoto;
      String caption = bot.messages[i].file_caption;
      Serial.print("- hasPhoto: ");
      Serial.println(hasPhoto);
      Serial.print("- caption: ");
      Serial.println(caption);

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
      else if (text.substring(0, 6) == "/shock")
      {
        if (text[6] == '_')
          ShockAction(text.substring(7), 1, from_id, chat_id);
        else
          ShockAction(text.substring(6), 1, from_id, chat_id);
      }
      else if (text == "/state")
      {
        session.InfoChastikey();
        MessageState(chat_id);
      }
      else if (text == "/unlock")
        UnlockAction(from_id, chat_id);
      else if (text == "/play4unlock")
        Play4UnlockAction(from_id, chat_id);
      else if (text.substring(0, 5) == "/play")
        PlayAction(text.substring(6), from_id, chat_id);
      else if (text == "/users")
        MessageUsers(chat_id);
      else if (text == "/roles")
        MessageRoles(chat_id);
      else if (text == "/holder")
      {
        HolderAction(from_id, chat_id);
        CaptureAction(from_id, chat_id);
      }
      else if (text == "/teaser")
      {
        if (users.IdIsHolder(from_id))
          ReleaseAction(from_id, chat_id);
        TeaserAction(from_id, chat_id);
      }
      else if (text == "/teasing_on")
        TeasingModeAction(true, from_id, chat_id);
      else if (text == "/teasing_off")
        TeasingModeAction(false, from_id, chat_id);
      else if (text == "/guest")
      {
        if (users.IdIsHolder(from_id))
          ReleaseAction(from_id, chat_id);
        GuestAction(from_id, chat_id);
      }
      else if (text == "/waiting")
        WaitingAction(from_id, chat_id);
      else if (text == "/capture")
        CaptureAction(from_id, chat_id);
      else if (text == "/release")
        ReleaseAction(from_id, chat_id);
      else if (text == "/free")
        FreeAction(from_id, chat_id);
      else if (text == "/tasklist")
        MessageTasks();
      else if (text.substring(0, 7) == "/verify")
        VerificationModeAction(text.substring(8), from_id, chat_id);
      else if (text.substring(0, 13) == "/verification")
        VerificationModeAction(text.substring(14), from_id, chat_id);
      else if (text.substring(0, 7) == "/random")
        RandomShockModeAction(text.substring(8), from_id, chat_id);
      //      else if (text == "/restrict")
      //        RestrictUserAction(from_id, chat_id);
      else if (text == "/thisisanemergency")
        EmergencyAction(from_id, chat_id);
      else if (text == "/readsettings")
        AdoptSettings();
      else if (text == "/writesettings")
        WriteCommandsAndSettings();
      else if (text == "/restartshockcell")
        RestartRequest(from_id, chat_id);
      else if (text.substring(0, 1) == "/")
        UnknownCommand(chat_id);
      else if (hasPhoto)
        CheckVerificationAction(caption, from_id, chat_id);
      else
        ProcessChatMessage(text, from_id, chat_id);
    }
    newMessageCount = bot.getUpdates(bot.last_message_received + 1);
    ProcessPendingRequests();
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
void Message::EditMessage(String msg, String messageId, String chatId)
{
  Serial.print(chatId);
  Serial.print("> ");
  Serial.println(msg);
  bot.editMessage(chatId, messageId, msg);
}


// -------------------------------------------------
int Message::GetLastSentMessageId()
{
  return bot.getLastSentMessageId();
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
unsigned long Message::ReadParamLong(String text, String id)
{
  /*
    Serial.println("ReadParamLong()");
    Serial.print(text);
    Serial.println();
  */
  int paramPos = text.indexOf(id);
  text = text.substring(paramPos);
  Serial.print(text);
  Serial.println();
  int nextColon = text.indexOf(':');
  Serial.print(nextColon);
  Serial.println();
  int nextSemi = text.indexOf(';');
  Serial.print(nextSemi);
  Serial.println();
  String value = text.substring(nextColon + 1, nextSemi);
  Serial.println("=============");
  Serial.println(value);
  Serial.println(atol(value.c_str()));
  Serial.println("=============");
  return atol(value.c_str());
}


// -------------------------------------------------
void Message::AdoptUserInfos(String text)
{
  Serial.println("*** AdoptUserInfos()");
  Serial.print(text);
  Serial.println();
  int userPos = text.indexOf(USERS_PREFIX) + String(USERS_PREFIX).length();
  int chatPos = text.indexOf(CHATS_PREFIX);
  text = text.substring(0, chatPos);
  Serial.println(String("- Input string: ") + text);
  Serial.println();
  int nextColon, nextSemi;
  int roleId;
  bool isBot;
  String name, id, role;
  while (userPos >= 0)
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
void Message::AdoptChatDescription()
{
  String descr;
  Serial.println("*** AdoptChatDescription()");
  
  if (bot.getChatDescription(GROUP_CHAT_ID, descr))
  {
    String response = bot.getMyCommands();
    DynamicJsonDocument doc(TELEGRAM_MAX_MESSAGE_LENGTH);
    DeserializationError error = deserializeJson(doc, (char *) response.c_str());

    if (!error)
    {
      if (doc.containsKey("result"))
      {
        int resultArrayLength = doc["result"].size();
        if (resultArrayLength > 0)
        {
          descr = "";
          int newMessageIndex = 0;
          // Step through all results
          for (int i = 0; i < resultArrayLength; i++)
          {
            JsonObject result = doc["result"][i];
            String command = result["command"];
            Serial.print("- command: ");
            Serial.println(command);
            String description = result["description"];
            Serial.print("- description: ");
            Serial.println(description);
            if (command.charAt(0) == '_')
              descr += " " + description;
          }
        }
        else
          Serial.println("- no new messages");
      }
      else
        Serial.println("- Response contained no 'result'");
    }
  }
}


// -------------------------------------------------
void Message::ParseSettings(String & descr)
{
  String response = bot.getMyCommands();
  DynamicJsonDocument doc(TELEGRAM_MAX_MESSAGE_LENGTH);
  DeserializationError error = deserializeJson(doc, (char *) response.c_str());

  if (!error)
  {
    if (doc.containsKey("result"))
    {
      int resultArrayLength = doc["result"].size();
      if (resultArrayLength > 0)
      {
        descr = "";
        int newMessageIndex = 0;
        // Step through all results
        for (int i = 0; i < resultArrayLength; i++)
        {
          JsonObject result = doc["result"][i];
          String command = result["command"];
          Serial.print("- command: ");
          Serial.println(command);
          String description = result["description"];
          Serial.print("- description: ");
          Serial.println(description);
          if (command.charAt(0) == '_')
            descr += " " + description;
        }
      }
      else
        Serial.println("- no new messages");
    }
    else
      Serial.println("- Response contained no 'result'");
  }

  Serial.print("- descr: ");
  Serial.println(descr);
}


// -------------------------------------------------
void Message::AdoptSettings()
{
  String descr;
  Serial.println("*** AdoptSettings()");

  if (bot.getChatDescription(GROUP_CHAT_ID, descr))
  {
    AdoptChatDescription();
    ParseSettings(descr);
    // LastOpening:1598445166; LastClosing:1598445167; LastShock:1598445168; USERS: Charly:1157999292:Wearer; ShockCell:1264046045:ShockCell; S:919525040:Guest;
    session.SetTimeOfLastOpening(ReadParamLong(descr, LAST_OPENING_TAG));
    session.SetTimeOfLastClosing(ReadParamLong(descr, LAST_CLOSING_TAG));
    session.SetTimeOfLastShock(ReadParamLong(descr, LAST_SHOCK_TAG));
    session.SetTimeOfRandomModeStart(ReadParamLong(descr, RANDOM_MODE_START_TAG));
    verification.Reset();
    verification.SetVerificationModeInt(ReadParamLong(descr, VERIFICATION_MODE_TAG));
    verification.SetActualToday((int) ReadParamLong(descr, ACTUAL_VERIFICATIONS_TAG));
    verification.SetDayOfWeek((int) ReadParamLong(descr, DAY_OF_WEEK_TAG));
    for (int i = 0; i < verification.GetRequiredCountPerDay(); i++)
    {
      verification.GetEvent(i)->SetTimeOfBegin(ReadParamLong(descr, NEXT_VERIFICATION_BEGIN_TAG + String(i, DEC)));
      verification.GetEvent(i)->SetTimeOfEnd(ReadParamLong(descr, NEXT_VERIFICATION_END_TAG + String(i, DEC)));
      Serial.println("- begin: " + timeFunc.Time2StringNoDays(verification.GetEvent(i)->GetTimeOfBegin()));
      Serial.println("- end: " + timeFunc.Time2StringNoDays(verification.GetEvent(i)->GetTimeOfEnd()));
      if (timeFunc.GetTimeInSeconds() > verification.GetEvent(i)->GetTimeOfBegin())
      {
        // we do not need to check, if we are past the end. This is handled in ProcessVerification()
        verification.SetCurrentIndex(i);
      }
      if ((i > 0) && (i < (verification.GetRequiredCountPerDay() - 1)))
        verification.GetEvent(i)->SetRandom(true);
    }
    verification.Print();

    session.SetTeasingModeInt(ReadParamLong(descr, TEASING_MODE_TAG));
    session.SetRandomModeInt(ReadParamLong(descr, RANDOM_MODE_TAG));
    Serial.print("- random Mode: ");
    Serial.println(session.IsRandomMode());
    Serial.print("- random Mode cycle: ");
    Serial.println(session.GetRandomModeShocksPerHour());
  //  session.SetRandomMode(false, 5);
    session.SetCredits(ReadParamLong(descr, CREDITS_TAG));
    session.SetVouchers(ReadParamLong(descr, VOUCHER_TAG));
    session.SetDeviations(ReadParamLong(descr, DEVIATIONS_TAG));
    session.SetFailures(ReadParamLong(descr, FAILURES_TAG));
    //  info += users.GetUsersInfo();
    AdoptUserInfos(descr);
  }
  else
  {
    // this is only needed for the initial setup of a chat group
    WriteCommandsAndSettings();
  }
}


/*
/start - Start communication\n
[{"command":"start","description":"Start communication"},{"command":"roles","description":"List roles"},{"command":"shock1","description":"Shock for 1 seconds"},{"command":"shock3","description":"Shock for 3 seconds"},{"command":"shock5","description":"Shock for 5 seconds"},{"command":"shock10","description":"Shock for 10 seconds"},{"command":"shock30","description":"Shock for 30 seconds"},{"command":"state","description":"Report the cover state"},{"command":"unlock","description":"Unlock key safe"},{"command":"users","description":"Lists users in chat"},{"command":"holder","description":"Adopt holder role"},{"command":"teaser","description":"Adopt teaser role"},{"command":"guest","description":"Adopt guest role"},{"command":"waiting","description":"Make wearer waiting to be captured by the holder"},{"command":"free","description":"Make wearer free again (stops waiting for capturing by the holder)"},{"command":"capture","description":"Capture wearer as a sub"},{"command":"release","description":"Release wearer as a sub"},{"command":"_lasttimes","description":"LastOpening:1604473444; LastClosing:1604473509; LastShock:1604853044; RandomModeStart:1604847654;"},{"command":"_modes","description":"TeasingMode:1; RandomMode:0; Credits:0;"},{"command":"_userlist","description":"Charly:1157999292:Wearer/captured; Ruler:1264046045:ShockCell; S:919525040:Guest; roberta:1209481168:Holder; Spark:780464021:Teaser"}]
*/
// -------------------------------------------------
void Message::WriteCommandsAndSettings()
{
  // This is the old way of storing the settings
  // Redundant saving...
  Serial.println("*** WriteCommandsAndSettings()");
  String info;
  info = verification.GetHash();
  info += " " USERS_PREFIX " ";
  info += users.GetUsersInfo();

  // we need to make sure that the description is changed with every request and not identical to the existing one.
  //  SetDescription(GROUP_CHAT_ID, "-");
  if (info != lastChatDescription)
  {
    SetChatDescription(GROUP_CHAT_ID, info);
    lastChatDescription = info;
  }

  // ----------------------------------------------
  // This is the new way of storing the settings
  String commands = "[";
  Serial.println(botCommandsAll);
  int pos = 0;
  // process all commands
  while ((pos = botCommandsAll.indexOf('/', pos)) != -1)
  {
    int posDash;
    int posNL;
    Serial.print(" - pos=");
    Serial.println(pos);
    if ((posDash = botCommandsAll.indexOf(" - ", pos)) != -1)
    {
      Serial.print(" - posDash=");
      Serial.println(posDash);
      if ((posNL = botCommandsAll.indexOf("\n", pos)) != -1)
      {
        commands += "{\"command\":\"" + botCommandsAll.substring(pos+1, posDash) + "\",\"description\":\"" + botCommandsAll.substring(posDash+3, posNL) + "\"},";
        Serial.print(" - posNL=");
        Serial.println(posNL);
        Serial.println(commands);
        Serial.println("-----------------------");
      }
    }
    pos++;
  }

  // write setting times
  commands += "{\"command\":\"_settingtimes\",\"description\":\""
              LAST_OPENING_TAG ":" + String(session.GetTimeOfLastOpening(), DEC) + "; "
              LAST_OPENING_TAG ":" + String(session.GetTimeOfLastOpening(), DEC) + "; "
              LAST_CLOSING_TAG ":" + String(session.GetTimeOfLastClosing(), DEC) + "; "
              LAST_SHOCK_TAG ":" + String(session.GetTimeOfLastShock(), DEC) + "; "
              RANDOM_MODE_START_TAG ":" + String(session.GetTimeOfRandomModeStart(), DEC) + "; "
              "\"},";
  commands += "{\"command\":\"_verification\",\"description\":\"";
  for (int i = 0; i < MAX_VERIFICATIONS; i++)
    commands += NEXT_VERIFICATION_BEGIN_TAG + String(i, DEC) + ":" + String(verification.GetEvent(i)->GetTimeOfBegin(), DEC) + "; " +
                NEXT_VERIFICATION_END_TAG + String(i, DEC) + ":" + String(verification.GetEvent(i)->GetTimeOfEnd(), DEC) + "; ";
  commands += "\"},";
  commands += "{\"command\":\"_settingmodes\",\"description\":\""
              TEASING_MODE_TAG ":" + String(session.GetTeasingModeInt(), DEC) + "; "
              RANDOM_MODE_TAG ":" + String(session.GetRandomModeInt(), DEC) + "; "
              VERIFICATION_MODE_TAG ":" + String(verification.GetVerificationModeInt(), DEC) + "; "
              ACTUAL_VERIFICATIONS_TAG ":" + String(verification.GetActualToday(), DEC) + "; "
              DAY_OF_WEEK_TAG ":" + String(verification.GetDayOfWeek(), DEC) + "; "
              CREDITS_TAG ":" + String(session.GetCredits(), DEC) + "; "
              VOUCHER_TAG ":" + String(session.GetVouchers(), DEC) + "; "
              DEVIATIONS_TAG ":" + String(session.GetDeviations(), DEC) + "; "
              FAILURES_TAG ":" + String(session.GetFailures(), DEC) + "; "
              "\"},";
  commands += "{\"command\":\"_users\",\"description\":\"" " " USERS_PREFIX " " +
              users.GetUsersInfo() +
              "\"},";
  commands += "{\"command\":\"_chats\",\"description\":\"" " " CHATS_PREFIX " " +
              chats.GetChatsInfo() +
              "\"},";

  // remove last comma
  commands.remove(commands.length() - 1);
  commands += "]";
  Serial.println(commands);
  bot.setMyCommandsStr("?commands=" + commands);
}


//
