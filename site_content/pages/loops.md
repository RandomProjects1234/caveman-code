# Loops

CMC has three loop words, each for a different job.

## booga -- do it a set number of times

```cmc
booga 3
    oga "bang rock!"
unga
```

Inside the loop, the magic box `lap` holds 1 on the first lap, 2 on the
second, and so on. It is perfect for counting:

```cmc
booga 5
    oga "rock number", lap
unga
```

The count can be any expression:

```cmc
grunk rocks = 3
booga rocks * 2
    oga "bang!"
unga
```

`booga 0` does nothing. A decimal like `booga 2.5` is an error with a hint to
use `round` or `flat`.

Careful with nested `booga` loops: there is only one `lap` box, so the inner
loop uses it up. If you need both counts, save the outer one in your own box
first.

## zug -- while something stays true

```cmc
grunk fuel = 3

zug fuel > 0
    oga "zoom! fuel left:", fuel
    fuel = fuel - 1
unga

oga "out of fuel"
```

`zug` checks **before** every lap. When the check turns `nork`, the loop
stops. Notice `fuel = fuel - 1` inside: something must change, or the check
will stay gronk forever.

If a loop never stops, CMC stops it for you after about five million steps and
says:

```text
OOGA! This program ran for a very long time. Maybe a loop never stops?
Hint: check your 'zug' loops. Make sure the box they check gets changed inside the loop.
```

That is much friendlier than a frozen window.

## zoop -- once for each thing

```cmc
grunk pets = snorf("dog", "cat", "rock")

zoop pet in pets
    oga "I love my", pet
unga
```

You pick the loop box name (`pet` above). It holds one thing each lap. This
works for text too, giving one letter at a time:

```cmc
zoop letter in "cave"
    oga letter
unga
```

`zoop` over something that is not a pile or text is an error with a hint.

## Which loop should I use?

| Job | Use |
| --- | --- |
| I know how many times | `booga` |
| Until something changes | `zug` |
| Once per thing in a pile | `zoop` |

## Combining loops and choices

```cmc
grunk numbers = snorf(4, 8, 15, 16, 23, 42)
grunk big_ones = 0

zoop n in numbers
    binga n > 10
        big_ones = big_ones + 1
    unga
unga

oga "big numbers:", big_ones
```

```text
big numbers: 4
```

## One more trick: building text in a loop

```cmc
grunk stars = ""
booga 5
    stars = stars + "*"
unga
oga stars
```

```text
*****
```

Next: [Clumps](functions.md) -- make your own words.