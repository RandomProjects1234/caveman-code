# Cave Man Code (CMC)

**A real programming language a four year old can start using in 10 minutes --
and still be learning from years later.**

```cmc
ugg your first program
oga "Ooga booga!"

grunk rocks = 3
booga rocks
    oga "bang rock!", lap
unga

binga rocks > 2
    oga "that is a lot of rocks"
wonga
    oga "need more rocks"
unga
```

No semicolons. No curly braces. Blocks end with `unga`. Every mistake gets a
friendly `OOGA!` message with a line number and a hint. And underneath the
silly words there is a complete little language: numbers, text, lists
(called *piles*), functions (*clumps*), recursion, random numbers, and
drawing.

## Quick start

Grab a zip from the [download page](website/download.html) (or the
`website/downloads/` folder in a fresh checkout), unzip it, and:

- **Windows:** double-click `OGABOOGA CODER.exe` (the IDE) or drag a `.cmc`
  file onto `cmc.exe` (the runner)
- **Terminal:** `cmc.exe run examples\01_hello_oga.cmc`

Everything is built in C++ with no dependencies. Nothing else to install.
To build it yourself with g++ or clang, run `native\build.bat`.

## OGABOOGA CODER

The IDE has the compiler inside it and three ways to play:

- **Blocks** -- drag-and-drop blocks that write CMC code as you build
- **Text** -- a real editor with line numbers and CMC syntax colors
- **Draw** -- a picture canvas where `skrib` draws

Press RUN (F5), press STOP (Esc), ask questions with `blorp`, save `.cmc`
files, and export any program as readable Python with one click.

## The language in one table

| CMC | English | Means |
| --- | --- | --- |
| `oga` | `print` | say something |
| `grunk x = 5` | `let x = 5` | make a box (variable) |
| `binga` / `wonga` | `if` / `else` | decide |
| `booga 5` | `repeat 5` | loop N times (with magic `lap`) |
| `zug x < 9` | `while x < 9` | loop while true |
| `zoop pet in pets` | `for pet in pets` | loop over a pile |
| `clump f(a)` / `ork` | `function` / `return` | make your own word |
| `unga` | `end` | close a block |
| `blorp` | `ask` | ask the person a question |
| `skrib circle 100, 100, 30` | `draw ...` | draw a picture |
| `ugg` | `comment` | a note for humans |

Plus helpers: `snorf` (make a pile), `plop` (add), `nom` (count), `skoop`
(spot), `yoink` (pop), `goop` (to text), `shout`, `whisper`, `flip`, `find`,
`split`, `join`, `what`, `numba`, `munga` (random), `round`, `flat`, `roof`,
`abs`, `small`, `big`, `root`, `pow`, and `wait`.

Every word has its own wiki page with examples: **45 word pages**, 30 guides,
and a 30-minute tutorial. See [website/index.html](website/index.html).

## What makes it good

- **Kind errors.** `OOGA! Line 4: A block was opened but never closed.
  Hint: every 'binga', 'booga', 'zug', 'zoop', and 'clump' block ends with
  'unga'.`
- **No freeze.** Runaway loops stop after five million steps with an
  explanation. Infinite recursion stops at 200 levels with a hint.
- **Blocks and text are one language.** Any program can become blocks and
  back (`source_to_blocks` / `blocks_to_source`), because both share the same
  compiler.
- **A real compiler.** `cmc compile hello.cmc` writes `hello.py`: standalone,
  readable Python with a small embedded runtime.
- **Zero dependencies.** The compiler, REPL, IDE, and tests are plain C++
  (C++17) with no libraries beyond the Windows system DLLs.

## Repo layout

```text
caveman-code/
  README.md          you are here
  CHEATSHEET.md      one-page reference
  native/src/engine/ the compiler (lexer, parser, interpreter, builtins, codegen)
  native/src/cli/    the cmc command line (run, compile, REPL)
  native/src/ide/    OGABOOGA CODER, the IDE with the compiler inside
  native/tests/      test scripts and expected outputs
  bin/               the built programs (cmc.exe, ogabooga.exe)
  examples/          31 programs with expected outputs
  docs/              the full manual (spec, errors, IDE guide, publishing)
  site_content/      wiki sources (markdown + dictionary data)
  website/           the built wiki, main page, and download page
  tools/             release builder (PowerShell)
```

## Commands

```text
bin\cmc.exe                     talk to CMC line by line (REPL)
bin\cmc.exe run hello.cmc       run a program
bin\cmc.exe compile hello.cmc   make hello.py
bin\cmc.exe init hello.cmc      make a starter program
bin\cmc.exe examples            list example programs
OGABOOGA CODER.exe                open the IDE
native\build.bat                build both programs with g++
powershell -File native\tests\run_tests.ps1     run the test suite
powershell -File tools\build_release.ps1        build the download zips
```

## Docs

| Where | What |
| --- | --- |
| [website/index.html](website/index.html) | the wiki home (open it in a browser) |
| [docs/00_INDEX.md](docs/00_INDEX.md) | the repo manual index |
| [docs/01_LANGUAGE_SPEC.md](docs/01_LANGUAGE_SPEC.md) | complete language specification |
| [docs/02_TUTORIAL_30_MINUTES.md](docs/02_TUTORIAL_30_MINUTES.md) | the tutorial |
| [docs/03_KEYWORD_DICTIONARY.md](docs/03_KEYWORD_DICTIONARY.md) | every word |
| [docs/04_ERROR_MESSAGES.md](docs/04_ERROR_MESSAGES.md) | every error and its fix |
| [docs/05_OGABOOGA_CODER_MANUAL.md](docs/05_OGABOOGA_CODER_MANUAL.md) | the IDE manual |
| [docs/09_PUBLISHING_GITHUB.md](docs/09_PUBLISHING_GITHUB.md) | how to publish this project to GitHub |

## Publishing

The step-by-step guide to putting CMC on GitHub (repo, Pages, releases) is in
[docs/09_PUBLISHING_GITHUB.md](docs/09_PUBLISHING_GITHUB.md). The short
version:

```text
gh auth login
git init && git add . && git commit -m "Cave Man Code v0.1.0: the first fire"
gh repo create caveman-code --public --source . --push
```

Then enable GitHub Pages on the `/website` folder.

## License

MIT. Go make something silly.

Ugg say: small steps, big rocks.