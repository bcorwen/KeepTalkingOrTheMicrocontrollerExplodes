[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: The Button module](devblog_7.md)

## 2021/06/11 - The Simon module
The Simon module also didn't take long to code, however getting around to this post did! I had this one running back in early May but it's only now I've remembered to write it up.

Just like the Button, I had this module working in the prototype, and yet the changes in the structure meant that very few lines of code were directly reused. The custom LED library I wrote for this project was excellent in handling the light blinking, so although there wa sa lot to re-write, it was fairly painless to get running quickly!

> ![Simon test with the Debugger](https://i.imgur.com/nY7y3fK.mp4)
>  
> Simon test with the Debugger

The key feature to ensure was working was the CAN comms. Much like the button, the solution to Simon requires some additional info, namely the presence of a vowel in the serial number and the current number of strikes. The module would listen out for these prompts on the CAN bus, extract the data and update the solution table.
An additional CAN feature (which I'd forgot to plan in) was audio. Simon makes tones with the lights, however I hadn't planned on a specific amp and speaker solution like the Widgets module. Initial tests with the simple pizeo buzzers were disappointing; the buzzer is designed for simple, high frequency sounds, and the lowest Simon notes were very quiet due to the frequency response. As the Widget already has the sound library and speaker, new CAN messages could initiate Simon tones without much delay.
Maybe a video of the three modules up to this point would demonstrate [something old, and something new](https://www.youtube.com/watch?v=sJcx_w03BgA).

As Simon only took a couple of days to build and another one to polish, it was time to move on to finish the last module I coded in the prototype - The Wires.

[Prev post: The Button module](devblog_7.md)
