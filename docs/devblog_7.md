[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: OOP revamp, CAN shake-up and the Debugger](devblog_6.md)

## 2021/04/27 - The Button module
The Button didn't take long to code so here's another post!

I'd hoped to use a lot of the prototype code to get the button up an running, however the only part I ended up being able to use was the function which looks at the button characteristics and widgets to decide what the correct input should be. The switch to OOP and multi-MCU layout meant it was easier to re-write the rest.

My custom LED class allows easy setting of pulsating light patterns, so emulating that fading up and down of the strip light. And my switch class can fetch the time of last state change, so can find when that button is held down long enough to be considered a long hold rather than a short push.

The biggest change was needing to ensure the Button module received timely information over the CAN bus.
One piece of info is the widget info to determine the correct input. The current game setup flow has the modules requested to create a game before the widget information is sent out by the Timer module. This means that the Button logic is split into two parts:
* The game creation call prompts the Button to assign a button colour and label. This isn't enough on its own to calculate the correct input for this game yet.
* After all modules needing manual setup are satisfied, and the game can be started from the app, the Timer will send the widget info to all modules. This is when the Button receives the final pieces of the puzzle and can work out the correct input for the game.

The other piece of info to fetch is the digits displayed on the Timer. The correct timing to release a long hold is dependent on whether the displayed time has a particular digit. Currently, when the held button is released, a timer request is sent to the Timer module which replies to the Button with the displayed digits. The Button can then check if the correct digit is present before declaring the solve or strike. This requires more testing at the time of writing this, but the response feels very fast. One failsafe I have already programmed is the case when a Button is solved as the last module, but the delay moves the timer on so it apperas as if the Button was solved when it shouldn't. When the timer digits are requested, the Timer stores the digits in a variable. If the module which requested those digits transmits a solve and is the last module to be solved, then the timer display will freeze on the stored digits rather than whatever the timer might have ticked over onto.

> ![Button strip lighting up during a long hold](https://i.imgur.com/OInxHsp.jpg)
>  
> Button module testing, with the strip lighting up during a long hold

More testing is required before I can call the Button done, but it behaves very closely to what's intended and so [maybe a video](https://www.youtube.com/watch?v=L1Xfc9mjphY&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) will show off the details a little better than this blog post.

The most encouraging thing about the Button was that it only took about 3 hours to code and build on the breadboard. The OOP structure of the Keypad meant I had a very easy basis to use for the Button, and this will only get easier to recreate the other modules.

[Prev post: OOP revamp, CAN shake-up and the Debugger](devblog_6.md)
