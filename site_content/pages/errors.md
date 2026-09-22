# Errors: what to do when OOGA happens

CMC never shows you a scary wall of gibberish. Every mistake looks like this:

```text
OOGA! Line 7: There is no box named 'rocks'.
Hint: make it first with: grunk rocks = ...
```

Two lines: what went wrong, and how to fix it. That is all.

## Reading an error

- **OOGA!** -- a mistake happened. Nobody is in trouble.
- **Line 7** -- the line to look at. In OGABOOGA CODER you can see line
  numbers in the gutter next to the code.
- **Hint** -- the friendly fix.

## The mistakes everyone meets

### Blocks

| Message | What happened |
| --- | --- |
| `A block was opened but never closed.` | you forgot an `unga` |
| `You said 'unga', but nothing was open for it to close.` | one `unga` too many |

Count your blocks. Every `binga`, `booga`, `zug`, `zoop`, and `clump` needs
exactly one `unga`.

### Boxes

| Message | What happened |
| --- | --- |
| `There is no box named 'x'.` | use before `grunk`, or a typo |
| `There is no box named 'x' to put that into.` | `x = ...` before making the box |
| `'nom' already means something in CMC` | pick a different name |

### Text and numbers

| Message | What happened |
| --- | --- |
| `Text and numbers cannot be stuck together directly.` | use `goop(7)` |
| `numba cannot turn "banana" into a number.` | the answer was not a number |
| `Cannot share by zero!` | someone divided by 0 |
| `I can only line up numbers with numbers, or text with text` | `<` between kinds |

### Piles

| Message | What happened |
| --- | --- |
| `There is no spot 9 in that pile (it has 3 spots).` | spot numbers start at 0 |
| `The pile is empty, so there is nothing to yoink!` | check `nom(pile) > 0` |
| `'zoop' walks through piles and text` | you gave it a number |

### Clumps

| Message | What happened |
| --- | --- |
| `'add' wants 2 things, but got 3.` | check the parentheses |
| `There is no clump named 'gret'.` | typo, or define it first |
| `'x' is a box, not a clump` | remove the `()` or rename |
| `'ork' can only live inside a clump.` | move it inside a `clump` block |

### Runaways

| Message | What happened |
| --- | --- |
| `This program ran for a very long time. Maybe a loop never stops?` | a `zug` check never becomes nork |
| `called itself too many times and went too deep.` | recursion without a stopping `binga` |

## The three step fix

1. **Read the line number.** Go there.
2. **Read the hint.** It usually names the exact word to add.
3. **Change one thing and run again.** Small steps are fastest.

If you get stuck, delete the last thing you added. The bug is almost always
there.

## Errors are nothing to fear

Every programmer in the world sees errors all day. In CMC they even have a
name: **OOGA**. Collect them. Each one teaches you one thing the computer
needs.

Next: [English mode](english-mode.md).