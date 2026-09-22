# The CMC keyword dictionary

Every word in the language with its meaning, syntax, example, and tips.
The website version (with one page per word) lives in
[`website/wiki/words.html`](../website/wiki/words.html).

Words are grouped by what they do.

| Category | Words |
| --- | --- |
| Talking | `oga`, `blorp` |
| Boxes | `grunk` |
| Choices | `binga`, `wonga`, `unga` |
| Logic | `and`, `or`, `not` |
| Loops | `booga`, `zug`, `zoop`, `in` |
| Clumps | `clump`, `ork` |
| Values | `gronk`, `nork`, `plop`, `pi` |
| Piles | `snorf`, `plop_call`, `nom`, `skoop`, `yoink` |
| Text | `goop`, `shout`, `whisper`, `flip`, `find`, `split`, `join`, `what` |
| Numbers | `munga`, `numba`, `round`, `flat`, `roof`, `abs`, `small`, `big`, `root`, `pow` |
| Drawing | `skrib`, `wait` |
| Notes | `ugg` |

## Talking

### `oga` (english: `print`, `say`)

**Say something on the screen.**

```cmc
oga <thing>
oga <thing>, <thing>, <thing>
```

`oga` is how a CMC program talks to you. Everything after it gets printed on one line, with spaces between the things.

You can `oga` numbers, text, piles, `gronk`, `nork`, `plop`, and boxes. A box shows what is *inside* it.

Example:

```cmc
oga "Hello cave!"

grunk name = "Ugg"
oga "Hi", name, "you are", 3, "years old"
```

Tips:
- `oga` with nothing after it prints an empty line.
- Quotes make text. `oga hi` looks for a box named `hi`; `oga "hi"` prints the word.

See also: `goop`, `blorp`, `unga`

### `blorp` (english: `ask`)

**Ask the user a question and get their answer as text.**

```cmc
blorp "What is your name?"
blorp()
```

`blorp` shows a question and waits for the person to type an answer. The answer comes back as text (goop).

In OGABOOGA CODER a friendly question window pops up. In the terminal the person types into the console.

Example:

```cmc
grunk name = blorp "What is your name?"
oga "Ooga", name, "!"
```

Tips:
- The answer is always text, even if it looks like a number. Use a number box if you need math.
- You can blorp with no question: `grunk answer = blorp()`

See also: `grunk`, `goop`, `oga`

## Boxes

### `grunk` (english: `let`, `make`)

**Make a box and put something in it.**

```cmc
grunk <name> = <thing>
```

A *box* remembers something for later. `grunk` makes a new box.

Box names are made of letters, numbers, and `_`, and they cannot start with a number. `grunk x = 5` makes a box named `x` holding 5.

To put something new into a box that already exists, just write `x = 6` without `grunk`. That is called *changing* the box.

Example:

```cmc
grunk age = 4
grunk name = "Ugg"
oga name, "is", age

age = age + 1   ugg a year passed
oga name, "is now", age
```

Tips:
- Using `grunk` again on the same name makes a fresh box (the old thing is forgotten).
- Changing a box that was never made says OOGA! -- make it first.

See also: `oga`, `clump`, `plop`

## Choices

### `binga` (english: `if`)

**Do something only when a check is gronk.**

```cmc
binga <check>
    <things to do>
unga
binga <check>
    <things>
wonga
    <other things>
unga
binga <check>
    <things>
wonga binga <check2>
    <more>
unga
```

`binga` asks a yes/no question and only does the things inside when the answer is `gronk` (true).

Use `wonga` to say what to do when it is not gronk. Use `wonga binga` to check something else first, like 'else if'.

Every `binga` needs its own `unga` at the end.

Example:

```cmc
grunk apples = 3

binga apples > 2
    oga "That is a lot of apples!"
wonga
    oga "Just a few apples."
unga
```

Tips:
- Checks use `==`, `!=`, `<`, `>`, `<=`, `>=`.
- You can stick checks together with `and`, `or`, and `not`.

See also: `wonga`, `unga`, `gronk`, `nork`, `and`, `or`, `not`

### `wonga` (english: `else`)

**What to do when the binga check was not gronk.**

```cmc
binga <check>
    <things>
wonga
    <other things>
unga
wonga binga <check2>   ugg check something else
```

`wonga` lives inside a `binga`. The things after it happen only when the `binga` check was `nork` (false).

`wonga binga` checks something else before giving up.

Example:

```cmc
grunk door = "open"

binga door == "open"
    oga "Walk in!"
wonga
    oga "Knock first."
unga
```

Tips:
- `wonga` never has its own `unga`; the `unga` belongs to the whole `binga`.

