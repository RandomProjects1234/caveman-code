# Welcome to the Cave Man Code wiki

This is the home of the friendliest real programming language on the internet.
CMC was built so that a brand new coder -- even a four year old sitting on a
lap -- can say "make the computer talk" in the first minute, and still be
learning clever things years later.

## What is Cave Man Code?

Cave Man Code (CMC, say "see-em-see") is a small programming language where the
words sound like a cave person invented them:

```cmc
ugg This is a note. Ugg means "ignore this line".
oga "Ooga booga!"

grunk rocks = 3
booga rocks
    oga "bang rock", lap
unga
```

- `oga` means **say something**
- `grunk` means **make a box** (a variable)
- `booga` means **repeat**
- `unga` means **done with this block**
- `ugg` means **a note for humans**

That is already a real program with a loop in it.

## Why is it easy?

- **One idea per line.** No semicolons, no curly braces, no setup.
- **Blocks close with a word you can see:** `unga`.
- **Friendly errors.** Instead of *SyntaxError: unexpected EOF*, CMC says
  `OOGA! Line 4: A block was opened but never closed.` and tells you the fix.
- **Two ways to code.** Drag-and-drop blocks in OGABOOGA CODER, or type the
  words. They are the same language underneath.
- **Nothing to install.** The compiler and the IDE are C++, and the programs
  have no dependencies at all. Prefer the browser? Open the
  [Web IDE](../ide/) and code with the same C++ compiler as WebAssembly.

## Why is it not a toy?

Under the simple words, CMC has numbers (int and decimal), text, lists
(called *piles*), booleans (*gronk* and *nork*), functions (*clumps*),
recursion, comprehension-style list loops, random numbers, and drawing.
If you know Python, the built-in `cmc compile` turns any CMC program into
readable Python.

## Where to go next

- [Install CMC](install.md) -- takes one minute
- [Getting started](getting-started.md) -- your first program
- [The 30 minute tutorial](tutorial.md) -- the whole language, gently
- [All the words A-Z](words.html) -- one page for every word
- [OGABOOGA CODER](ogabooga-coder.md) -- meet the IDE

## A taste of the drawing power

```cmc
skrib size 400, 400
skrib color "gold"

booga 12
    skrib line 200, 200, 200 + lap * 30, 20
unga
oga "Sun rays drawn!"
```