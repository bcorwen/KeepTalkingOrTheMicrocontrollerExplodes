[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: Moving to the ESP32](devblog_2.md)

## 2021/01/31 - Communications (part 1): CAN
Another catch-up update covering about August to November 2020.

### The need for comms
As discussed in the [previous post](devblog_2.md), now that I would be using one ESP32 per module there would need to be some way for the microcontrollers to communicate with one another. Having converted a good portion of the old code to create basic Timer and Keypad modules, I began to break down potential types of messages that would need to be exchanged to ensure a game flow and consistent states across the system.

> ![Flow diagram](https://i.imgur.com/ritYqLI.jpg)
> 
> An early flow diagram showing messages moving between devices.


### Communication options
After a quick search as for communication options, there appeared to be three common protocols used with microcontrollers:
- **I2C** - Inter-Integrated Circuit communication
- **SPI** - Serial Peripheral Interface
- **CAN** - Controller Area Network

Although I2C and SPI were by far the most used protocols, being used to support data transfer with many displays and chips, they both appeared the weakest choice:

**I2C** is a multi-master, multi-slave architecture, with two lines: data and clock. Devices are assigned an address, allowing the aiming of messages to certain devices only. I started exploring this as a promising protocol... However discovered that the slave-side implementation of the protocol was not supported by the ESP32!

**SPI** uses a single-master, multi-slave architecture, with two data lines (Master-In Slave-Out and Master-Out Slave-In), one clock line and slave-select lines. Typically, SPI is used with one slave-select line per slave. The master would set a line high if it wanted to communicate with that slave. So for this application, assuming a maximum of 11 modules alongside the Timer, that's a lot of pins! There are alternatives to this, such as creating a daisy chain where messages to circulated around each device, however this wasn't the largest issue. SPI is driven by the master. For example: a Keypad (slave) records a strike and wishes to send this to the Timer (master), it cannot choose to do so. It will need to wait for the master to select this device, open communications and then read from the slave. So a couple of challenges present to work around with this protocol.

Looking into CAN, it appeared a perfect fit. Any microcontroller could initiate sending a message and, with its flexible addressing system, could target that message to one or more particular devices. Although requiring CAN bus chips to be wired up, CAN is natively supported on the ESP32, including buffers, interrupts and checks to ensure the line is free before attempting to send a message.

### CAN basics
So a little more on how CAN works, which will set the scene for how I used the protocol to send messages for KTOME:

#### IDs and masks
Devices connected to a CAN bus will have an ID: an 11-bit string (or a 29-bit string when "extended frame" IDs are used). Individual messages are sent with an ID attached, and the connected device will filter out any messages which do not match the ID exactly. Essentially:
```
if (message_id == device_id) {
  // Message is accepted
}
```
To show an example:

> Example 1: Basic ID matching
> 
Source | ID | IDs equal?
--- | --- | ------- 
Message | 0b00010011110 | - 
Device 1 | 0b00010011010 | 9th bit mismatch: message ignored
Device 2 | 0b00000011110 | 4th bit mismatch: message ignored
Device 3 | 0b00010011110 | All bits match: message read

Hmm ok... well how can a message with one ID be received by multiple devices with different IDs?

Alongside the ID, a device may optionally register a mask of the same length as the ID. This bit-mask acts allows for partial matches to be done. Any bits marked with a '0' in the mask will not be checked when comparing the message ID with the device ID. So now the pseudo-code would be:
```
if ((message_id & device_mask) == (device_id & device_mask)) {
  // Message is accepted
}
```
>Example 2: IDs and masks
> 
Source | ID | mask | ID & mask | IDs equal?
--- | --- | --- | ------- | --- 
Message | 0b00010011110 | - | 0b00010011110 | - 
Device 1 | 0b00010011010 | 0b11111111111 | 0b00010011010 | 9th bit mismatch: message ignored
> 
> This case was the same as not setting a mask, as every bit is compared.
> 
Source | ID | mask | ID & mask | IDs equal?
--- | --- | --- | ------- | --- 
Message | 0b00010011110 | - | 0b00010010000 | - 
Device 2 | 0b00010011010 | 0b11111110000 | 0b00010010000 | All bits match: message read
> 
> The last 4 bits are not compared, enabling this device with a different ID to still accept this message.
> 
Source | ID | mask | ID & mask | IDs equal?
--- | --- | --- | ------- | --- 
Message | 0b00010011110 | - | 0b00000000000 | - 
Device 3 | 0b00010011010 | 0b00000000000 | 0b00000000000 | All bits match: message read
> 
> An all-zero mask will allow the device to read every message, no matter its ID!

#### Message characteristics
Message IDs are followed by the body of the message. CAN messages are pretty short: only 8-bytes maximum! So you won't be able to send a huge status update with one message, however they are quick to send. It appears appropriate to use CAN to send quick flags, triggers and other short pieces of info, but so long as we're smart about encoding it down to keep it brief. Better yet, we can use the IDs to assist in keeping things moving by being smart about where messages are sent.

### How I used CAN for KTOME
Firstly, credit to where credit's due. I used the CAN library for ESP32 forked by [timurrrr](https://github.com/timurrrr/arduino-CAN), originally from [Sandeep Mistry](https://github.com/sandeepmistry/arduino-CAN). These were excellent at keeping things simple, with easy function calls to set up the CAN bus on the right pins, sending and receiving messages.

For hardware, all the ESP32 needed as an extra was a bus board. I chose the cheap and simple [TJA1050 CAN chips](https://www.amazon.co.uk/gp/product/B07VJ6XF92/) which worked without problems - although I did need to set up a 5V line to power these chips, and they were not spaced to fit breadboards, leaving them dangling at the end of cables.

#### Allocating IDs
Since the prototype, I had been thinking about how to get a plug-and-play architecture with as little manual intervention required, it was clear that the CAN IDs would be ideal to become a distinguishing characteristic. After a few attempts at drawing up different ways of using the IDs, I settled on using the following key:

CAN ID bit | '1' in this bit means...
--- | ---
29 | Timer (master)
28 | Wires
27 | Button
26 | Keypad
25 | Simon Says
24 | Who's on First
23 | Memory
22 | Morse
21 | Complicated Wires
20 | Wire Sequence
19 | Maze
18 | Password
17 | Venting Gas
16 | Capacitor Discharge
15 | Knobs
14 | (not used)
13 | (not used)
12 | (not used)
11 | Unique ID #1
10 | Unique ID #2
9 | Unique ID #3
8 | Unique ID #4
7 | Unique ID #5
6 | Unique ID #6
5 | Unique ID #7
4 | Unique ID #8
3 | Unique ID #9
2 | Unique ID #10
1 | Unique ID #11

The first 18 bits are used to define the module type (with 3 of these currently unused but available for future modded or custom modules. The remaining 11 bits denote unique identifiers for each module type. This means a bomb with multiple Keypad modules would be able to distinguish the two modules by the position of their unique identifier bit.

For example:
`0b10000000000000000000000000000` is the Timer
`0b01000000000000000010000000000` is the 1st Wires module
`0b01000000000000000000001000000` is the 5th Wires module

A special case: the ESP32 controlling the Widgets is given a blank ID (all zeroes). It will also always be present but will not transmit any messages, and so this ID is acceptable as we'll discuss later.

It would have been a more economical use of the ID to consider assigning the totals rather than individual bits, e.g. 0b0001... is the Timer, 0b0010... is Wires, 0b0011... is Button, 0b0100... is Keypad, etc. However doing so would have made it challenging to target messages as specific modules, therefore the bits were considered separately.

#### Designing the messages
The 8-byte maximum CAN messages would need to be planned out to prevent multiple messages having to be sent when not necessary, helping to prevent the bus becoming too burdened.

I planned out a system where the majority of messages could be communicated in one byte, assigned a letter to keep it easy to remember and use. Messages from the master would be upper case and messages from slaves would be lower case. Some comms would require more info than just stating an event has occurred, so some would have a defined use for the remaining bytes.

Message to send | CAN message | Comments
--- | --- | ---
Master to poll if modules is connected | P | 
Module to reply that it's connected | p | 
Master to have modules initialise a game | I | 
Module to reply when it's initialised a game | i | 
Master to ask module to manual setup info | C | 
Module to reply with manual setup info | c... | The content following the 'c' is module specific, e.g. wire colours
Master to confirm manual setup aligned with inputs | M | 
Module does not confirm inputs match | m0 | 
Module confirms inputs match expected setup | m1 | 
Master to send game start | A | 
Master to send game stop | Z$ | The character following 'Z' denotes whether bomb exploded, bomb defused or game aborted
Master to send edgework setup | W##### | Characters after 'W' denote counts for batteries, ports, useful indicators, serial number factors
Master to send serial number | S###### | Characters after 'S' denote serial number
Master to send strike count | X# | Number after 'X' denotes the number of strikes
Master to send heartbeat for ticking sound | H | Send every second decrement on the timer, to trigger ticking
Module to tell master of a strike | x | 
Module to tell master of a defusal | d | 
Module to request time from master | t | 
Master to transmit time on display | T#### | Numbers after 'T' denote the digits of the timer display

#### How CAN comms were used in the script
I created a short library to use CAN in the project. Although the main CAN library was already pretty simple, I could incorporate some formatting and quality of life improvements, such as the option to serial print the message contents and IDs being sent and received. It also contained some definitions for the IDs, as explained above.

Modules would be registered to the bus with its type and a unique number, e.g. A Keypad module might be `0b00010000000000000000000000000 & 0b00000000000000000010000000000 = 0b00010000000000000010000000000` . The mask would typically equal the ID, resulting in the module only listening out for any message which contained the module's ID, e.g. A message with ID `0b00010000000000000011111111111` is designed to target every Keypad module (as it is composed of the Keypad bit following by all unique identifier bits, so it would be accepted by the previous Keypad module.

As the timer is assigned the unofficial "Master" of the network, it typically initiates a back-and-forth with the other modules.

One of the first messages the Timer would be a 'P' to see which modules are connected to the bus. As this needs to pick up every module, it sends 'P' with the message ID of `0b01111111111111111111111111111` - encompassing the ID of every module, bar the Timer itself.

A slave module, when replying to the Timer, is programmed to include its own ID in the message ID. For example, on receiving the 'P' message, the Keypad replies to the Timer to let it know it is connected. It sends 'p' in a message with ID `0b10010000000000000010000000000`. The Timer receives this as the first bit is a one, it removes the first one and so it is left with the ID of the Keypad! The Timer can now store this ID and remember which modules are plugged in, and so can tell how many modules are attached, which module types and specifically address this module in particular (when the manual setup of attaching the correct Keypad keys is required).

Many of the messages are obviously exchanged in the order they appear in the above table, from registering modules, to asking them to set up a game, to starting a game. Some messages occur on specific events, such as a module declaring the user has entered a wrong input so sends a 'x' strike to the Timer, which is then followed up by the Timer broadcasting the strike total to all modules who require it (such as Simon Says).

### Wrap-up
The CAN was a pretty easy protocol to setup, however needed a fair bit of planning (and a couple of revisions) to make best use of it and to tidy up any omissions.

One of my brief video updates covers much of what is discussed here in context; you can view it [here](https://www.youtube.com/watch?v=9nwYC-B_rjc), as well as a few smaller pieces of CAN usage in the following two videos in the [project playlist.](https://www.youtube.com/watch?v=X2lTdU5nDYY&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK&index=5)

And since I started with the Keypad module as our only slave and that we have been mentioning manual setup in this post and last, it's no spoiler that I needed to test that Keypad module by knowing which symbols are being generated and whether it is correctly striking or defusing. After thinking of options, it seemed plausible to create a phone app which could pair to the Timer via Bluetooth and give the user a visual read-out of the Keypad symbols. The next blog post will cover the creation of the companion app, how to communicate with the ESP32 with Bluetooth Low Energy, and how the message protocol was created.

[Prev post: Moving to the ESP32](devblog_2.md)
