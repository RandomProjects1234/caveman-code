# Troubleshooting

## The program does not start

### Double-clicking the .bat does nothing

The window may open and close too fast to read. Run it from a terminal:

```text
powershell -File run_ogabooga.bat
```

The error will stay on screen. Usually it is Python missing from PATH.

### `python is not recognized`

Python is not installed, or not on the PATH. Reinstall Python from
python.org and tick **Add Python to PATH**.

### `ModuleNotFoundError: No module named 'cmc'`

You are running from the wrong folder, or `src` is not on the import path.

```text
# from the repo folder
set PYTHONPATH=src          (Windows cmd)
$env:PYTHONPATH = "src"     (PowerShell)
export PYTHONPATH=src       (macOS/Linux)
```

Or install it: `pip install .`

## The window is open but weird

### The question window from blorp never appears

It appears when the program actually asks. If your program never reaches a
`blorp`, nothing pops up. Check the output panel for where it stopped.

### The picture is blank

- Make sure you pressed RUN while on the Text tab (or that the blocks call
  `skrib`).
- CMC wipes the picture at the start of each run -- if your program draws and
  then clears, you see white.
- Check the coordinates: `0, 0` is the top-left. Drawing at `900, 900` on a
  400x400 picture is off the edge.

### My text is not colored

Colors appear in the Text and Blocks code panels. If they vanish, the word may
be misspelled -- misspelled words are not keywords, so they stay plain.

## The program runs but the answer is wrong

### It never stops

CMC stops runaway loops after five million steps and shows the OOGA message
about a loop never stopping. Look at your `zug` check: is the box it checks
changed inside the loop?

### It stops immediately

Read the red OOGA line. The line number points at the problem. Nine times out
of ten it is one of:

- a missing `unga`
- a box used before `grunk`
- `oga` missing in front of an expression

### Numbers come out as text

`blorp` answers are always text. Wrap them in `numba(...)`.

### Weird decimals (like 194.2222222222)

That is a real decimal. Use `round`, `flat`, or `roof` to tidy it for people.
If you expected a whole number and got a decimal, look for a division that did
not fit evenly -- or parentheses in the wrong place.

## Blocks trouble

### A block will not drop into a slot

Only **value** blocks (number, text, box value, random) fit in slots. If you
want to compute something (like `x + 1`), click the slot and type it.

### Load from Text says "cannot become a block yet"

Some CMC ideas are text-only for now (for example mixing `blorp` inside a
math expression). The message tells you the line. Use the Text tab for that
program, or simplify the line.

### Undo went too far

Ctrl+Z undoes one change at a time, up to about eighty changes back. There is
no redo for blocks yet -- keep pressing RUN and experimenting; you can always
load the previous save.

## Terminal troubles

### `cmc run` opens a picture window and it will not go away

Close the picture window. CMC keeps it open so you can admire the drawing.

### Text looks garbled in the console

The terminal is not in UTF-8. CMC sets UTF-8 when it can. On very old
consoles, set `chcp 65001` first.

## Still stuck?

- Read the [Errors page](errors.md) -- it lists every message with a fix.
- Ask the [Dictionary](words.html) what a word expects.
- Make the smallest program you can that shows the problem. Bugs usually
  vanish while you do this, which is a kind of magic.

Next: [Examples](examples.md).