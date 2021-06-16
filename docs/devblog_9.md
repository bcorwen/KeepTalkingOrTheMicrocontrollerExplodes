[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: The Simon module](devblog_8.md)

## 2021/06/14 - The Wires module
Another quick module (barring the debugging time after finding some of my rules were slightly out!) which completed the fourth module and all of the modules previously completed in the prototype.

The code is pretty short and sweet, with the longest and most complex piece being the rules to determine the correct wire to cut.

> ![Wires module on the breadboard](https://i.imgur.com/xymZz9t.jpg)
> 
> Wires module on the breadboard

One of the most time consuming parts of bringing this module fully online is preparing the phone app. As this module needs manual setup, the wire colours needed to be relayed through the Timer ESP32 and on to the phone app for display. This is tricky in the graphical coding interface of the Kodular site, however was up and running pretty quickly.

> ![Phone app setup screen for Wires](https://i.imgur.com/mdUHj9m.png?)
> 
> Phone app setup screen for Wires

One key deviation from Wires and the other manual setup modules so far is the fact that this module can test and should verify the connected wires before game start. Say it expects 5 wires with the 3rd connection empty. Well the ESP can detect that and when the user presses the button on the app saying that the module is setup, then the ESP can double check this. It could be a very important check to avoid an instant strike during gameplay due to a faulty connector. This caused a huge headache with the Timer code so far, and so got a huge overhaul to have a better flow as messages pass over CAN and BLE.

It's been tested to work with actual wire cutters and has been coded to ignore any disconnected wires reconnecting (say if you pulled a wire out but it momentarily connected back up, which is easy to do on the timescales microcontrollers work). It's a satifying "snip" sound to defuse the module and the whole bomb too!

Now I'm sick of looking at breadboards and a tangle of wires. Why not try to switch tack away from electronics and coding, and on to something more physical?

[Prev post: The Simon module](devblog_8.md)
