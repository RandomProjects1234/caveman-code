# Getting started

Five minutes from now you will have written and run a real program.

## 1. Open OGABOOGA CODER

Double-click `OGABOOGA CODER.exe`. You get a big
friendly window with three tabs:

- **Blocks (drag and drop)** -- the block workshop
- **Text (caveman code)** -- where you type CMC words
- **Draw (picture)** -- where `skrib` drawings appear

Click the **Text** tab.

## 2. Say hello

Type this:

```cmc
oga "Ooga booga! Me make program!"
```

Press **RUN** (or the F5 key). The output panel shows:

```text
Ooga booga! Me make program!
```

`oga` is how CMC talks. Everything in quotes is text.

## 3. Make a box

A *box* remembers something. `grunk` makes a box:

```cmc
grunk name = "Ugg"
grunk rocks = 3

oga name, "has", rocks, "rocks"
```

Output:

```text
Ugg has 3 rocks
```

`oga` prints each thing with a space between them.

## 4. Change the box

```cmc
rocks = rocks + 1
oga "Now", name, "has", rocks, "rocks"
```

Output:

```text
Now Ugg has 4 rocks
```

Notice: `grunk` makes a **new** box, plain `=` puts something new **into a box
that already exists**. If you use `=` on a box that was never made, CMC says
`OOGA!` and reminds you.

## 5. Ask for a turn

```cmc
grunk answer = blorp "Do you like rocks? (yes or no)"
oga "You said:", answer
```

A question window pops up. Whatever the person types comes back as text.

## 6. Repeat things

```cmc
booga 3
    oga "bang rock!"
unga
```

`booga 3` does the things inside three times. `unga` means "the block is
done". Inside the loop, the magic box `lap` tells you which lap you are on.

## 7. Decide things

```cmc
grunk rocks = 5

binga rocks > 3
    oga "That is a lot of rocks!"
wonga
    oga "Just a few rocks."
unga
```

`binga` checks; `wonga` means "otherwise". This is the same `unga` again.

## 8. Draw something

Click the **Draw** tab and run this:

```cmc
skrib size 400, 400
skrib color "orange"
skrib circle 200, 200, 120
skrib color "black"
skrib dot 200, 200, 12
oga "Sun drawn!"
```

## Where next

- [First program, explained line by line](first-program.md)
- [The 30 minute tutorial](tutorial.md)
- [The cheat sheet](cheatsheet.md)
- [All the words A-Z](words.html)