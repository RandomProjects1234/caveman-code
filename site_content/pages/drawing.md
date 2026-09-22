# Drawing with skrib

`skrib` (caveman for *scribble*) draws in the **Draw tab** of OGABOOGA CODER,
or in a picture window when you run from the terminal.

## The picture

The spot `0, 0` is the **top-left** corner. `x` goes right, `y` goes down.
The default picture is 400 by 400.

```cmc
skrib size 400, 400
```

## Commands

| Write this | It draws |
| --- | --- |
| `skrib color "red"` | pick a color (names or `"#ff8800"`) |
| `skrib dot x, y, r` | a filled circle (a dot) |
| `skrib circle x, y, r` | an outline circle |
| `skrib line x1, y1, x2, y2` | a line |
| `skrib box x, y, w, h` | an outline rectangle |
| `skrib blob x, y, w, h` | a filled rectangle |
| `skrib write "words", x, y` | text on the picture |
| `skrib clear` | wipe the picture |

Color names include the usual ones: `"red"`, `"orange"`, `"gold"`,
`"green"`, `"blue"`, `"purple"`, `"brown"`, `"black"`, `"white"`,
`"skyblue"`, and any hex color like `"#ff8800"`.

## A sun

```cmc
skrib size 400, 400
skrib color "gold"
skrib dot 200, 200, 60
skrib color "orange"
booga 12
    grunk angle_x = 200 + 90 * small(1, lap % 2)
    skrib line 200, 200, angle_x, 20 + lap * 30
unga
```

## A house

```cmc
skrib size 400, 400
skrib color "skyblue"
skrib blob 0, 0, 400, 260
skrib color "green"
skrib blob 0, 260, 400, 140
skrib color "brown"
skrib blob 140, 150, 120, 130
skrib color "red"
skrib line 120, 150, 200, 80
skrib line 200, 80, 280, 150
skrib color "gold"
skrib box 180, 190, 40, 40
skrib color "black"
skrib write "home", 170, 300
oga "Drew a home!"
```

## Moving things: wait

`wait(milliseconds)` pauses the program. 1000 milliseconds is one second.
Draw, wait, clear, draw again -- that is animation:

```cmc
skrib size 300, 300

booga 20
    skrib clear
    skrib color "purple"
    skrib dot 30 + lap * 12, 150, 15
    wait(80)
unga

oga "The dot ran across the screen!"
```

The **STOP** button still works while `wait` is running.

## Drawing with loops and math

```cmc
skrib size 400, 400
skrib color "teal"

booga 30
    skrib circle 200, 200, lap * 6
unga
```

```cmc
skrib size 400, 400
skrib color "purple"

grunk x = 200
grunk y = 20
booga 30
    grunk nx = x + 4 * lap
    grunk ny = y + 12
    skrib line x, y, nx, ny
    x = nx
    y = ny
unga
```

## Saving your picture

OGABOOGA CODER does not save pictures yet -- use a screenshot, or write a
program that draws the same thing every time (that is why code is better than
a paint program!).

## Drawing in the terminal

`cmc run picture.cmc` opens a window with the drawing. The window stays open
after the program ends, so you can look at the art. Close it when you are
done.

If the drawing does nothing, you are probably running in a place with no
windows (like a server). The program still runs, it just cannot show the
picture.

Next: [Errors](errors.md) -- what to do when OOGA happens.