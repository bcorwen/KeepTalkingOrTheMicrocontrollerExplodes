[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: Communications (part 2): BLE](devblog_4.md) | [Next post: Object-Oriented Programming revamp, CAN shake-up and the Button module](devblog_6.md)

## 2021/02/06 - Timer and Widgets module
This is another catch-up update covering about August 2020 to January 2021.

### The Timer
Not exactly a module, as there is no puzzle to defuse, the Timer is one of the common components in any bomb. There will always be the need for timer display! Because of its requirement in very game, it became the obvious choice to bear the game logic and be the comms hub.

The Timer holds a variable to describe the game state:
* Comms setup - Waits for the phone app to pair and allows time for other modules to initialise their CAN nodes.
* Game setup - Handles activities to set up a new game, taking input from the phone app's game manager screen, polling to find other connected modules and co-ordinating manual setup. The Timer also creates the non-module setup, including generating a serial code and widgets.
* Game running - Runs the game clock, the physical displays of the Timer module, co-ordinates messages with the other modules and tracks defusal and strike events.
* Game debrief - Responsible for ending the game, halting the other modules, feeding info to the app for debrief, and resetting for the next game.

On the physical side, there are three components:
* The time display - I have used the [Adafruit 1.27" 4-digit 7-segment display with I2C backpack](https://www.adafruit.com/product/1270). It comes with a handy controller and library, looks fairly similar to the game's display and is a reasonable size (looking pretty large but would not inflate the size of the Timer module to be able to house it).
* The strike display - These are odd x-with-a-strikethrough shaped displays. I'm currently using a 2-digit 14-segment display to create this shape, which are also convenient due to library support and don't require something custom to be created to get the desired effect.
* A buzzer/speaker - For the Arduino version, I had been using a piezo buzzer which did a basic job, however the Timer has no functional speaker (as we'll see later).

It was planned for the Timer module to be self-contained, so allowing the module to be positioned anywhere in the bomb chassis. For the Timer to generate **and** control the serial number e-ink display, there would have to be another specialised bus wired in between all module slots and widget slots. This seemed very impractical, and so the serial number control was broken out into a new ESP32.

> ![Timer, Widget and Keypad modules](https://i.imgur.com/PVWNPcI.jpg)
> 
> The Timer (top), Keypad (right) and Widget (bottom) modules.

### The Widget controller
This controller, assisting the Timer with common game tasks, is designed to sit permanently in the case to interface with the widget slots in the chassis.

Currently, there is no functionality to show indicators, ports or batteries (these are currently intended to be changed manually), but this Widget controller has two main tasks:
* Serial number display
* Audio output

#### Serial display
[Syber from the Discord server](https://discord.com/channels/711013430575890432) came up with the brilliant idea of using an e-ink display to emulate the serial number sticker on the bomb case. These can be a little sluggish to update, but this only needs to happen once before the game starts, and they can look quite convincing as a passive paper sheet! This also removes a time consuming manual intervention to get the game ready. I'm using the [2.9" three colour Waveshare e-ink display](https://www.waveshare.com/2.9inch-e-paper-module-b.htm) which can conveniently emulate the white, black and red colours of the sticker.

Syber also did a great job of copying the fonts into the [u8g2 library](https://github.com/olikraus/u8g2). This library is pretty big, and understandably so having the ability to draw to a huge number of displays in a variety of fonts and drawing functions - which was another driving factor to break the serial number controller out from the Timer.

The Timer generates a serial number for a new game, passes this to the Widget ESP32 via CAN, then the Widget ESP can update the e-ink display.

#### Audio output
Although possible to put a speaker in the Timer (and I may still do this as an addition), it felt sensible to also move this function to the Widget controller. As this will be permanently seated in the case, so too can some good-sized speakers. And since this ESP32 will only have the serial number display to update, it will have a lot of free time (and memory) to play sound cues. Currently, the sound cues are some common game sounds (ticking, strike warnings and an explosion) but expanding this to cover other game sound effects will be trivial.

The audio amp is the [PAM8403](https://components101.com/modules/pam8403-stereo-audio-amplifier-module), driving an old laptop speaker for the moment. The sound library was created by [XTronical](https://www.xtronical.com/the-dacaudio-library-download-and-installation/) which is very powerful, being able to play multiple sounds at once from hex-encoded .wav files.

The Widget controller listens on the CAN bus for cues from the Timer, then plays sound files with little overall delay according to a specific audio trigger (tick sound when the timer ticks over a second) or other game triggers (strike sound when a strike message is broadcast). As the Widget controller doesn't have to send a message, it is given a blank (all-zeros) CAN ID and mask.

[See it all in action here!](https://www.youtube.com/watch?v=DGMcetRa-00)

[Prev post: Communications (part 2): BLE](devblog_4.md) | [Next post: Object-Oriented Programming revamp, CAN shake-up and the Button module](devblog_6.md)
