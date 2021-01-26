[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

> WIP post...

[Prev post: Moving to the ESP32](devblog_2.md)

## 2021/01/26 - Communications (part 1): CAN
Another catch-up update covering about August to November 2020.

### The need for comms
As discussed in the [previous post](devblog_2.md), now I would be using one ESP32 per module there would need to be some way for the microcontrollers to communicate with one another. Having converted a good portion of the old code to create basic Timer and Keypad modules, I began to break down potential types of messages that would need to be exchanged to ensure a game flow and consistent states across the system.

### Communication options
After a quick search as for communication options, there appeared to be three common protocols used with microcontrollers:
- I2C - Inter-Integrated Circuit communication
- SPI - Serial Peripheral Interface
- CAN - Controller Area Network

Although I2C and SPI were by far the most used protocols, being used to support data transfer with many displays and chips, they both appeared the weakest choice:

I2C is a multi-master, multi-slave architecture, with two lines: data and clock. Devices are assigned an address, allowing the aiming of messages to certain devices only. I started exploring this as a promising protocol... However discovered that the slave-side implementation of the protocol was not supported by the ESP32!

SPI uses a single-master, multi-slave architecture, with two data lines (Master-In Slave-Out and Master-Out Slave-In), one clock line and slave-select lines. Typically, SPI is used with one slave-select line per slave. The master would set a line high if it wanted to communicate with that slave. So for this application, assuming a maximum of 11 modules alongside the Timer, that's a lot of pins! There are alternatives to this, such as creating a daisy chain where messages to circulated around each device, however this wasn't the largest issue. SPI is driven by the master. For example: a Keypad (slave) records a strike and wishes to send this to the Timer (master), it cannot choose to do so. It will need to wait for the master to select this device, open communications and then read from the slave. So a couple of challenges present to work around with this protocol.

Looking into CAN, it appeared a perfect fit. Any microcontroller could initiate sending a message and, with its flexible addressing system, could target that message to one or more particular devices. Although requiring CAN bus chips to be wired up, CAN is natively supported on the ESP32, including buffers, interrupts and checks to ensure the line is free before attempting to send a message.

### CAN basics
So a little more on how CAN works, which will set the scene for how I used the protocol to send messages for KTOME:

#### IDs and masks
Devices connected to a CAN bus will have an ID: an 11-bit string (or a 29-bit string when "extended frame" IDs are used). Individual messages are sent with an ID attached, and the connected device will filter out any messages which do not match the ID exactly. Hmm ok... well how can a message with one ID be received by multiple devices with different IDs?

Alongside the ID, a device may optionally register a mask of the same length as the ID. This bit-mask acts as a filter over the message. Any bits marked with a 0 in the mask will not be checked when comparing the message ID with the device ID.

Example 1: Basic ID matching

Source | ID | Comment
--- | --- | -------
Message | 0b00010011110 | -
Device 1 ID | 0b00010011010 | 9th bit mismatch: message ignored
Device 2 ID | 0b00000011110 | 4th bit mismatch: message ignored
Device 3 ID | 0b00010011110 | All bits match: message read

Example 2: IDs and masks

Source | ID | Comment
--- | --- | -------
Message | 0b00010011110 | -
Device 1 mask | 0b11111111111 | All bits compared
Device 1 ID | 0b00010011010 | 9th bit mismatch: message ignored

Source | ID | Comment
--- | --- | -------
Message | 0b00010011110 | -
Device 2 mask | 0b11111110000 | First 7 bits compared, last 4 ignored
Device 2 ID | 0b00010011010 | 9th bit mismatch ignored: message read

Source | ID | Comment
--- | --- | -------
Message | 0b00010011110 | -
Device 3 mask | 0b00000000000 | No bits compared!
Device 3 ID | 0b11101011000 | Message and device IDs very different but message read!

#### Message characteristics


### How I used CAN for KTOME


[Prev post: Moving to the ESP32](devblog_2.md)
