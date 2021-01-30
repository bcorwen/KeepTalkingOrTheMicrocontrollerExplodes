[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

> WIP post...

[Prev post: Moving to the ESP32](devblog_2.md)

## 2021/01/31 - Communications (part 1): CAN
Another catch-up update covering about August to November 2020.

### The need for comms
As discussed in the [previous post](devblog_2.md), now I would be using one ESP32 per module there would need to be some way for the microcontrollers to communicate with one another. Having converted a good portion of the old code to create basic Timer and Keypad modules, I began to break down potential types of messages that would need to be exchanged to ensure a game flow and consistent states across the system.

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
Message IDs are followed by the body of the message. CAN messages are pretty short: only 8-bytes! So you won't be able to send a huge status update with one message, however they are quick to send. It appears appropriate to use CAN to send quick flags, triggers and other short pieces of info, but so long as we're smart about encoding it down to keep it brief. Better yet, we can use the IDs to assist in keeping things moving by being smart about where messages are sent.

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

The first 18 bits are used to used to define the module type (with 3 of these currently unused but available for future modded or custom modules. The remaining 11 bits denote unique identifiers for each module type. This means a bomb with multiple Keypad modules would be able to distinguish the two modules by the position of their unique identifier bit.

>For example:
0b10000000000000000000000000000 is the Timer
0b01000000000000000010000000000 is the 1st Wires module
0b01000000000000000000001000000 is the 5th Wires module

A special case: the ESP32 controlling the Widgets is given a blank ID, as it will also always be present but will not transmit any messages.

#### Designing the messages


#### How CAN comms were used in the script



[Prev post: Moving to the ESP32](devblog_2.md)
