[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Current Tasks

- [ ] : Refine sound cues by adding local back-up timer to widget ESP
- [ ] : Alter sound amp circuit to make full range of input voltage range
- [ ] : Tweek CAN setup (possibly pull out into another header or otherwise make simpler to define)
- [ ] : Insert game over effects (shut down all lights and flash timer on success, or flash all lights briefly before going dark on failure)
- [ ] : Distil Keypad module down to a generic template as basis for other modules
- [ ] : Convert previously coded modules to work on ESP and interact with CAN bus (Button, Wires and Simon says)

---

# Project To-do List

## Programming

- [x] : Basic game logic common to all modules
- [x] : CAN communication between modules
- [x] : BLE communication between Master ESP32 and phone app
- [x] : Complete timer (Master) module
- [ ] : Complete standard modules (in progress)
- [ ] : Complete needy modules

## Design

- [ ] : Basic module designs
- [ ] : Case and edgework
- [ ] : Find electronics which matches game as closely as possible (in progress)
- [ ] : Power solution (in progress - 5V 18600 cell likely)
- [ ] : Module connectors (in progress - USB type-A)
- [ ] : Complete standard modules
- [ ] : Complete needy modules
