# Syntax and grammar

CMC is deliberately small. There is one thing per line, blocks end with
`unga`, and notes start with `ugg`.

## The shape of a program

```cmc
ugg notes first (optional)
grunk boxes
clump helpers
booga / binga / zug / zoop blocks
oga answers
```

The computer reads from top to bottom. A clump must be made before you call
it. A box must be made before you use it.

## Lines

- **One statement per line.** No semicolons. Pressing Enter ends the thought.
- **Text is in quotes.** `"double"` or `'single'` both work.
- **Notes:** `ugg` ignores the rest of the line.
- **Blank lines** do nothing and make your code easier to read.
- **Case matters** for names you invent (`rocks` and `Rocks` are different
  boxes). The CMC words themselves are always lowercase, but English aliases
  like `PRINT` also work because the words are matched without case.

## Blocks

Every block starts with a word and ends with `unga`:

```cmc
binga <check>
    <things>
unga

booga <count>
    <things>
unga

zug <check>
    <things>
unga

zoop <name> in <pile>
    <things>
unga

clump <name>(<boxes>)
    <things>
unga
```

Blocks can live inside blocks. Every block needs its own `unga`:

```cmc
booga 3
    binga lap == 2
        oga "the middle lap!"
    unga
unga
```

An `unga` with nothing open gives a friendly error, and a missing `unga` does
too.

## Statements

| Statement | Meaning |
| --- | --- |
| `oga <things>` | print |
| `grunk <name> = <thing>` | make a box |
| `<name> = <thing>` | change a box |
| `<pile>[<spot>] = <thing>` | change one spot of a pile |
| `binga` / `booga` / `zug` / `zoop` / `clump` | blocks |
| `ork <thing>` | give an answer back |
| `skrib <shape> <args>` | draw |
| `<clump>(<args>)` | call one of your clumps |

A line that only makes an answer without using it (like `5 + 5` alone) gets a
kind error that suggests `oga`.

## Expressions

Expressions are how you build values. From strongest glue to weakest:

| Level | What | Example |
| --- | --- | --- |
| 1 | numbers, text, names, `(...)` | `42`, `"hi"`, `x`, `(1 + 2)` |
| 2 | spots and calls | `pets[0]`, `double(4)`, `snorf(1, 2)` |
| 3 | minus in front | `-5`, `-(a + b)` |
| 4 | `*` `/` `%` | `6 * 7`, `10 % 3` |
| 5 | `+` `-` | `2 + 3`, `a - b` |
| 6 | `==` `!=` `<` `<=` `>` `>=` `in` `not in` | `rocks >= 3`, `"cat" in pets` |
| 7 | `not` | `not gronk` |
| 8 | `and` | `a > 1 and b > 1` |
| 9 | `or` | `a == 1 or a == 2` |

You never compare three things at once. `1 < 2 < 3` gives a friendly error
telling you to use `and`.

## Names you invent

- Letters, numbers, and `_`
- Cannot start with a number: `rock2` is fine, `2rock` is not
- Cannot be a CMC word: `nom`, `unga`, `gronk`, and friends are taken
- `lap` is reserved for the magic loop box

Good names say what is inside: `rocks`, `player_name`, `total_score`.

## Text details

Text can contain escape codes:

| Write | You get |
| --- | --- |
| `\n` | a new line |
| `\t` | a tab |
| `\"` | a quote |
| `\\` | a backslash |
| `\u2764` | a character by code point |

Text cannot run over more than one line. Use `+` to join pieces.

## Numbers

- Whole numbers: `0`, `7`, `-3`, `1000000`
- Decimals: `3.5`, `0.25`, `-2.75`
- Division gives a whole number when it fits (`4 / 2` is `2`), otherwise a
  decimal (`5 / 2` is `2.5`)
- `%` gives the remainder (`10 % 3` is `1`)

## What the parser checks for you

- `unga` without an open block
- blocks that never close
- `binga` with no check
- `zoop` without `in`
- skipped commas in `skrib`
- calling something that is not a clump
- making up names that CMC already uses

All of these come with a line number and a hint. See [Errors](errors.md).