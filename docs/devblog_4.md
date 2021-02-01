[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

> WIP post!

[Prev post: Communications (part 1): CAN](devblog_3.md)

## 2021/02/02 - Communications (part 2): BLE
This is another long overdue update covering about August to November 2020.

### On the subject of manual setup
As hinted at in previous posts, you can categorise modules into two groups: those that need manual setup and those which do not.

In the videogame, each bomb is randomly generated. You start a new game, you are handed a bomb with random modules, and on those modules are random coloured wires / symbols / buttons etc.

There is a choice to be made on where to draw the line between the game being generated by the ESP32 and where it is built by the user. Other builders on the Discord server have different opinions, but my approach is to have the computer do most of this generation. By taking it out of the hands of the user, that same user can enjoy a more random experience and not influence the gameplay (knowingly or subconsciously).

The flip side to having the computer generate the game is that there is more manual setup required.  For a module such as password, the game generates a password and a list of characters to scroll through on the screen. This needs no manual setup as the ESP would just have to change what it shows on the screen during the game. However a module such as Keypad would need physical keys with the correct symbols attached to it before the game starts. This is an inconvenience which I believe is worth the downsides!

However, there is the question about how the ESP32 is going to inform the user which keys need to be attached to the Keypad module. While building the Arduino prototype, I started displaying much of the info to the serial monitor (a text window on the computer showing messages from the microcontroller). However, I knew this needed to be more physical and my solution, at that time with my limited resources and knowledge, was an LCD screen to display the relevant data. This was fine for the bare bones physical layout I had, but would be difficult to integrate such a useful display into the look of the bomb prop while keeping it as close to the original as possible. A random screen might distract a player! I had also thought about trying to double the screen up as the serial number display, however the LCD is also quite limited in what it can display.

> ![The Keypad symbols on the LCD](https://i.imgur.com/2fTtbNI.jpg)
> 
> Trying to show the Keypad symbols on an LCD. I used custom characters but it was still a challenge to decypher!

[Prev post: Communications (part 1): CAN](devblog_3.md)