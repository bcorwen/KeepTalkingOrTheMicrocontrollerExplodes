# Keep Talking Or the Microcontroller Explodes
KTOME - the ESP32-based follow-on from the Arduino Mega prototype

## Goal of the project
I'm aiming to copy the Keep Talking And Nobody Explodes videogame as closely as possible, within the bounds of my skill and what parts are available.

I started this project around February 2020, going down the route of one Arduino Mega controlling the bomb. I completed a prototype of 3 - 4 modules plus the timer, before deciding the change my approach given what I had learned from the prototype. The prototype can be viewed [here](https://www.youtube.com/watch?v=qZbycguCcf4)!

The current plan is to have each module run with its own ESP32 microcontroller, communicating with each other using the CAN protocol. The modules will be plug-and-play into the bomb chassis, allowing you to bomb your own challenge. The parts will be built up with electronics which can provide the same functionality and aesthetics as the game, allowing the original manual to be used, with use of a 3D printer which is a new investment for me. The game will be customisable through a companion phone app, which can set things such as hardcore mode or the game timer via a BLE connection.

## Current tasks
### Design:
- [X] : Complete design and print of widgets
- [X] : Complete design and print of blank modules
- [ ] : Complete design and print of lower case panels
- [X] : Design and print Memory
- [ ] : Design and print Morse Code
- [ ] : Design and print Venting Gas
- [ ] : Design and print Complicated Wires
### PCB:
- [X] : Draw up generic PCB schematic and gerber for modules
- [X] : Order test PCBs
- [X] : Test PCB (programming, CAN comms, serial connector)
- [ ] : Create specific module PCBs
- [ ] : Wire module back-panels
- [ ] : Wire widget docks

## Project to-do list
### Programming:
- [x] Basic game logic common to all modules
- [x] CAN communication between modules
- [x] BLE communication between Master ESP32 and phone app
- [x] Complete timer (Master) module
- [ ] Complete standard modules
- [ ] Complete needy modules
### Design:
- [x] Basic module designs (blank faceplate and box)
- [x] Case and edgework
- [ ] Find electronics which matches game as closely as possible
- [X] Power solution
- [x] Module connectors
- [ ] Complete standard modules
- [ ] Complete needy modules
### PCB:
- [X] Test an ESP32 custom PCB
- [ ] Build specific PCBs for modules
- [X] Build widget/infrastructure PCB
