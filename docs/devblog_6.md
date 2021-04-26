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

I decided to keep bits for each module, however the unique module IDs are counted 

### The Button module


[Prev post: Timer and Widgets module](devblog_5.md)
