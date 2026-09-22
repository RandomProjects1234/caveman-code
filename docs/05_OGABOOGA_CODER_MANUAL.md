# OGABOOGA CODER manual

OGABOOGA CODER is the official CMC editor, and the compiler lives inside it.
One window, no installs, no accounts, no internet.

```text
python -m ogabooga          open it
python -m ogabooga --selftest   check that it works, then exit
```

## 1. The window at a glance

```text
+---------------------------------------------------------------+
| RUN  STOP | New Open Save | Examples Dictionary Export .py ... |   toolbar
+---------------------------------------------------------------+
| [ Blocks (drag and drop) ] [ Text (caveman code) ] [ Draw ]   |   tabs
|                                                               |
|   ...the tab you are looking at...                            |
|                                                               |
+---------------------------------------------------------------+
| OUTPUT                                              [Clear]   |   output
| ...program talking and OOGA errors...                         |
+---------------------------------------------------------------+
| status message              |   line/col or file name         |   status
+---------------------------------------------------------------+
```

## 2. Running programs

- **RUN** (F5) runs whatever tab you are looking at.
- **STOP** (Esc) stops a running program. Stop works even during `wait(...)`.
- Programs run in their own thread, so the window never freezes.
- The Draw tab is wiped at the start of every run.
- The output panel is cleared at the start of every run.

What the program can do while it runs:

| CMC thing | What you see |
| --- | --- |
| `oga` | a line in the output panel |
| `blorp` | a pop-up question window |
| `skrib` | drawing in the Draw tab |
| `wait` | the program pauses, STOP still works |

When the program ends:

- green `Program finished! Well done, cave coder.`
- red `Stopped! (you pressed stop)`
- red OOGA error with line number and hint

## 3. The Text tab

A real editor with:

- **Line numbers** on the left.
- **Syntax colors**: CMC words in amber, built-ins in blue, text in green,
  numbers in purple, notes in grey.
- **Tab** inserts four spaces.
- **Enter** keeps your indentation and adds four spaces after `binga`,
  `booga`, `zug`, `zoop`, and `clump`.
- **Ctrl+Z / Ctrl+Y** undo and redo.

Write a program, press RUN, watch the output panel. That is the whole loop.

## 4. The Blocks tab

Three columns:

1. **Palette** (left): blocks grouped by category. Click one to add it to the
   end of the stack, or drag it where you want it.
2. **Workshop** (middle): your blocks. Drop a block inside a shaded area to
   nest it. A white line shows where a dragged block will land.
3. **YOUR CODE** (right): the CMC text the blocks are writing. It updates as
   you build.

Slots are the little dark boxes. Click to type an expression, or drag a value
block (number, text, box value, random) into them.

Right-click a block for **Delete**, **Copy**, **Move up**, **Move down**.
Drag a block onto the palette to delete it. **Undo** (Ctrl+Z) remembers every
change.

The blocks toolbar also has:

- **Send to Text tab** -- put the generated CMC in the editor
- **Load from Text tab** -- turn the editor's CMC back into blocks
- **Clear blocks**

## 5. The Draw tab

A 500x400 white picture. `skrib` draws here. The **Wipe picture** button
clears it. Coordinate `0, 0` is the top-left.

## 6. Examples

**Examples** opens a list of every program in `examples/`. Click a name to
preview it. Then choose:

- **Open in Text tab** -- edit it as code
- **Open as Blocks** -- play with its shape (if a line has no block yet, CMC
  says which one and opens it as text instead)

Example programs are read-only friends: use **Save as** to keep your changes.

## 7. The dictionary

**Dictionary** opens the whole language: search for `grunk`, `munga`, or
`skrib`, and see the syntax, explanation, example, tips, and see-also links.

## 8. Saving and opening

- Files end in `.cmc` and are plain UTF-8 text.
- **Save** asks for a name the first time.
- Saving while on the Blocks tab writes the generated CMC text.
- **New** clears the editor and forgets the current file.

## 9. Export as Python

**Export .py** compiles the current tab to standalone Python. The generated
file contains a readable copy of the CMC runtime and your program translated
line for line. It runs anywhere Python 3.9+ runs:

```text
python my_program.py
```

## 10. Fonts and comfort

**A+** and **A-** change the code and output font size. Everything else scales
with your system settings.

## 11. Keyboard shortcuts

| Key | Action |
| --- | --- |
| `F5` | run |
| `Esc` | stop |
| `Ctrl+N` | new |
| `Ctrl+O` | open |
| `Ctrl+S` | save |
| `Ctrl+Z` | undo (text: editor undo; blocks: block undo) |
| `Ctrl+Y` | redo in text |
| `Tab` | indent four spaces |
| `Return` | new line with smart indentation |
| `Delete` | delete selected block (blocks tab) |

## 12. Troubleshooting

| Problem | Likely cause | Fix |
| --- | --- | --- |
| RUN does nothing | current tab has no code | check which tab is showing |
| `no display` error | no desktop available | use `cmc run` from a terminal |
| question window missing | program never reached `blorp` | read the output for where it stopped |
| picture is blank | drew, then cleared; or wrong tab? | `skrib` draws in the Draw tab |
| `Load from Text` refuses | a line has no block yet | message says which line; use text |
| blocks not running | a slot has a problem | red hint in the status bar names it |

## 13. How the compiler is inside

`src/ogabooga/` imports `src/cmc/` directly:

- the runner calls `cmc.Interpreter` in a worker thread
- the code panel uses `cmc.parse_source` and the block generator
- the dictionary is built from `src/cmc/words.py`
- Export uses `cmc.to_python_source`

There is no second copy of the language: pressing RUN runs exactly what
`cmc run file.cmc` would run, with the same errors and the same results.

## 14. The self test

```text
python -m ogabooga --selftest
```

Builds the whole window without showing it, runs a program through the
interpreter, round-trips a program through the block model, and prints
`OGABOOGA SELFTEST OK`. Useful before shipping a build.