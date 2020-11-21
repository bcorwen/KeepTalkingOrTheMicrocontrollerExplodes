# BLE communication

## BLE introduction
Bluetooth Low Energy (BLE) is a light-weight version of Bluetooth, utilised in many low-power devices such as fitness wearables. This differs from core Bluetooth communications as it limits communications to short, discrete messages rather than data streaming.
The master device (in this case the ESP32 timer) starts and advertises a service. Other devices (the companion phone app) can detect the advertisement and connect, becoming slaves.
Messages are limited to 19 bytes each, requiring careful thought to how to utilise the service.

## BLE messages
Message | BLE content
------------ | -------------
App moving to Game manager screen | I
App asking for a refresh of mods | I
ESP replying with list of connected mods | i 1 0 0... 
App enabling hardcore | h=1
App disabling hardcore | h=0
App setting game timer | t=5
App to start manual mod setup | C
App to confirm this mod is setup | >
App to abort manual setup | <
ESP to send setup data: wires | c w 1 2 3 4 5 6
ESP to send setup data: button | c b 1 2
ESP to send setup data: keypad | c k 1 2 3 4
ESP to send setup data: cwires | c p ...
ESP to send setup data: wiresq | c q ...
ESP to send setup data: all setup complete | c Y
ESP to reject confirmation of mod setup | c X
App to start game | A
App to stop game | Z

The companion app makes good use of space-separated lists due to the limitations of the programming environment.

When the ESP sends module list to the app, a space-separated list of the module count for each module is sent. Two messages are required in total to communicate all of the information, as one 19-byte message cannot send info for all 14 modules using a space-separated list.
ESP messages with module setup info have numbers which reference lists of colours, labels, symbols etc. So a message of "c b 1 2" would state the setup for the Button module, with 1 referring to the first colour in the colour array and 2 referring to the second label in the label array.