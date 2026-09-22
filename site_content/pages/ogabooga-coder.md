# OGABOOGA CODER

OGABOOGA CODER is the home cave of CMC: an editor, a run button, a picture
canvas, and the whole compiler **inside one window**. There is nothing else to
install.

Start it by double-clicking `run_ogabooga.bat`, or:

```text
python -m ogabooga
```

## The window

- **Toolbar:** RUN, STOP, New, Open, Save, Examples, Dictionary, Export .py,
  font size buttons.
- **Tabs:**
  - **Blocks (drag and drop)** -- the block workshop
  - **Text (caveman code)** -- type real CMC
  - **Draw (picture)** -- where `skrib` draws
- **Output panel** at the bottom: everything your program says, plus friendly
  OOGA errors.
- **Status bar**: the current hint on the left, line/column or the file name
  on the right.

## Running

Press **RUN** (or F5). CMC always runs the tab you are looking at: the blocks
if you are on the Blocks tab, the text if you are on the Text tab.

While it runs:

- `oga` lines appear in the output panel
- `blorp` pops up a friendly question window
- `skrib` draws in the Draw tab
- the STOP button (or Esc) stops even a runaway loop
- `wait(500)` still lets Stop work

When it finishes you get a green **Program finished!** line, or a red OOGA
error with a line number and a hint.

## The Blocks tab

Drag colorful blocks from the left, stack them, nest them, and drop little
value blocks into slots. The right side shows the CMC code writing itself.
Press RUN to run it. See the full [Blocks guide](blocks-guide.md).

## The Text tab

A real code editor with:

- line numbers
- CMC syntax colors (words, text, numbers, notes)
- Tab indents four spaces
- Enter keeps your indentation, and adds four spaces after `binga`, `booga`,
  `zug`, `zoop`, or `clump`
- full undo (Ctrl+Z)

## The Draw tab

A 500x400 white picture for `skrib`. The Wipe button clears it between runs
(CMC also clears it each time you press RUN).

## Examples and dictionary

- **Examples** opens all the example programs with a preview. Open one in the
  Text tab to read it, or as Blocks to play with its shape.
- **Dictionary** is the whole caveman dictionary: search a word, see how to
  write it, what it does, an example, tips, and see-also links.

## Export as Python

**Export .py** turns your CMC program into a real Python file. You can run
that file anywhere Python is installed, read it, and change it. It is a
friendly way to grow into Python later.

```cmc
oga "hello"
```

becomes Python with a small `_cmc_*` helper runtime and:

```python
_cmc_say("hello")
```

## Keyboard shortcuts

See [Keyboard shortcuts](keyboard-shortcuts.md). The big three: **F5** run,
**Esc** stop, **Ctrl+S** save.

## Files

CMC programs are plain text files ending in `.cmc`. They open in any editor.
Keep them anywhere; OGABOOGA CODER remembers the file you are working on when
you save.

Next: [Blocks guide](blocks-guide.md).