See also: `binga`, `unga`

### `unga` (english: `end`, `done`, `finish`)

**Close a block of code.**

```cmc
unga
```

`unga` means 'that block is done'. Every `binga`, `booga`, `zug`, `zoop`, and `clump` block ends with `unga`.

Think of it like a cave door closing behind the things you put inside.

Example:

```cmc
booga 3
    oga "unga!"
unga
```

Tips:
- Blocks can live inside blocks. Each one needs its own `unga`.
- An extra `unga` with nothing open makes a friendly OOGA! error.

See also: `binga`, `booga`, `zug`, `zoop`, `clump`

## Logic

### `and`

**Gronk only when both sides are gronk.**

```cmc
<check> and <check>
```

`and` joins two checks. The answer is `gronk` only when both sides are gronk.

Example:

```cmc
grunk age = 7

binga age > 4 and age < 10
    oga "You are a big kid!"
unga
```

Tips:
- `and` stops early if the left side is nork.

See also: `or`, `not`, `binga`

### `or`

**Gronk when at least one side is gronk.**

```cmc
<check> or <check>
```

`or` joins two checks. The answer is `gronk` when either side is gronk.

Example:

```cmc
grunk day = "sunday"

binga day == "saturday" or day == "sunday"
    oga "No school!"
unga
```

Tips:
- `or` stops early if the left side is gronk.

See also: `and`, `not`, `binga`

### `not`

**Flip gronk to nork and nork to gronk.**

```cmc
not <check>
```

`not` flips a check. `not gronk` is `nork`.

Example:

```cmc
grunk hungry = nork

binga not hungry
    oga "Then let us go play!"
unga
```

Tips:
- Double flip: `not not gronk` is `gronk` again.

See also: `and`, `or`, `gronk`, `nork`

## Loops

### `booga` (english: `repeat`)

**Do something again and again, a set number of times.**

```cmc
booga <count>
    <things to do>
unga
```

`booga 5` does the things inside five times.

The count must be a number. `booga 0` does nothing.

Inside the loop there is a magic box named `lap` that remembers which lap you are on: 1 for the first lap, 2 for the second, and so on.

Example:

```cmc
booga 3
    oga "BOOGA!"
unga

oga "done"
```

Tips:
- Use the magic box `lap` to do different things each time: `oga "lap", lap`.
- For 'keep going until something changes' use `zug`.

See also: `zug`, `zoop`, `unga`

### `zug` (english: `while`)

**Keep doing something while a check stays gronk.**

```cmc
zug <check>
    <things to do>
unga
```

`zug` checks before every lap. If the check is `gronk`, it does the things inside and checks again.

When the check becomes `nork`, the loop stops.

Example:

```cmc
grunk fuel = 3

zug fuel > 0
    oga "Zoom! fuel left:", fuel
    fuel = fuel - 1
unga

oga "Out of fuel."
```

Tips:
- If the check never becomes nork you get the 'ran for a very long time' error.
- Change a box inside the loop so the check can finish.

See also: `booga`, `zoop`, `unga`

### `zoop` (english: `for`)

**Do something once for each thing in a pile or text.**

```cmc
zoop <name> in <pile or text>
    <things to do>
unga
```

`zoop` walks through a pile (or the letters of a text) one item at a time. The box you name holds the item for that lap.

Example:

```cmc
grunk pets = snorf("dog", "cat", "rock")

zoop pet in pets
    oga "I love my", pet
unga
```

Tips:
- For text, each lap gives you one letter.
- The loop box is a normal box: you can oga it, do math with it, and so on.

See also: `snorf`, `booga`, `zug`

### `in`

**Am I inside this pile / text?  Also used by zoop.**

```cmc
<thing> in <pile or text>
zoop <name> in <pile> ... unga
```

`in` asks: does this pile contain that thing? For text it asks about smaller pieces of text.

`in` is also the little word inside every `zoop` loop.

Example:

```cmc
grunk pets = snorf("dog", "cat")

binga "cat" in pets
    oga "There is a cat in the pile!"
unga
```

Tips:
- `not in` works too: `binga "fish" not in pets`

See also: `zoop`, `snorf`

## Clumps

### `clump` (english: `fn`, `function`, `def`)

**Make your own word that does a bunch of things.**

```cmc
clump <name>(<boxes>)
    <things to do>
unga
clump <name>()          ugg no boxes
clump <name>(a, b, c)   ugg many boxes
```

A `clump` is a bunch of code with a name. Run it any time by writing its name with parentheses.

The names in parentheses become boxes filled with whatever you pass in. Use `ork` to give an answer back.

