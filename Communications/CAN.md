# CAN communication

## CAN introduction
Controller Area Network (CAN) is a serial communications protocol, designed and used in vehicles for systems to transfer data. It is a multi-master system, with messages passed to all nodes on the network.
As ESP32 have an in-built CAN-controller, adding CAN transceiver chips allow for easy use of CAN communications between microcontrollers.
CAN is used in this project to enable module-to-module, ESP-to-ESP communications.
CAN nodes each have a CAN ID, an 11-bit (or 29-bit for extended frame) number. Messages can be tagged with an ID, which can allow messages to be directed at certain nodes and ignored by others. Filters can be employed with masks to allow for partial ID matches.
The extended frame is used in this project, with the first 15 bits used to specify the type of module, and the next 4 bits used to differentiate modules of the same type. Only one bit is used at a time (so a max of 4, not 2^4) due to limitations on how to address specific or all modules without changing the filter.
Messages are 29-bytes long, requiring some planning with the message formats.

## Module identification
CAN ID bit | Used to denote
------------ | -------------
29 | Timer (Master)
28 | Wires
27 | Button
26 | Keypad
25 | Simon
24 | Who's
23 | Memory
22 | Morse
21 | Complicated Wires
20 | Wire Sequence
19 | Maze
18 | Password
17 | Venting Gas
16 | Capacitor
15 | Knobs
14 | Unique ID #1
13 | Unique ID #2
12 | Unique ID #3
11 | Unique ID #4
10 | Unused
9 | Unused
8 | Unused
7 | Unused
6 | Unused
5 | Unused
4 | Unused
3 | Unused
2 | Unused
1 | Unused

## CAN messages

Message | CAN content
------------ | -------------
Master to poll if module is connected | P
Module to reply if connected | p
Master to have modules initialise a game | I
Module to reply when initialised a game | i
Master to ask module for manual setup info | C
Module to reply with manual setup | c...
Master to confirm manual setup aligns with inputs | M
Module (does not) confirm inputs match expected setup | m0 or m1
Master to send game start signal | A
Master to send game stop signal | Z
Master to send strike count | X1, X2 or X3
Master to send widget setup | S0000000 ...
Module to tell master a strike occurred | x
Module to tell master of a defusal | d
Module to request time from master | t
Master to transmit time | T0000 *

* Time sent as digits appear on the clock, not the time in seconds!

Manual setup message | CAN content
------------ | -------------
Wires | crwk yb (lower case letter or space)
Button | crd (colour then label)
Keypad | c479: (number for each symbol, starting from ASCII 0 for easy serial printing)
C Wires | c1234 (each byte uses 6 bits for each wire slot, referrencing whether the wire is white, red, blue, * - LED is not needed to be known by the user)
Wire S | c1  rAbC (two messages to be sent, last 6 bytes denote colour and destination for each numbered slot)

Widget setup message | CAN content
------------ | -------------
1st bit | S (denotes widget message)
2nd bit | 0 or 1 (Serial vowel boolean)
3rd bit | 0 or 1 (Serial last digit odd boolean)
4th bit | Battery number
5th bit | 0 or 1 (CAR indicator boolean)
6th bit | 0 or 1 (FRK indicator boolean)
7th bit | 0 or 1 (Serial port boolean)
8th bit | 0 or 1 (Parallel port boolean)