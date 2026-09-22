# Choices (binga / wonga)

`binga` runs some code only when a check is `gronk` (yes).

```cmc
grunk rocks = 5

binga rocks > 3
    oga "That is a rock collection!"
unga
```

`wonga` means "otherwise":

```cmc
binga rocks > 3
    oga "Lots of rocks!"
wonga
    oga "Not many rocks."
unga
```

Only **one** `unga` is needed. It closes the whole `binga`, whether or not the
`wonga` part ran.

## Else if, caveman style

Chain checks with `wonga binga`:

```cmc
binga score >= 90
    oga "A"
wonga binga score >= 80
    oga "B"
wonga binga score >= 70
    oga "C"
wonga
    oga "keep trying"
unga
```

CMC reads them in order and stops at the first `gronk`.

## The checks

| Check | Means |
| --- | --- |
| `a == b` | a is equal to b |
| `a != b` | a is not equal to b |
| `a < b` | a is smaller than b |
| `a <= b` | a is smaller or equal |
| `a > b` | a is bigger |
| `a >= b` | a is bigger or equal |
| `x in pile` | x is inside a pile or inside text |
| `x not in pile` | x is not inside |

Join checks with the logic words:

```cmc
binga hungry and rocks > 0
    oga "eat a rock"
unga

binga name == "Ugg" or name == "Oona"
    oga "family!"
unga

binga not tired
    oga "let us play!"
unga
```

`and` needs both sides to be gronk; `or` needs just one; `not` flips.
They stop early when the answer is already known.

## What counts as yes?

See [Values and types](values-and-types.md), but the short version:
`0`, `nork`, `""`, an empty pile, and `plop` are all nork. Everything else is
gronk. So this works:

```cmc
binga rocks
    oga "there is at least one rock"
unga
```

## One compare at a time

This is **not** allowed:

```cmc
binga 1 < x < 10     ugg OOGA!
```

Write it with `and`:

```cmc
binga 1 < x and x < 10
    oga "one digit"
unga
```

## Nesting

Blocks can live inside blocks:

```cmc
binga door == "open"
    binga light == "on"
        oga "walk on in"
    wonga
        oga "it is dark in there"
    unga
wonga
    oga "knock first"
unga
```

Every `binga` gets its own `unga`. Line up the `unga` under its `binga` and
your code will look like a tidy cave shelf.

Next: [Loops](loops.md).