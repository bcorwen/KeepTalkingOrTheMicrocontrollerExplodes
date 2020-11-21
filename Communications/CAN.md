# CAN communication

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
Master to send strike count | X1 or X2 etc...
Master to send serial vowel bool | V0 or V1
Master to send serial last digit even/odd bool | 20 or 21
Master to send battery number | B0 etc...
Master to send indicator status | F0C0 (FRK then CAR)
Master to send port status | LS0P0 (Serial then parallel)
Module to tell master a strike occurred | x
Module to tell master of a defusal | d
Module to request time from master | t
Master to transmit time | T0000 (as appears on the timer - not seconds but digits)

Manual setup message | CAN content
------------ | -------------
Wires | crwk yb (lower case letter or space)
Button | crd (colour then label)
Keypad | c479: (number for each symbol, starting from ASCII 0 for easy serial printing)
C Wires | c1234 (each byte uses 6 bits for each wire slot, referrencing whether the wire is white, red, blue, * - LED is not needed to be known by the user)
Wire S | c1  rAbC (two messages to be sent, last 6 bytes denote colour and destination for each numbered slot)