Example:

```cmc
clump yell(word)
    oga shout(word), "!!!"
unga

yell("hello")
yell("booga")
```

Tips:
- Make a clump before you use it (the computer reads top to bottom).
- A clump with an `ork` gives something back: `grunk x = double(4)`
- A clump that uses itself is called recursion -- powerful and fun.

See also: `ork`, `unga`, `grunk`

### `ork` (english: `return`, `give`)

**Give an answer back from a clump.**

```cmc
ork <thing>
```

`ork` hands a value back to whoever called the clump and stops the clump right there.

A clump without `ork` gives back `plop` (nothing).

Example:

```cmc
clump double(n)
    ork n * 2
unga

oga double(21)
```

Tips:
- `ork` with nothing after it gives back `plop`.
- `ork` outside of a clump is an OOGA! error.

See also: `clump`, `plop`

## Values

### `gronk` (english: `true`)

**The truth value for YES.**

```cmc
gronk
```

`gronk` means yes/on/true. It is what checks give back when they are happy.

Example:

```cmc
grunk light_on = gronk
oga light_on   ugg prints gronk
```

Tips:
- `oga gronk` prints the word gronk.

See also: `nork`, `binga`

### `nork` (english: `false`)

**The truth value for NO.**

```cmc
nork
```

`nork` means no/off/false.

Example:

```cmc
grunk light_on = nork
binga not light_on
    oga "It is dark in here."
unga
```

Tips:
- Numbers can be truthy too: 0 is like nork, other numbers are like gronk.

See also: `gronk`, `binga`

### `plop` (english: `nothing`, `none`)

**Nothing at all. An empty spot.**

```cmc
plop
```

`plop` is the value of nothing. A clump without `ork` gives back `plop`. A fresh empty box holds `plop`.

There is also the pile helper `plop(pile, thing)` -- see its page.

Example:

```cmc
grunk treasure = plop
oga treasure   ugg prints: plop
```

Tips:
- `plop` is a fine way to say 'no answer yet'.

See also: `ork`, `grunk`, `plop_call`

### `pi`

**The circle number, about 3.14159.**

```cmc
pi
```

`pi` is the magic number of circles. It is already there, ready to use.

Example:

```cmc
oga "A circle with radius 2 has around area", pi * 2 * 2
```

Tips:
- It is really 3.141592653589793 -- CMC remembers the rest.

See also: `root`, `round`

## Piles

### `snorf`

**Make a pile of things.**

```cmc
snorf(<thing>, <thing>, ...)
```

`snorf` builds a *pile* (a list). Piles keep things in order and can grow and shrink.

A pile can hold any mix of things: numbers, text, even other piles.

Example:

```cmc
grunk pets = snorf("dog", "cat", "rock")
oga pets
oga nom(pets)   ugg how many
```

Tips:
- `snorf()` makes an empty pile.
- Spot numbers start at 0: the first thing is `pets[0]`.

See also: `nom`, `skoop`, `yoink`, `plop_call`, `zoop`

### `plop`

**Plop a thing onto the end of a pile.**

```cmc
plop(<pile>, <thing>)
```

`plop(pile, thing)` adds a thing to the end of a pile.

The same word `plop` by itself means *nothing* -- this is the doing version.

Example:

```cmc
grunk bag = snorf()
plop(bag, "apple")
plop(bag, "rock")
oga bag
```

Tips:
- A pile grows as big as you like. Keep plopping!

See also: `snorf`, `yoink`, `nom`

### `nom`

**How many things? (length of a pile or text)**

```cmc
nom(<pile or text>)
```

`nom` counts. For a pile it counts the things. For text it counts the letters.

Example:

```cmc
grunk word = "cave"
oga nom(word)      ugg 4

grunk xs = snorf(1, 2, 3)
oga nom(xs)        ugg 3
```

Tips:
- Empty pile: nom is 0. Empty text "": nom is 0.

See also: `snorf`, `skoop`, `goop`

### `skoop`

**Scoop out one thing from a pile or text by its spot number.**

```cmc
skoop(<pile or text>, <spot>)
```

`skoop(pile, 2)` gives you the thing at spot 2. Spots start at 0.

`pile[2]` does exactly the same thing. Negative spots count from the end: `-1` is the last thing.

Example:

```cmc
grunk pets = snorf("dog", "cat", "fish")
oga skoop(pets, 0)   ugg dog
oga pets[2]          ugg fish
oga pets[-1]         ugg fish too
```

Tips:
- Asking for a spot that is not there gives an OOGA! error.

See also: `snorf`, `nom`, `yoink`

### `yoink`

**Take the last thing off a pile.**

