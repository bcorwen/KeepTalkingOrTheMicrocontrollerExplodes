[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev Blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

## 2021/01/24 - Moving to the ESP32 and the new bomb structure
This is another long overdue update covering about May to July 2020, after retiring the prototype to move from using the Arduino Mega to ESP32s. It's a short one but covers a lot of the considerations of moving microcontrollers.

### The ESP32
The initial move to the ESP32 was relatively straightforward; I could continue scripting through the Arduino IDE, there were only a few libraries that would need to be updated for the ESP32 board, and by-and-large the ESP32 behavioured similarly to the Arduino. The only big change was the operating voltage of 3.3V to keep an eye on, however this was trivial in the early days when I only had a couple of switches and LEDs to power.

> ![ESP32 NodeMCU](https://i.imgur.com/CDSgzhz.jpg)
>  
> The ESP32 (NodeMCU board)

However, there were big advantages to the ESP32 that many people explained on the [KTANE IRL Discord](https://discord.com/channels/711013430575890432). The ESP32 has much more memory available to use, directly solving one of the biggest problems with the Arduino version. An ESP could store a lot of big libraries, use lots of Strings and structs, and store much more to PROGMEM. My code no longer *needed* to be so lean to make sure there was space so in turn could be quicker and easier to read.

The ESP32 also has a dual-core architecture (plus an extra Ultra Low Power processor), in-built Wi-Fi and Bluetooth, and some boards can also have integrated cameras, SD card readers and other peripherals. At the time of switching, I wasn't planning on touching those features... but more on that later.

### Decentralisation
A lot of my Arduino code could be transferred across to the ESP32 version, however one of the first issues what that it was still not clear that a single ESP32 could run potentially 12 modules from a pool of 14 different module types, and how it could detect which modules were plugged in from the simple components in each module box. A solution to this was one ESP32 for each module, which would be specifically coded to run that module type only. However that introduced a new challenge; inter-module communications were now necessary.

Comms was something I'd seen in a couple of other KTANE-like projects online, however back when I started this seemed like pure magic. After spending a while wrestling with learning libraries and using shift registers with other components, I began to understand how things were put togethr and felt a little more confident in attempting it. Comms between the ESP32s was high on my to-do list, as it would underpin a lot of how the old code would be divided up across the new microcontrollers.

### Starting out

To start, the majority of the Arduino code was moved across to the ESP which would be seated in the Timer module. This would be the master. It makes sense - every bomb needs the Timer! The Timer would obviously track the game time and strikes, but also co-ordinate the other modules to track strikes, successes and setup.

Alongside this, the Keypad module was chosen to be imported, as it is a completely self-contained module. It doesn't need to know the time, the number of strikes, the serial number, etc. This means it could be tested in isolation of connection to the Timer.

One additional consideration was brewing in the back of my mind: how to setup modules which needed manual intervention. For a module such as password, the game generates a password and a list of characters to scroll through on the screen. This needs no manual setup as the ESP would just have to change what it shows on the screen during the game. However a module such as Keypad would need physical keys with the correct symbols attached to it before the game starts.

This manual setup was inplemented crudely on an LCD attached to the Mega in the prototype, but this was always a temporary solution and a much better one would have to be found now I was aiming a little higher with the quality of the game. I wondered how easy it would be to not show this info directly on a display of some sort which would need to be physically engineered into the module, but instead fetching that info digitally - much like using the serial monitor while debugging the ESP32 sketches.

While that idea was forming, I imported the Timer and Keypad modules pretty easily. And so it was time to start on the comms...

---

## 2021/01/23 - Origins
It's been about one year since I started this project, and so it feels like I should get this blog going with a recap of the journey so far. For the first entry, I'll revisit the Arduino prototype.

### Arduino
As stated on the Goals page, I received an Arudino starter kit for Christmas 2019 from my family. Not wanting to simply go through the rather basic tutorials (although to give them credit they were interesting concepts, just slow on the progress), I remembered my love of the game Keep Talking.

At its fundamental level, it was a game that came with a clear set of (codable) rules and was constructed from simple parts like lights, buttons and wires (and screens and motors but that's still not so crazy - the game was designed in a really grounded way).

I figured I could learn how to use these pieces as I go building up the game, module by module. I had a little grounding in electronics, having had a similar kit years previous (before Arduinos were a thing) and previous coding experience, but not in C++; enough to believe I could make some facsimile of the videogame bomb.

The Arduino Mega was a reasonable microcontroller to run the game. It had enough memory to keep hold the rules of many modules and keep track of all of the variables required during gameplay. With about 50 I/O pins, it could connect up to a reasonable impressive amount of electronics too.

### Building and learning
The starter kit had a good variety of components: LCD screens, LED segment displays, single LEDs, buttons and switches. I bought in a few other parts, thinking ahead to the look and feel of the modules, and began piecing things together.

> ![Early photo of the prototype](https://i.imgur.com/an2r3ZR.jpg)
>  
> An early photo of the prototype

The code was a mess and I was struggling with a few basics, but thanks to some good tutorials and libraries I managed to create a 4 module prototype on that Mega:
* Timer: with a nice big time display, two LED strike lights and a piezobuzzer for all sound effects (this was difficult to organise a priority system for more important sounds (strike buzzer) to play over less important ones (clock ticking)!)
* Wires: taking a guess at the generating algorithm, this would create a 3-6 wire sequence for the user to set-up before a game. The Mega would check inputs to see if the correct wires were (dis)connected to know it was properly set-up.
* Button: after learning how to do debouncing, a relatively simple module to create.
* Simon Says: the arcade buttons really completed the look, but this was a relatively tricky module which needed smart state tracking and timing for lighting up the buttons in sequence.
* Keypad: this was only half-implemented but playable, and made use of the custome character creation of the LCD screen used for setup to display the keypad symbols.

> ![Keypad setup screen](https://i.imgur.com/2fTtbNI.jpg)
>  
> Using the custom characters of the LCD to setup the Keypad module

### Reaching the limit
I originally wanted to create as much as I possibly could of the game, emulating the 14 modules as close as I physically could, plug-and-play modules, and the ability to have 6 or 12 modules in play at once. This was beginning to look like a huge stretch for one Mega to handle.

I started building a wooden case for the modules which did the job but it was a struggle with hand tools and I had no ability to use a makerspace in the start of the Covid lockdown. Then I moved the electronics into the case with a forward-looking view to achieving more than those 4 modules.

> ![Button in the wooden case](https://i.imgur.com/IVS5SpJ.jpg)
>  
> The Button module being tested in its wooden case

> ![The modules in their wooden cases](https://i.imgur.com/VMsLNAq.jpg)
>  
> The finished modules

In order to try to get the most out of the Mega, I refined the code to slim it down as much as possible and tried to extend the limited number of I/O pins to be able to maximise the amount of supported modules at one time. I hadn't done much soldering at this point, but designed two boards to help increase the number of I/Os and make that plug-and-play aspect a little easier.

> ![One of the socket boards](https://i.imgur.com/orsPxDr.png)
>  
> One of the socket plug-and-play boards (red pins denote power lines, green pins are inputs to the shift registers from other modules, blue pins are outputs from the shift registers to the Mega, magenta pins are the common lines to drive LCDs and cyan pins are individual chip selects so only a specific LCD is edited at any one time)

After discovering shift registers, I reimplemented the code to read in any input and push out and output through a register. Then devised a scheme where reigsters would all be chained to each other and divided into nibbles. So one shift in register could support the 4 buttons for Simon Says and 4 buttons from Keypad, for example. This way, the three I/Os needed to drive the chain of 3 registers could handle 24 input lines.

I also soldered a line of pins to act as an LCD bus, realising I could use many LCDs across the modules. Breaking out the chip select, I could keep the other lines common.

Despite all of this, it was obvious that the Mega would really struggle to get much further, and so I needed to make a choice on where to go next. I took the difficult decision to scrap the Mega prototype and go down a different path, one which could get me much closer to the original game.

In getting help breaking down the workings of the game and getting feedback on my build, I found Nick on the official KTANE discord who was also starting his real bomb build. As there were a few of us doing a similar project, he set up the [KTANE IRL Discord](https://discord.com/channels/711013430575890432) for us builders to talk shop.

He pointed me further down the rabbit hole, suggesting other microcontrollers and methods to use. My skill in coding and understanding electronics was still quite basic but had developed a lot since starting, and I was ready to attempt to use more complicated hardware and attempt the previously unthinkable communications protocols between mulitple microcontrollers!

The ESP32 version took shape and work began again...
