# The style guide (easy to learn, hard to master)

Anyone can make a program that runs. This page is about making programs that
**read** well, because code is read far more often than it is written --
mostly by future-you.

## 1. Say what the box holds

```cmc
grunk r = 3            ugg what is r?
grunk rock_count = 3   ugg ah, rocks
```

Short names are fine for tiny loops (`i`, `n`), but boxes that live a while
deserve names that read like words.

## 2. One idea per line

```cmc
ugg hard to read
grunk total = 0
zoop n in xs
    total = total + n
unga
oga total

ugg clear
grunk total = 0
zoop number in numbers
    total = total + number
unga
oga "total:", total
```

## 3. Blank lines are paragraphs

Group the setup, the work, and the answers with blank lines. A reader should
be able to skim your program and see its shape.

## 4. Notes explain WHY, not WHAT

```cmc
ugg BAD: adds one to rocks
rocks = rocks + 1

ugg GOOD: every winter, one new rock
rocks = rocks + 1
```

If a note just repeats the code, delete it. If it explains the reason, keep
it.

## 5. Small clumps

A clump should do one job and fit on one screen. If you cannot name it with a
short verb phrase (`shout_name`, `draw_house`), it is doing too much.

```cmc
clump ask_number(question)
    grunk answer = blorp question
    ork numba(answer)
unga

grunk age = ask_number("How many winters old?")
grunk rocks = ask_number("How many rocks?")
```

## 6. Prefer loops to copy-paste

If you typed the same two lines three times, you wanted a clump or a loop.
Real cave painters learned this the hard way.

## 7. Guard your mistakes

Check the inputs before trusting them:

```cmc
clump safe_divide(a, b)
    binga b == 0
        oga "cannot share by zero, giving back 0"
        ork 0
    unga
    ork a / b
unga
```

## 8. Line up your ungas

```cmc
booga 3
    binga hungry
        oga "eat"
    wonga
        oga "sleep"
    unga
unga
```

The shape of the indentation *is* the shape of the logic. When it looks wrong,
it usually is.

## 9. Keep the scary words for later

Beginners should live in `grunk`, `oga`, `booga`, `binga`, and `zoop`. Reach
for `clump`, recursion, and piles-of-piles when you actually need them. Mastery
is not using everything -- it is using the smallest thing that works.

## 10. Make it fun

The best program is the one you finish. Add a silly `oga` message, draw a
little picture, name a variable after your pet. Joy makes practice easy, and
practice makes mastery.

Next: [FAQ](faq.md) or [the examples](examples.md).