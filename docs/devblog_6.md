[Dev blog](devblog.md) | [Project aims and goals](goals.md) | [Project to-do](todo.md) | [Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK) | [KTANE IRL Discord server](https://discord.com/channels/711013430575890432)

---

# Dev blog
I'm currently making short update videos on progress on [my Youtube playlist](https://www.youtube.com/watch?v=8m7peVlW2mE&list=PLJqFvAhkcSkkks42zClG5WlvO1khFZCKK). It's certainly much quicker to film/narrate what's new than type up a new blog post! Check there for more up-to-date info, meanwhile a more detailed write-up will appear here going into much more detail.

---

[Prev post: Timer and Widgets module](devblog_5.md)

## 2021/04/26 - Object-Oriented Programming revamp, CAN shake-up and the Button module
Quite a varied blog post as you can tell by the title! I've had a lot of different projects going on since the last update in February, but dove in again recently to score some successes.

### Object-Oriented Programming (OOP) revamp
I've known about OOP methodology and built a few things that way, but not in this project. I have no formal programming training and so default to building longer and more complex procedures... and the KTOME project certainly does begin to balloon in size after adding comms and further modules into the mix! Although I was writing smart functions, the modules were one long file, making it hard to navigate and harder to follow the flow that the ESP would take through the code.

The straw that broke the camel's back was my attempt to add in a brief LED flash when the bomb explodes, for user feedback since he bomb wouldn't actually explode! Adding in this simple blink to my sprawling code was causing difficult to pin down occurrences, such as the lights staying on in some circumstances or not all of the lights activating on the cue. I needed to simplify the code, so turned to OOP.

Unfortunately, a code re-write doesn't really give you much to demonstrate, so sorry for the lack of pictures here! But my foray into OOP started with the construction of special LED and switch classes. I decided to have a plain LED class and augment that with more specialist classes for blinking or dynamic intensity (pulsating) lights. This way, I could call for a one-time blink on a bomb explosion and I wouldn't have to track the blink timer and update the LED with stray lines of code in the main file.

Then I decided to further build on the component-level classes with a module-level class. I pulled all of the Keypad logic out of the main file and wrote a Keypad class which would generate the symbols for each game, track the inputs and ultimately output a defuse or strike signal back into the main code.

So not only was the main body of code massively simplified and shortened, but nearly made it agnostic of the module type! This created the ideal basis to start to import the other modules from my prototype code, by changing a few lines and building a new module class.

### CAN shake-up

From the previous blog post detailing the way I had set out CAN, I decided to make a few changes to how addresses are assigned. The previous method was ideal for the way CAN is typically utilised, however there was a bug in the library when using the extended IDs meaning I had to implement the bit mask and matching manually. In that case, why stick to the typical implementation?

I decided to keep bits for each module, however the unique module IDs are counted across the last few bits. This allows me to assign more bits to module types and so reserve bits for building modded or custom modules in the future.

### The Button module

Here's the bigger piece of progress that is much easier to show!

I'd hoped to use a lot of the prototype code to get the button up an running, however the only part I ended up being able to use was the function which looks at the button characteristics and widgets to decide what the correct input should be. The switch to OOP and multi-MCU layout meant it was easier to re-write the rest.

My custom LED class allows easy setting of pulsating light patterns, so emulating that fading up and down of the strip light. And my switch class can fetch the time of last state change, so can find when that button is held down long enough to be considered a long hold rather than a short push.

The biggest change was needing to ensure the Button module received timely information over the CAN bus.
One piece of info is the widget info to determine the correct input. The current game setup flow has the modules requested to create a game before the widget information is sent out by the Timer module. This means that the Button logic is split into two parts:
* The game creation call prompts the Button to assign a button colour and label. This isn't enough on its own to calculate the correct input for this game yet.
* After all modules needing manual setup are satisfied, and the game can be started from the app, the Timer will send the widget info to all modules. This is when the Button receives the final pieces of the puzzle and can work out the correct input for the game.

The other piece of info to fetch is the digits displayed on the Timer. The correct timing to release a long hold is dependent on whether the displayed time has a particular digit. Currently, when the held button is released, a timer request is sent to the Timer module which replies to the Button with the displayed digits. The Button can then check if the correct digit is present before declaring the solve or strike. This requires more testing at the time of writing this, but the response feels very fast. One failsafe I have already programmed is the case when a Button is solved as the last module, but the delay moves the timer on so it apperas as if the Button was solved when it shouldn't. When the timer digits are requested, the Timer stores the digits in a variable. If the module which requested those digits transmits a solve and is the last module to be solved, then the timer display will freeze on the stored digits rather than whatever the timer might have ticked over onto.

> ![Button strip lighting up during a long hold](https://imgur.com/OInxHsp)
>  
> Button module testing, with the strip lighting up during a long hold

More testing is required before I can call the Button done, but it behaves very closely to what's intended and so a video will be put up when it's all ready!

The most encouraging thing about the Button was that it only took about 3 hours to code and build on the breadboard. The OOP structure of the Keypad meant I had a very easy basis to use for the Button, and this will only get easier to recreate the other modules.

[Prev post: Timer and Widgets module](devblog_5.md)
