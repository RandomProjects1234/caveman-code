# Piles (lists)

A pile keeps many things in order. Make one with `snorf`:

```cmc
grunk bag = snorf("apple", "rock", "banana")
oga bag
```

```text
[apple, rock, banana]
```

An empty pile is `snorf()`.

## Growing and shrinking

```cmc
grunk bag = snorf()
plop(bag, "apple")        ugg add to the end
plop(bag, "rock")
oga nom(bag)              ugg 2, how many

grunk last = yoink(bag)   ugg take the last thing off
oga "yoinked", last       ugg yoinked rock
oga bag                   ugg [apple]
```

| Word | What it does |
| --- | --- |
| `snorf(a, b, c)` | make a pile |
| `plop(pile, thing)` | add `thing` to the end |
| `nom(pile)` | how many things |
| `yoink(pile)` | remove and give back the last thing |
| `skoop(pile, i)` | the thing at spot i |

## Spots

Spot numbers start at **0**. Negative spots count from the end:

```cmc
grunk pets = snorf("dog", "cat", "fish")

oga pets[0]        ugg dog
oga pets[1]        ugg cat
oga pets[-1]       ugg fish (the last one)
oga skoop(pets, 2) ugg fish
```

Asking for a spot that is not there is a friendly error:

```text
OOGA! There is no spot 9 in that pile (it has 3 spots).
Hint: spots start at 0, so the last spot is 2.
```

## Changing a spot

```cmc
pets[1] = "tiger"
oga pets           ugg [dog, tiger, fish]
```

You can only change a spot that already exists. To add a new thing, use
`plop`.

## Walking through a pile

```cmc
zoop pet in pets
    oga "I love my", pet
unga
```

## Is it inside?

```cmc
oga "cat" in pets            ugg gronk
oga "dragon" in pets         ugg nork
oga "dragon" not in pets     ugg gronk
```

## Piles of piles

Anything can go in a pile, including other piles:

```cmc
grunk grid = snorf(snorf(1, 2), snorf(3, 4))
oga grid[1][0]      ugg 3
```

## Useful tricks

### Add up a pile

```cmc
grunk numbers = snorf(4, 8, 15, 16, 23, 42)
grunk total = 0
zoop n in numbers
    total = total + n
unga
oga "total:", total      ugg total: 108
```

### Find the biggest

```cmc
grunk biggest = numbers[0]
zoop n in numbers
    binga n > biggest
        biggest = n
    unga
unga
oga "biggest:", biggest
```

### Turn a pile into text

```cmc
oga join(numbers, " + ")     ugg 4 + 8 + 15 + 16 + 23 + 42
```

### Turn text into a pile

```cmc
grunk words = split("rock,paper,scissors", ",")
oga words                     ugg [rock, paper, scissors]
oga words[2]                  ugg scissors
```

## A small game: hot potato

```cmc
grunk players = snorf("Ugg", "Oona", "Grog", "Zug")
grunk rock = munga(0, nom(players) - 1)
oga "The rock stops with", players[rock]
```

Next: [Text](text.md).