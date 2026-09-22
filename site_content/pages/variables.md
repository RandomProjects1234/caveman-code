# Boxes (variables)

A box remembers one thing so you can use it later. Making a box is called
`grunk`; changing one is a plain `=`.

```cmc
grunk name = "Ugg"
grunk rocks = 3

oga name, "has", rocks, "rocks"

rocks = rocks + 1
oga "now", rocks
```

## Making vs. changing

| Write | What happens |
| --- | --- |
| `grunk x = 5` | makes a new box `x` |
| `x = 6` | puts 6 into the box that already exists |
| `grunk x = 7` | makes the box fresh again (the old thing is forgotten) |
| `y = 6` | `OOGA!` if `y` was never made |

Why so strict? Because typos are the number one bug for new coders. If you
write `rock = rock + 1` but the box is called `rocks`, CMC tells you right
away instead of quietly making a new box.

## Boxes hold one thing

A box holds one value. You can put a text in it today and a number tomorrow,
but the value it holds right now is the value you use:

```cmc
grunk thing = "rock"
oga thing            ugg rock
thing = 5
oga thing + 1        ugg 6
```

## Boxes live in the shape of your program

- Boxes made at the top level live for the whole program.
- Boxes made inside a `clump` are that clump's own private boxes. They
  disappear when the clump finishes.
- A clump can read boxes from outside, but if it uses `grunk` on a name it
  makes its own new box with that name.

```cmc
grunk greeting = "hello"

clump say_it()
    grunk greeting = "BOOGA"    ugg this is a new, private box
    oga greeting
unga

say_it()                 ugg BOOGA
oga greeting             ugg hello (the outside box is untouched)
```

## The magic boxes

| Box | What it holds |
| --- | --- |
| `gronk` | yes (true) |
| `nork` | no (false) |
| `plop` | nothing |
| `pi` | the circle number 3.14159... |
| `lap` | the current `booga` lap, 1, 2, 3... |

You cannot use these names for boxes you make.

## Good names

- `rocks`, `total`, `player_name`, `is_awake`
- Say what is inside, not what kind of thing it is.
- `r` is fine for a quick loop, but `rock_count` is better when you come back
  tomorrow.

## A tiny example

```cmc
grunk apples = 3
grunk friends = 2

grunk each_gets = flat(apples / friends)
grunk leftover = apples % friends

oga "each friend gets", each_gets
oga "left over:", leftover
```

```text
each friend gets 1
left over: 1
```

Next: [Choices](if.md) -- making the computer decide things.