```cmc
yoink(<pile>)
```

`yoink(pile)` removes the last thing and hands it back to you, like pulling the top rock off a stack.

Example:

```cmc
grunk stack = snorf(1, 2, 3)
grunk top = yoink(stack)
oga "I yoinked", top
oga stack   ugg [1, 2]
```

Tips:
- Yoinking an empty pile gives a friendly OOGA! error.

See also: `plop_call`, `snorf`, `nom`

## Text

### `goop`

**Squish anything into text.**

```cmc
goop(<thing>)
```

`goop` turns a number, pile, truth, or plop into text (a string).

This is how you stick numbers and words together: `"I am " + goop(7)`.

Example:

```cmc
grunk age = 7
oga "I am " + goop(age) + " years old"
```

Tips:
- `oga` already goops things for you. `goop` is for joining.

See also: `oga`, `shout`, `whisper`, `split`, `join`

### `shout`

**Make text ALL CAPS.**

```cmc
shout(<text>)
```

`shout(text)` turns every letter big: `shout("hi")` is `"HI"`.

Example:

```cmc
oga shout("hello cave")
```

Tips:
- Alien caves: `shout` only knows the letters it knows.

See also: `whisper`, `goop`

### `whisper`

**Make text all small letters.**

```cmc
whisper(<text>)
```

`whisper(text)` turns every letter small.

Example:

```cmc
oga whisper("SHHH VERY LOUD")
```

Tips:
- Shout then whisper to make text calm again.

See also: `shout`, `goop`

### `flip`

**Turn text backwards.**

```cmc
flip(<text>)
```

`flip(text)` turns the text around: `flip("cave")` is `"evac"`.

Example:

```cmc
oga flip("booga")   ugg agoob
```

Tips:
- flip works on any text, even one letter.

See also: `goop`, `nom`

### `find`

**Where does a smaller text start inside a bigger text?**

```cmc
find(<text>, <piece>)
```

`find(text, piece)` gives the spot number where `piece` starts inside `text`, or `-1` when it is not there at all.

Example:

```cmc
grunk word = "caveman"
oga find(word, "man")   ugg 4
oga find(word, "z")     ugg -1
```

Tips:
- Spots start at 0, so the first letter is spot 0.

See also: `skoop`, `split`, `nom`

### `split`

**Chop text into a pile of pieces.**

```cmc
split(<text>, <separator>)
```

`split(text, ",")` cuts text wherever the separator is and gives back a pile of the pieces.

Example:

```cmc
grunk words = split("rock,paper,scissors", ",")
oga words
oga nom(words)
```

Tips:
- Split on a space to get words: split(sentence, " ")

See also: `join`, `snorf`, `nom`

### `join`

**Stick a pile of things together into one text.**

```cmc
join(<pile>, <separator>)
```

`join(pile, "-")` turns a pile into one text, putting the separator between the items.

Example:

```cmc
grunk xs = snorf("a", "b", "c")
oga join(xs, " + ")   ugg a + b + c
```

Tips:
- Numbers get gooped automatically.

See also: `split`, `snorf`, `goop`

### `what`

**Ask what kind of thing something is.**

```cmc
what(<thing>)
```

`what` gives back a word: `"number"`, `"text"`, `"truth"`, `"pile"`, or `"plop"`.

Example:

```cmc
oga what(3)        ugg number
oga what("hi")     ugg text
oga what(snorf())  ugg pile
```

Tips:
- Telling apart numbers and text is a very useful trick.

See also: `goop`, `nom`

## Numbers

### `munga`

**Magic random number between two numbers.**

```cmc
munga(<low>, <high>)
```

`munga(1, 6)` gives a random whole number from 1 to 6 -- both ends included. Perfect for dice and games.

`munga(1, 1)` is always 1, which is handy for testing.

Example:

```cmc
grunk dice = munga(1, 6)
oga "You rolled a", dice
```

Tips:
- Run the program again for a different number.

See also: `round`, `small`, `big`, `skrib`

### `numba`

**Turn text into a number.**

```cmc
numba(<text>)
```

`blorp` always gives back text, even when someone types `7`. `numba("7")` turns that text into a real number so math works.

It also cleans up extra spaces people type.

Example:

```cmc
grunk answer = blorp "Pick a number!"
grunk n = numba(answer)
oga n, "plus one is", n + 1
```

Tips:
- If the text is not a number, numba says OOGA! with a kind hint.
- A number passed to numba comes back unchanged.

See also: `blorp`, `goop`, `what`, `round`

### `round`

**Make a decimal into the closest whole number.**

```cmc
round(<number>)
```

