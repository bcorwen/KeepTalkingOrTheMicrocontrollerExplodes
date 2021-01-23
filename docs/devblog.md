[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev Blog

---

## 2021/01/23 - Origins
It's been about one year since I started this project, and so it feels like I should get this blog going with a recap of the journey so far. For the first entry, I'll revisit the Arduino prototype.

### Arduino
As stated on the Goals page, I received an Arudino starter kit for Christmas 2019 from my family. Not wanting to simply go through the rather basic tutorials (although to give them credit they were interesting concepts, just slow on the progress), I remembered my love of the game Keep Talking and that. At its fundamental level, it was a game that came with a clear set of (codable) rules and was constructed from simple parts like lights, buttons and wires (and screens and motors but that's still not so crazy - the game was designed in a really grounded way). I figured I could learn how to use these pieces as I go building up the game, module by module. I had a little grounding in electronics, having had a similar kit years previous (before Arduinos were a thing) and previous coding experience, but not in C++; enough to believe I could make some facsimile of the videogame bomb.
The Arduino Mega was a reasonable microcontroller to run the game. It had enough memory to keep hold the rules of many modules and keep track of all of the variables required during gameplay. With about 50 I/O pins, it could connect up to a reasonable impressive amount of electronics too.

### Building and learning
The starter kit had a good variety of components: LCD screens, LED segment displays, single LEDs, buttons and switches. I bought in a few other parts, thinking ahead to the look and feel of the modules, and began piecing things together.
> ![Early photo of the prototype](https://i.imgur.com/an2r3ZR.jpg)
> An early photo of the prototype
The code was a mess and I was struggling with a few basics, but thanks to some good tutorials and libraries I managed to create a 4 module prototype on that Mega:
* Timer: with a nice big time display, two LED strike lights and a piezobuzzer for all sound effects (this was difficult to organise a priority system for more important sounds (strike buzzer) to play over less important ones (clock ticking)!)
* Wires: taking a guess at the generating algorithm, this would create a 3-6 wire sequence for the user to set-up before a game. The Mega would check inputs to see if the correct wires were (dis)connected to know it was properly set-up.
* Button: after learning how to do debouncing, a relatively simple module to create.
* Simon Says: the arcade buttons really completed the look, but this was a relatively tricky module which needed smart state tracking and timing for lighting up the buttons in sequence.
* Keypad: this was only half-implemented but playable, and made use of the custome character creation of the LCD screen used for setup to display the keypad symbols.
> ![Keypad setup screen](https://i.imgur.com/2fTtbNI.jpg)
> Using the custom characters of the LCD to setup the Keypad module

### Reaching the limit
I originally wanted to create as much as I possibly could of the game, emulating the 14 modules as close as I physically could, plug-and-play modules, and the ability to have 6 or 12 modules in play at once. This was beginning to look like a huge stretch for one Mega to handle.
I started building a wooden case for the modules which did the job but it was a struggle with hand tools and I had no ability to use a makerspace in the start of the Covid lockdown. Then I moved the electronics into the case with a forward-looking view to achieving more than those 4 modules.
> ![Button in the wooden case](https://i.imgur.com/IVS5SpJ.jpg)
> The Button module being tested in its wodden case
> ![The modules in their wooden cases](https://i.imgur.com/VMsLNAq.jpg)
> The finished modules

In order to try to get the most out of the Mega, I refined the code to slim it down as much as possible and tried to extend the limited number of I/O pins to be able to maximise the amount of supported modules at one time. I hadn't done much soldering at this point, but designed two boards to help increase the number of I/Os and make that plug-and-play aspect a little easier.
> ![One of the socket boards](https://i.imgur.com/orsPxDr.png)
> One of the socket plug-and-play boards

After discovering shift registers, I reimplemented the code to read in any input and push out and output through a register. Then devised a scheme where reigsters would all be chained to each other and divided into nibbles. So one shift in register could support the 4 buttons for Simon Says and 4 buttons from Keypad, for example. This way, the three I/Os needed to drive the chain of 3 registers could handle 24 input lines.
I also soldered a line of pins to act as an LCD bus, realising I could use many LCDs across the modules. Breaking out the chip select, I could keep the other lines common.

Despite all of this, it was obvious that the Mega would really struggle to get much further, and so I needed to make a choice on where to go next. I took the difficult decision to scrap the Mega prototype and go down a different path, one which could get me much closer to the original game.
In getting help breaking down the workings of the game and getting feedback on my build, I found Nick on the official KTANE discord who was also starting his real bomb build. As there were a few of us doing a similar project, he set up the [KTANE IRL Discord](https://discord.com/channels/711013430575890432) for us builders to talk shop. He pointed me further down the rabbit hole, suggesting other microcontrollers and methods to use. My skill in coding and understanding electronics was still quite basic but had developed a lot since starting, and I was ready to attempt to use more complicated hardware and attempt the previously unthinkable communications protocols between mulitple microcontrollers!
The ESP32 version took shape and work began again...
