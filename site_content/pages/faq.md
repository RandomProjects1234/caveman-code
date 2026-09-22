# FAQ

## Is CMC a real programming language?

Yes. It has a lexer, a parser, an interpreter, a compiler, a test suite, a
language manual, and a package you can install. "Real" means it runs programs
and people can build things with it. It is also small enough to read in an
afternoon, which most languages are not.

## Do I need to be good at math?

No. CMC has the normal four operations and a handful of helpers. You need
counting more than math.

## What age is CMC for?

The words and blocks are designed so a four year old can copy a working
program and change it. The full language has enough depth (recursion, piles,
text processing, drawing) for older kids and adults learning their first
language.

## Why do errors say OOGA?

Because it is funny and it removes fear. A mistake is an *ooga* -- a bump in
the cave. The message underneath always explains the fix in plain words.

## How long does it take to learn?

- 10 minutes: print, boxes, one loop.
- 30 minutes: the whole tutorial (see [Tutorial](tutorial.md)).
- A weekend: comfortable with clumps, piles, and text.
- Years: writing big programs well, like any language.

## Can I use it in a classroom?

Yes. MIT license, no dependencies, no accounts, no internet. The IDE is a
single window. Teachers can print the [cheat sheet](cheatsheet.md) and the
repo has a full test suite for grading projects.

## Does it run on a phone or tablet?

Not yet. It runs on Windows, macOS, and Linux desktops and laptops. A browser
version is on the [roadmap](roadmap.md).

## Can CMC make real games?

Small ones, yes -- text adventures, guessing games, drawing toys, and simple
animations with `wait` and `skrib`. Full graphics games would need more
library work; that is part of what "hard to master" means.

## Why Python underneath?

Python is already installed on millions of computers, reads like English, and
lets `cmc compile` produce code a learner can grow into. CMC is also small
enough that a future version could target other machines.

## Is CMC just a Python wrapper?

No. It has its own grammar (`unga` blocks, `skrib` statements, friendly
checks) and its own runtime rules (whole-number division, friendly step
limits, type-aware errors). The interpreter is written in Python, the same way
many languages are written in C.

## Can I mix CMC and Python?

Through the compiler: `cmc compile hello.cmc` makes readable Python. CMC
programs cannot import Python libraries directly in v0.1 -- that is on the
roadmap.

## Why are lists called piles and booleans called gronk?

Because naming things like a caveman makes them sticky in your memory, and
because it is fun. Every caveman word also has an English twin you can use
instead: `snorf`/`list`-style words, `gronk`/`true`.

## Is it safe for kids to run?

Yes. Programs cannot touch files, networks, or the system in v0.1. The worst
thing a program can do is run a very long loop, and CMC stops that for you
after about five million steps.

## How do I share my program?

Save the `.cmc` file and send it. It is plain text. Anyone with CMC (or the
website's future playground) can run it.

## How can I help?

See [Contributing](contributing.md). New examples, translations of the
dictionary, bug reports with tiny test programs, and classroom stories are all
gold.

Next: [Glossary](glossary.md).