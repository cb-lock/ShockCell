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
