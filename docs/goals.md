[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Goals

I'm aiming to copy the Keep Talking And Nobody Explodes (KTANE) videogame as closely as possible, within the bounds of my skill and what parts are available.

I started this project around February 2020. The Christams before, I received an Arduino starter kit which I had a brief play with before wondering what I could do this it. The tutorials were interesting but it was quite a slow introduction and I wanted to do something with a bit more meaning than just learning how to use different electronic components. That's when I remembered about KTANE, a game a loved, which is, when you break it down, just a series of buttons, wires and LEDs. I figured it would be complicated and I wouldn't be able to make a perfect replica but it would be a great learning experience and might be a little entertaining when built, rather than being pulled apart immediately or sitting in a drawer somewhere.

> ![Screenshot of Keep Talking](https://i.imgur.com/fZXojRZ.jpg)
> Screenshot of "Keep Talking and Nobody Explodes" videogame.

Originally, I was going down the route of one Arduino Mega controlling the bomb. By May 2020, I completed a prototype of 3 - 4 modules plus the timer, before deciding the change my approach given what I had learned from the prototype. The prototype can be viewed [here](https://www.youtube.com/watch?v=qZbycguCcf4)!

The issue with this approach was primarily memory of the Mega and the number of input/output pins available. This was pretty valuable in having me learn C++ and use it very efficiently, and forced me to look wider at electronics and experiment with more (I discovered shift registers and had to devise clever ways of using them to extend the effective pin count).

> ![Prototype photo](https://i.imgur.com/t7DDHaU.jpg)
> The prototype version freshly wired into wooden module cases.

The current plan is to have each module run with its own ESP32 microcontroller, communicating with each other using the CAN (aka TWAI) protocol. The modules will be plug-and-play into the bomb chassis, allowing you to build-your-own-bomb. The parts will be designed to have the same functionality and aesthetics as the game (where possible or appropriate!), allowing the original manual to be used. I've purchased a 3D printer which is a new investment for me and another skill to learn. The game will be customisable through a companion phone app, which can set things such as hardcore mode or the game timer via a Bluetooth Low Energy connection.

> ![Recent progress photo](https://i.imgur.com/PVWNPcI.jpg)
> Recent progress photo showing three ESP32s connected by CAN bus.
