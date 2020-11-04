# The ShockCell Project

## Overview

This project is about creating an internet-controlled electronic key safe with additional control functions. The key safe uses Wi-Fi to connect to the internet and provides a Telegram messenger chat bot for interaction with remote users.

A major additional function of the key safe is to trigger shocks using a shock device for remote controlled punishments.

## Use Case

Chastity play between a remote holder and a chastity device wearer requires secure management of the keys allowing to unlock the chastity device. This key safe is meant to serve the purpose of allowing the holder to decide when keys are returned to the wearer.

I use a chastity assembly consisting of a special chastity cage with an integrated shock device and an electronically controlled, Wi-Fi enabled key safe for my chastity keys with an integrated remote control for the shock device. With this project, I want to give others the opportunity to do the same or exchange ideas and improve the concept.

## Description

The key safe consists of a flat box with a hinged cover that automatically locks once pressed shut. It incorporates a microcontroller board hosting the entire software of this project. The microcontroller board connects to several sensors and actors:
* Actor activating the keypad button of the remote control for the shock device,
* servo motor for pulling open the locking bracket of the cover ratchet mechanism,
* sensor of the state of the cover (open/closed) and
* display for information about the current state of the key safe.
The key safe encloses a compartment for storing the keys as well as all other components.

It is connected to the LAN as a Wi-Fi based client and powered over USB.

## Operation

To begin a session, you put all the chastity device keys into the key safe and close it. The ratchet mechanism keeps the lock box now closed until a valid unlock command is sent to the Telegram bot.

## Telegram bot

The ShockCell key safe runs a bot for the Telegram Messenger App. The bot is the only interface to the key safe. It can be added to a group chat to control the key safe. While the group chat can be used for general communication between the holder, wearer and other users. the following commands are available to control the key safe. Any command must be send as a separate message and should not contain any other text. The prefix for each command is a slash "/".

### Roles

The ShockCell key safe knows four different roles in the group chat.

1. Wearer - the person being controlled and wearing a chastity device with a lock and shock device.
2. Holder - the person controlling (holding) the key safe storing the keys to that lock. She/he also controls the shocks for the wearer.

### Start communication/overview

/start

The /start command lists all commands available for the role 

#define BOT_COMMANDS_GENERAL "/start - Start communication\n/state - Report the cover state\n/roles - List roles\n/users - List users in chat\n"
#define BOT_COMMANDS_SHOCKS "/shock1 - Shock for 1 seconds\n/shock3 - Shock for 3 seconds\n/shock5 - Shock for 5 seconds\n/shock10 - Shock for 10 seconds\n/shock30 - Shock for 30 seconds\n"
#define BOT_COMMANDS_RANDOM "/random_5 - Switch on random shock mode with 5 shocks per hour (other intervals work with corresponding numbers)\n/random_off - Switch off random shock mode\n"
#define BOT_COMMANDS_TEASING "/teasing_on - Enable teasing\n/teasing_off - Disable teasing\n"
#define BOT_COMMANDS_UNLOCK "/unlock - Unlock key safe\n"
#define BOT_COMMANDS_ROLES "/holder - Adopt holder role\n/teaser - Adopt teaser role\n/guest - Adopt guest role\n"
#define BOT_COMMANDS_WAITING "/waiting - Make wearer waiting to be captured by the holder\n/free - Make wearer free again (stops waiting for capture)\n"
#define BOT_COMMANDS_CAPTURE "/capture - Capture wearer as a sub\n/release - Release wearer as a sub"
#define BOT_COMMANDS_EMERGENCY "/thisisanemergency - Release the wearer in case of an emergency\n"

