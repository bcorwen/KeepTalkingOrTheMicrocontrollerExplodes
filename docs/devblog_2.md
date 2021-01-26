[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: Origins](devblog_1.md) | [Next post: Communications (part 1): CAN](devblog_3.md)

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

While that idea was forming, I began importing the Timer and Keypad modules pretty easily. And so it was time to start on the comms...

> ![ESP32 timer and keypad modules](https://i.imgur.com/2LLJpaU.jpg)
> 
> The Timer and Keypad build using ESP32s

[Prev post: Origins](devblog_1.md) | [Next post: Communications (part 1): CAN](devblog_3.md)
