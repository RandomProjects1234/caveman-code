# Values and types

Everything in CMC is one of five kinds of thing. `what(x)` tells you which:

```cmc
oga what(3)          ugg number
oga what("hi")       ugg text
oga what(gronk)      ugg truth
oga what(plop)       ugg plop
oga what(snorf())    ugg pile
```

## number

Whole numbers and decimals. Everything you expect works:

```cmc
oga 7, 3.5, 0 - 2
oga 2 + 3 * 4        ugg 14, times goes first
oga (2 + 3) * 4      ugg 20, parentheses are your friend
```

Division gives a whole number when it fits:

```cmc
oga 4 / 2            ugg 2
oga 5 / 2            ugg 2.5
```

Numbers print naturally: `2.0` shows as `2`.

## text (goop)

Text is letters, numbers, and symbols inside quotes:

```cmc
grunk name = "Ugg"
grunk empty = ""
oga name + " the cave kid"
oga nom(name)                  ugg 3, how many letters
oga name[0]                    ugg U
oga shout(name), whisper("HEY") ugg UGG hey
```

Text cannot be glued directly to numbers with `+`. Use `goop`:

```cmc
oga "I am " + goop(7) + " years old"
```

## truth (gronk / nork)

Checks give back `gronk` (yes) or `nork` (no):

```cmc
grunk hungry = gronk
oga hungry                ugg gronk
oga not hungry            ugg nork
oga 3 > 5                 ugg nork
```

`and`, `or`, and `not` work the way you expect, and stop early when they can:

```cmc
binga hungry and rocks > 0
    oga "eat a rock"
unga
```

## plop

`plop` means *nothing at all*:

- A clump without `ork` gives back `plop`
- A fresh box with nothing in it holds `plop`
- `plop` prints as the word `plop`

```cmc
clump shout_if_loud(word)
    binga nom(word) > 4
        ork shout(word)
    unga
    ork plop
unga

oga shout_if_loud("hi")        ugg plop
oga shout_if_loud("booga")     ugg BOOGA
```

The same word has a doing version: `plop(pile, thing)` adds to a pile. Read
[plop(pile, thing)](words/plop_call.html) for that one.

## pile (snorf)

A pile is an ordered list that can grow and shrink:

```cmc
grunk bag = snorf("apple", "rock")
plop(bag, "banana")
oga bag            ugg [apple, rock, banana]
oga nom(bag)       ugg 3
```

Piles can hold any mix of kinds, even other piles:

```cmc
grunk treasure = snorf(1, "gold", gronk, snorf("deep", "pile"))
oga treasure
```

```text
[1, gold, gronk, [deep, pile]]
```

## What counts as gronk in a check?

| Value | In a check it means |
| --- | --- |
| `gronk` | yes |
| `nork` | no |
| `0` | no |
| any other number | yes |
| `""` (empty text) | no |
| other text | yes |
| empty pile `snorf()` | no |
| other pile | yes |
| `plop` | no |

This means `binga rocks` works and means "does rocks hold a non-zero number?"

## Comparing unlike things

`==` and `!=` never crash: different kinds are simply not equal.

```cmc
oga 1 == "1"        ugg nork
oga gronk == 1      ugg nork
oga plop == plop    ugg gronk
```

But `<` and `>` need two numbers or two texts:

```cmc
oga "apple" < "banana"    ugg gronk (alphabetical)
oga 3 < "four"            ugg OOGA! with a hint
```

## Making one kind out of another

| From | To | Use |
| --- | --- | --- |
| number | text | `goop(7)` |
| text | number | `numba("42")` |
| anything | text | `goop(x)` (piles print with brackets) |
| pile of things | text | `join(pile, ", ")` |
| text | pile | `split("a,b", ",")` |