`round(2.6)` is 3. `round(2.4)` is 2.

Example:

```cmc
oga round(2.6)
oga round(2.4)
```

Tips:
- `flat` always goes down, `roof` always goes up.

See also: `flat`, `roof`

### `flat`

**Smash a decimal down to the whole number below it.**

```cmc
flat(<number>)
```

`flat(2.9)` is 2. It always goes down, like water.

Example:

```cmc
oga flat(2.9)
oga flat(2.0)
```

Tips:
- flat is also called 'floor' in other languages.

See also: `round`, `roof`

### `roof`

**Lift a decimal up to the whole number above it.**

```cmc
roof(<number>)
```

`roof(2.1)` is 3. It always goes up, like a roof.

Example:

```cmc
oga roof(2.1)
oga roof(2.0)
```

Tips:
- roof is also called 'ceil' in other languages.

See also: `round`, `flat`

### `abs`

**How far from zero? (make negative numbers positive)**

```cmc
abs(<number>)
```

`abs(-7)` is 7. `abs(7)` is 7. It measures distance from zero.

Example:

```cmc
oga abs(-7)
oga abs(3 - 10)
```

Tips:
- Great for finding how far apart two things are.

See also: `small`, `big`

### `small`

**The smaller of two numbers.**

```cmc
small(<a>, <b>)
```

`small(3, 9)` is 3.

Example:

```cmc
oga small(3, 9)
```

Tips:
- The opposite twin is `big`.

See also: `big`, `abs`

### `big`

**The bigger of two numbers.**

```cmc
big(<a>, <b>)
```

`big(3, 9)` is 9.

Example:

```cmc
oga big(3, 9)
```

Tips:
- Chain them: big(big(1, 5), 3) is 5.

See also: `small`, `abs`

### `root`

**The square root: what number times itself makes this?**

```cmc
root(<number>)
```

`root(9)` is 3. Negative numbers have no normal root, so CMC says OOGA!

Example:

```cmc
oga root(9)
oga root(2)
```

Tips:
- root(2) is a wiggly decimal -- `round` it if you like.

See also: `pow`, `round`, `pi`

### `pow`

**A number times itself many times.**

```cmc
pow(<number>, <times>)
```

`pow(2, 3)` is 2 * 2 * 2 = 8.

Example:

```cmc
oga pow(2, 3)
oga pow(10, 2)
```

Tips:
- pow(x, 2) is x squared.

See also: `root`

## Drawing

### `skrib` (english: `draw`)

**Draw on the picture window (OGABOOGA CODER's Draw tab).**

```cmc
skrib circle <x>, <y>, <r>
skrib dot <x>, <y>, <r>      ugg filled circle
skrib line <x1>, <y1>, <x2>, <y2>
skrib box <x>, <y>, <w>, <h> ugg outline rectangle
skrib blob <x>, <y>, <w>, <h> ugg filled rectangle
skrib write "words", <x>, <y>
skrib color "red"            ugg names or "#ff8800"
skrib size <w>, <h>          ugg picture size
skrib clear                  ugg wipe the picture
```

`skrib` (caveman for *scribble*) draws on the picture. The spot `0, 0` is the top-left corner. `x` goes right, `y` goes down.

Colors can be names like `"red"`, `"blue"`, `"gold"` or hex colors like `"#ff8800"`.

Example:

```cmc
skrib size 400, 400
skrib color "orange"
skrib circle 200, 200, 100
skrib color "black"
skrib dot 200, 200, 10
skrib write "SUN!", 175, 320
```

Tips:
- The picture window opens by itself the first time you draw.
- Use `wait(ms)` between drawings to make a simple animation.
- In the terminal build, close the picture window when the program is done.

See also: `wait`, `booga`, `munga`

### `wait`

**Pause the program for some milliseconds.**

```cmc
wait(<milliseconds>)
```

`wait(1000)` pauses for one second. 1000 milliseconds = 1 second.

Waiting between drawings makes animation: draw, wait, clear, draw again.

Example:

```cmc
skrib color "red"
skrib dot 50, 50, 10
wait(500)
skrib clear
skrib color "blue"
skrib dot 100, 50, 10
```

Tips:
- The Stop button still works while waiting.

See also: `skrib`, `booga`

## Notes

### `ugg` (english: `comment`)

**A note for humans. The computer ignores it.**

```cmc
ugg <anything at all>
```

`ugg` starts a note (comment). Everything after it on that line is ignored by the computer.

Notes are how you explain your code to future-you.

Example:

```cmc
ugg this whole line is a note
oga "hi"   ugg this note comes after code
```

Tips:
- Notes do not slow the program down at all.
