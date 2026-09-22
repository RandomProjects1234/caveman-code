# Numbers, math, and random magic

## The signs

| Sign | Means | Example | Answer |
| --- | --- | --- | --- |
| `+` | plus | `2 + 3` | 5 |
| `-` | minus | `10 - 4` | 6 |
| `*` | times | `6 * 7` | 42 |
| `/` | share | `10 / 4` | 2.5 |
| `%` | remainder | `10 % 3` | 1 |

Times happens before plus, just like in school. Use parentheses when you want
to be the boss:

```cmc
oga 2 + 3 * 4        ugg 14
oga (2 + 3) * 4      ugg 20
```

Dividing by zero does not crash -- it gives a friendly error:

```text
OOGA! Cannot share by zero!
Hint: check the number is not 0 before dividing.
```

Whole numbers stay whole when they can:

```cmc
oga 4 / 2      ugg 2 (not 2.0 -- CMC keeps it tidy)
oga 5 / 2      ugg 2.5
```

## The math helpers

```cmc
oga round(2.5)     ugg 3
oga flat(2.9)      ugg 2    (always down)
oga roof(2.1)      ugg 3    (always up)
oga abs(0 - 7)     ugg 7    (how far from zero)
oga small(3, 9)    ugg 3
oga big(3, 9)      ugg 9
oga root(81)       ugg 9
oga root(2)        ugg 1.4142135623730951
oga pow(2, 5)      ugg 32
oga pi             ugg 3.141592653589793
```

`root` of a negative number is an error with a hint to use `abs` first.

## Random magic: munga

`munga(low, high)` gives a random whole number, and **both ends are
possible**:

```cmc
grunk dice = munga(1, 6)
oga "You rolled a", dice
```

```cmc
grunk coin = munga(0, 1)
binga coin == 0
    oga "heads"
wonga
    oga "tails"
unga
```

Picking a random thing from a pile:

```cmc
grunk snacks = snorf("berry", "fish", "mystery meat")
grunk pick = snacks[munga(0, nom(snacks) - 1)]
oga "tonight we eat:", pick
```

`munga(1, 1)` is always 1. That is handy for testing: your program is random
everywhere except where you say otherwise.

## Text to number

`blorp` answers are text, so use `numba` before doing math:

```cmc
grunk answer = blorp "How many rocks?"
grunk count = numba(answer)
oga count * 2, "is twice that many"
```

If the answer was not a number, `numba` says:

```text
OOGA! numba cannot turn "banana" into a number.
Hint: answers that are numbers look like "42" or "3.5".
```

## Number to text

Use `goop` when gluing a number into text:

```cmc
oga "half of 9 is " + goop(9 / 2)
```

## A tiny calculator

```cmc
clump calc(a, op, b)
    binga op == "+"
        ork a + b
    wonga binga op == "-"
        ork a - b
    wonga binga op == "*"
        ork a * b
    wonga binga op == "/"
        ork a / b
    wonga
        ork plop
    unga
unga

oga calc(6, "*", 7)      ugg 42
```

Next: [Asking questions](input.md).