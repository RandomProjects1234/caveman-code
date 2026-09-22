# Your first program, explained

Here is a complete little program. Read the notes under each part.

```cmc
ugg 1. A note. The computer skips "ugg" lines.
oga "Hello cave friend!"

ugg 2. A box that holds text.
grunk name = "Ugg"

ugg 3. A box that holds a number.
grunk rocks = 3

ugg 4. gluing things with +
oga "My name is " + name + " and I have " + goop(rocks) + " rocks"

ugg 5. a loop that counts laps
booga 3
    oga "bang! lap", lap
unga

ugg 6. a decision
binga rocks >= 3
    oga "That is a rock collection!"
wonga
    oga "Need more rocks."
unga
```

Output:

```text
Hello cave friend!
My name is Ugg and I have 3 rocks
bang! lap 1
bang! lap 2
bang! lap 3
That is a rock collection!
```

## Line by line

### `ugg ...`

`ugg` starts a note (also called a comment). Notes are for humans. You can put
one at the end of a line too:

```cmc
oga "hi"   ugg this note is ignored
```

### `oga "Hello cave friend!"`

`oga` prints. Text must be inside quotes. Without quotes, CMC looks for a box
with that name:

```cmc
oga "name"    ugg prints the word: name
oga name      ugg prints what is inside the box: Ugg
```

### `grunk name = "Ugg"`

`grunk` makes a new box called `name` and puts the text `Ugg` in it.

### `rocks` and `goop`

`goop(rocks)` turns the number 3 into the text `"3"` so it can be glued to
other text with `+`. Gluing text and numbers directly gives a friendly error
that reminds you to use `goop`.

### `booga 3 ... unga`

Everything between `booga 3` and `unga` happens three times. The magic box
`lap` counts 1, 2, 3.

### `binga ... wonga ... unga`

`binga` means "if". `wonga` means "else". The check `rocks >= 3` is either
`gronk` (yes) or `nork` (no).

## Try these changes

1. Change the name to your own.
2. Change `booga 3` to `booga 10`.
3. Change `rocks >= 3` to `rocks >= 100`.
4. Add a fourth bang with `booga rocks`.

## Ideas to try next

- [Tutorial](tutorial.md)
- [Loops](loops.md) -- `booga`, `zug`, and `zoop`
- [Piles (lists)](lists.md)
- [Drawing](drawing.md)