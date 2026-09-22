# Cave Man Code -- Language Specification v0.1.0

This is the complete formal description of the CMC language. If you find a
place where the compiler disagrees with this document, that is a bug -- please
report it.

**Contents**

1. [Overview](#1-overview)
2. [Source files and encoding](#2-source-files-and-encoding)
3. [Lexical structure](#3-lexical-structure)
4. [Grammar](#4-grammar)
5. [Values and types](#5-values-and-types)
6. [Statements](#6-statements)
7. [Expressions and operators](#7-expressions-and-operators)
8. [Scopes and names](#8-scopes-and-names)
9. [Builtin words](#9-builtin-words)
10. [Drawing](#10-drawing)
11. [Errors](#11-errors)
12. [Resource limits](#12-resource-limits)
13. [Compilation to Python](#13-compilation-to-python)

---

## 1. Overview

Cave Man Code (CMC) is an imperative, dynamically typed language designed for
first-time programmers, especially young children. Its design goals, in order:

1. **Readability by a beginner.** One statement per line, blocks closed by a
   visible word (`unga`), no punctuation ceremony.
2. **Kind failure.** Every mistake produces a message with a line number and a
   concrete hint.
3. **Real capability.** The language is not a toy subset: it has lists,
   functions, recursion, string processing, random numbers, and drawing.
4. **Zero dependencies.** The reference implementation runs on a bare Python
   3.9+ installation.

The reference implementation consists of a lexer, a recursive-descent parser,
a tree-walking interpreter, and a source-to-source compiler that emits Python.
The IDE (OGABOOGA CODER) embeds the same implementation.

---

## 2. Source files and encoding

- Source files use the extension `.cmc` and are UTF-8 text.
- Line endings may be `\n` or `\r\n`; both are treated identically.
- A program is a sequence of lines. Blank lines and comment-only lines have no
  effect.
- One statement occupies one line. There is no line continuation and no
  statement separator other than the end of the line.

---

## 3. Lexical structure

### 3.1 Whitespace

Spaces and tabs separate tokens and are otherwise ignored. Whether two tokens
are separated by one space or eight makes no difference.

### 3.2 Comments

A comment begins with the word `ugg` (or its alias `comment`) and continues to
the end of the line.

```cmc
ugg this is a comment
oga "hi"   ugg this is also a comment
```

Comments inside text are just text: `"ugg"` prints the word.

### 3.3 Identifiers

An identifier is a sequence of letters, digits, and underscores that does not
begin with a digit. Unicode letters are permitted. Identifiers are
case-sensitive: `Rocks` and `rocks` are different names.

Reserved identifiers: the keywords in section 3.4, the builtin constants
`gronk`, `nork`, `plop`, `pi`, and the magic loop name `lap`. Using a reserved
name where an identifier is required is a compile-time error.

### 3.4 Keywords

Keyword recognition is case-insensitive and includes the English aliases. The
lexer normalizes all spellings to the canonical caveman word.

| Canonical | Aliases | Kind |
| --- | --- | --- |
| `oga` | `print`, `say` | statement |
| `grunk` | `let`, `make` | statement |
| `binga` | `if` | statement |
| `wonga` | `else` | statement modifier |
| `unga` | `end`, `done`, `finish` | statement |
| `booga` | `repeat` | statement |
| `zug` | `while` | statement |
| `zoop` | `for` | statement |
| `clump` | `fn`, `function`, `def` | statement |
| `ork` | `return`, `give` | statement |
| `blorp` | `ask` | expression |
| `skrib` | `draw` | statement |
| `and`, `or`, `not`, `in` | -- | operators |
| `ugg` | `comment` | comment |

### 3.5 Literals

**Numbers.** Digits with an optional single decimal point. Examples: `0`, `7`,
`1000000`, `3.5`, `.5` is *not* valid (write `0.5`), and `3.` is an error.
A number with a decimal point is a float; otherwise it is an integer.

**Text.** Single- or double-quoted. Text may not span lines. Escape sequences:

| Sequence | Meaning |
| --- | --- |
| `\n` | newline |
| `\t` | tab |
| `\r` | carriage return |
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |
| `\uXXXX` | Unicode code point (exactly four hex digits) |
| `\x` | literal `x` for any other character |

**Constants.** `gronk` (true), `nork` (false), `plop` (nothing), and `pi` are
predefined names, not literal syntax.

### 3.6 Operators and punctuation

`+` `-` `*` `/` `%` `=` `==` `!=` `<` `<=` `>` `>=` `(` `)` `[` `]` `,`

`!` alone, `&`, `|`, and `;` are rejected with targeted hints.

---

## 4. Grammar

The following is the complete grammar in EBNF-ish form. `NL` is a newline.

```
program        := { NL | statement NL }
statement      := say | grunk | assignment | index-assignment | if
                | repeat | while | foreach | clump | return | skrib
                | expression-statement
say            := "oga" expr { "," expr }
grunk          := "grunk" NAME "=" expr
assignment     := NAME "=" expr
index-assignment := postfix "[" expr "]" "=" expr
if             := "binga" expr NL block { "wonga" "binga" expr NL block }
                  [ "wonga" NL block ] "unga"
repeat         := "booga" expr NL block "unga"
while          := "zug" expr NL block "unga"
foreach        := "zoop" NAME "in" expr NL block "unga"
clump          := "clump" NAME "(" [ NAME { "," NAME } ] ")" NL block "unga"
return         := "ork" [ expr ]
skrib          := "skrib" shape arg-list
expression-statement := call | ask
block          := { NL | statement NL }

expr           := or-expr
or-expr        := and-expr { "or" and-expr }
and-expr       := not-expr { "and" not-expr }
not-expr       := "not" not-expr | comparison
comparison     := additive [ compare-op additive ]
compare-op     := "==" | "!=" | "<" | "<=" | ">" | ">="
                | "in" | "not" "in"
additive       := multiplicative { ("+" | "-") multiplicative }
multiplicative := unary { ("*" | "/" | "%") unary }
unary          := "-" unary | postfix
postfix        := primary { "[" expr "]" }
primary        := NUMBER | TEXT | NAME | NAME "(" [ expr { "," expr } ] ")"
                | "(" expr ")" | "blorp" [ expr | "(" [expr] ")" ]
```

Notes:

- A second comparison operator in the same comparison is a parse error.
- `ork` outside a `clump` is a runtime error.
- An expression statement that is not a call or a `blorp` is a parse error
  ("this line makes an answer but does nothing with it").

---

## 5. Values and types

| Type name (`what`) | Python type | Literal / producer |
| --- | --- | --- |
| `number` | `int`, `float` | number literals, math |
| `text` | `str` | quoted text, `goop` |
| `truth` | `bool` | `gronk`, `nork`, comparisons |
| `plop` | `None` | `plop`, empty `ork` |
| `pile` | `list` | `snorf(...)` |

Arithmetic between an integer and a float yields a float, except division,
which yields an integer when both operands are integers and the division is
exact.

Type errors are always errors, with two deliberate exceptions: `==` and `!=`
return `nork`/`gronk` for mismatched types instead of failing, and printing
anything via `oga` always succeeds.

---

## 6. Statements

### 6.1 `oga` -- print

Prints its expressions, converted with `show`, joined by single spaces, on one
line.

### 6.2 `grunk` and `=` -- variables

`grunk name = expr` creates (or replaces) a box in the current scope.
`name = expr` changes an existing box, searching enclosing scopes, and fails
if the box does not exist.

### 6.3 `binga` / `wonga` -- conditionals

The first `binga` whose condition is truthy runs; otherwise the `wonga` block
runs. `wonga binga` chains are parsed as nested conditionals but require only
one `unga` at the end of the chain.

### 6.4 `booga` -- counting loop

Runs the body once per integer from 1 to N inclusive. N is evaluated once.
Negative or zero N runs zero times. Each iteration sets the magic box `lap` in
the current scope.

### 6.5 `zug` -- while loop

Evaluates the condition before each iteration. The body must change something
or the step limit (section 12) stops the program.

### 6.6 `zoop` -- for-each loop

Iterates over a pile (items) or text (characters). The loop name is defined in
the current scope, refreshed each iteration.

### 6.7 `clump` / `ork` -- functions

`clump name(a, b)` defines a function in the current scope. Calling it creates
a fresh scope whose parent is the defining scope, binds the parameters, and
runs the body. `ork value` returns immediately. Falling off the end returns
`plop`.

### 6.8 Expression statements

A bare call such as `cheer()` or `plop(bag, "x")` is allowed. Any other
expression alone on a line is a parse error.

### 6.9 `skrib` -- drawing

See section 10.

---

## 7. Expressions and operators

### 7.1 Precedence

From tightest to loosest:

1. Grouping `( ... )`, calls `f(...)`, indexing `x[...]`
2. Unary minus `-`
3. `*` `/` `%` (left-associative)
4. `+` `-` (left-associative)
5. Comparisons `==` `!=` `<` `<=` `>` `>=` and `in` / `not in`
   (non-chainable)
6. `not` (right-associative)
7. `and`
8. `or`

### 7.2 Arithmetic semantics

| Operator | Operands | Result |
| --- | --- | --- |
| `+` | two numbers | numeric sum |
| `+` | two texts | concatenation |
| `+` | text and number | error with a `goop` hint |
| `-` `*` | numbers | numeric |
| `/` | numbers | int if exact, else float; division by zero is an error |
| `%` | numbers | remainder; zero divisor is an error |

### 7.3 Comparison semantics

`==` and `!=` compare numbers numerically, texts by value, piles deeply, and
booleans/`plop` by identity. Mismatched types are not equal (no error).

Ordering comparisons (`<` `<=` `>` `>=`) require two numbers or two texts.

### 7.4 Membership

`x in y`: if `y` is a pile, tests deep equality against each item; if `y` is
text, tests substring containment and requires `x` to be text. `not in` is the
negation.

### 7.5 Truthiness

| Value | Truthy? |
| --- | --- |
| `plop` | no |
| `nork` | no |
| `gronk` | yes |
| `0` / `0.0` | no |
| other numbers | yes |
| `""` | no |
| non-empty text | yes |
| empty pile | no |
| non-empty pile | yes |

### 7.6 Indexing

Piles and text support `x[i]` with integer indices. Positive indices count
from 0; negative indices count from the end (`-1` is the last item).
Out-of-range indices are errors with the valid range in the message. Text
items are one-character texts. Assigning to a pile spot is allowed when the
spot exists; assigning to text is an error.

---

## 8. Scopes and names

- The top level is the global scope.
- `grunk` defines a name in the current scope.
- Blocks (`binga`, `booga`, `zug`, `zoop`) do **not** create scopes; names
  defined inside them are visible after the block, and `lap`/loop names are
  defined in the enclosing scope.
- `clump` bodies run in their own scope whose parent is the scope where the
  clump was defined (lexical scoping).
- Lookup walks the scope chain, then the builtin constants, then errors.

---

## 9. Builtin words

All builtins validate their argument count and types. See
[03_KEYWORD_DICTIONARY.md](03_KEYWORD_DICTIONARY.md) for the full reference.

| Builtin | Signature | Notes |
| --- | --- | --- |
| `snorf` | `snorf(...)` | make a pile |
| `plop` | `plop(pile, x)` | append |
| `nom` | `nom(pile_or_text)` | length |
| `skoop` | `skoop(pile_or_text, i)` | same as `x[i]` |
| `yoink` | `yoink(pile)` | pop, error when empty |
| `goop` | `goop(x)` | to text (`show`) |
| `shout` / `whisper` | `(text)` | case conversion |
| `flip` | `(text)` | reverse |
| `find` | `(text, piece)` | index or -1 |
| `split` | `(text, sep)` | pile of pieces |
| `join` | `(pile, sep)` | text |
| `what` | `(x)` | type name |
| `numba` | `(text_or_number)` | to number |
| `munga` | `(low, high)` | random int inclusive |
| `round` / `flat` / `roof` | `(number)` | nearest / floor / ceil |
| `abs` | `(number)` | absolute value |
| `small` / `big` | `(a, b)` | min / max |
| `root` | `(number)` | square root, non-negative |
| `pow` | `(base, exp)` | power |
| `wait` | `(ms)` | pause |

`munga` with both arguments integers uses a uniform inclusive integer range.
`root` returns an integer when exact.

---

## 10. Drawing

`skrib` is a statement, not an expression.

| Statement | Arguments |
| --- | --- |
| `skrib clear` | none |
| `skrib size w, h` | whole numbers |
| `skrib color c` | text or expression |
| `skrib dot x, y, r` | numbers |
| `skrib circle x, y, r` | numbers |
| `skrib line x1, y1, x2, y2` | numbers |
| `skrib box x, y, w, h` | numbers |
| `skrib blob x, y, w, h` | numbers |
| `skrib write text, x, y` | any expression for text |

Coordinates start at the top-left (`0, 0`) with `x` right and `y` down.
Drawing commands are no-ops when no drawing surface is attached (for example
in tests). The IDE draws into its Draw tab; the terminal runs open a window.

---

## 11. Errors

All errors are instances of `CmcError` with:

- a message (kid-friendly),
- a line number when known,
- a hint (always present for runtime errors, present for most parse errors).

Formatted form:

```
OOGA! Line 7: There is no box named 'rocks'.
Hint: make it first with: grunk rocks = ...
```

Categories: `CmcLexError`, `CmcParseError`, `CmcRuntimeError`, plus control
exceptions `CmcStopped` (user pressed stop), `CmcStepLimit`, `CmcTooDeep`.

---

## 12. Resource limits

| Limit | Default | Notes |
| --- | --- | --- |
| Steps | 5,000,000 | counted per statement and per loop iteration |
| Call depth | 200 | `clump` calls, including recursion |
| `wait` | user-supplied | stop-aware when run by the IDE |

Exceeding a limit produces a friendly runtime error, not a crash.

---

## 13. Compilation to Python

`cmc compile program.cmc` writes `program.py`, a standalone Python 3 file:

- a header comment naming CMC and the source file,
- an embedded runtime (`_cmc_*` helpers) implementing CMC semantics,
- the translated program,
- a footer that keeps a picture window open if anything was drawn.

Translation is direct: `oga` becomes `_cmc_say`, `booga` becomes
`for lap in _cmc_laps(n)`, `binga/wonga` chains become `if/elif/else`, clumps
become `def`, and operators become helper calls that preserve CMC's type rules.

Semantic differences to be aware of:

- The generated file has no step limit or recursion depth check (Python's own
  recursion limit applies).
- `grunk` and `=` both become plain Python assignment, so assigning to an
  undefined name does not raise CMC's "no box named" error in the compiled
  file.