# Keep Talking Or the Microcontroller Explodes
KTOME - the ESP32-based follow-on from the Arduino Mega prototype

## Goal of the project
I'm aiming to copy the Keep Talking And Nobody Explodes videogame as closely as possible, within the bounds of my skill and what parts are available.

I started this project around February 2020, going down the route of one Arduino Mega controlling the bomb. I completed a prototype of 3 - 4 modules plus the timer, before deciding the change my approach given what I had learned from the prototype. The prototype can be viewed [here](https://www.youtube.com/watch?v=qZbycguCcf4)!

The current plan is to have each module run with its own ESP32 microcontroller, communicating with each other using the CAN protocol. The modules will be plug-and-play into the bomb chassis, allowing you to bomb your own challenge. The parts will be built up with electronics which can provide the same functionality and aesthetics as the game, allowing the original manual to be used, with use of a 3D printer which is a new investment for me. The game will be customisable through a companion phone app, which can set things such as hardcore mode or the game timer via a BLE connection.

## Current progress and to-do
### Programming
- [x] Basic game logic common to all modules
- [x] CAN communication between modules
- [ ] BLE communication between Master ESP32 and phone app
- [ ] Complete timer (Master) module
- [ ] Complete standard modules
- [ ] Complete needy modules
### Design
- [ ] Basic module designs (blank faceplate and box)
- [ ] Case and edgework
- [ ] Find electronics which matches game as closely as possible
- [ ] Power solution
- [ ] Module connectors
- [ ] Complete standard modules
- [ ] Complete needy modules