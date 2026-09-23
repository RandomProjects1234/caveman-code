# Error messages: the complete list

Every message CMC can show, what caused it, and the fix. Every runtime error
carries a line number and a hint, so the compiler output alone is usually
enough -- this page is for teachers, or for reading with a cup of tea.

## Lexer errors (typos in the shapes of words)

| Message | Cause | Fix |
| --- | --- | --- |
| `I found a single '!' but I do not know what it means.` | `!` alone | `!=` to compare, `not` to flip |
| `I found '&' but CMC uses words for that.` | `&&`/`\|\|` habits | `and` / `or` |
| `I found a ';'. CMC does not use those.` | semicolons | one statement per line |
| `I do not know the squiggle 'X'.` | an unexpected character | type the word again |
| `The number 3. is missing the numbers after the dot.` | trailing dot | `3.0` |
| `There are too many dots in that number.` | `1.2.3` | one dot only |
| `Some text started with a " but never finished.` | unclosed quote | close the text |
| `Text cannot go over more than one line.` | newline inside text | use `+` to join |
| `The text ends with a lonely backslash.` | `\` at the end | `\\` for a real backslash |
| `That \u escape needs four numbers after it, like \u2764.` | bad escape | exactly four hex digits |

## Parser errors (shapes of statements)

| Message | Cause | Fix |
| --- | --- | --- |
| `A block was opened but never closed.` | missing `unga` | add it under the block |
| `You said 'unga', but nothing was open for it to close.` | extra `unga` | remove it or add a block |
| `There are extra things on this line that I do not understand: '2'.` | two statements on a line | press Enter between them |
| `After 'grunk' comes the name of the new box.` | `grunk = 5` | `grunk x = 5` |
| `The box 'x' needs a '=' and something to put in it.` | `grunk x 5` | add `=` |
| `'nom' already means something in CMC, so it cannot be a name you make up.` | reserved name | add a letter/number |
| `A 'zoop' needs the little word 'in'` | `zoop x xs` | `zoop x in xs` |
| `A clump name needs '(' after it` | `clump foo` | `clump foo()` |
| `This clump's box list needs a ')' to close it.` | unclosed params | add `)` |
| `This line makes an answer but does nothing with it.` | `5 + 5` alone | `oga 5 + 5` |
| `One compare at a time, please!` | `1 < x < 9` | `1 < x and x < 9` |
| `'skrib' does not know how to draw 'dragon'.` | unknown shape | one of the nine shapes |
| `'skrib circle' needs 3 numbers after it` | wrong arg count | `skrib circle 100, 100, 30` |
| `This '(' needs a ')' to close it.` | unbalanced parens | count them |
| `'and' is a joining word, so a line cannot start with it.` | line starts with `and` | put a check first |
| `I expected something to use here, but found ...` | expression missing | add a value |

## Runtime errors

### Names and scopes

| Message | Cause | Fix |
| --- | --- | --- |
| `There is no box named 'x'.` | used before `grunk`, or typo | `grunk x = ...` |
| `There is no box named 'x' to put that into.` | `x = ...` before making it | make the box first |
| `There is no clump named 'f'.` | called before defining | define it above the call |
| `'x' is a box, not a clump, so it cannot be called with ( ).` | added `()` to a value | remove the `()` |
| `'f' wants 2 things, but got 3.` | wrong argument count | match the definition |
| `'ork' can only live inside a clump.` | `ork` at the top level | move it into a clump |

### Types and math

| Message | Cause | Fix |
| --- | --- | --- |
| `Text and numbers cannot be stuck together directly.` | `"a" + 1` | `"a" + goop(1)` |
| `Cannot add ... and ...` | `+` on piles/truth | only numbers or two texts |
| `Text cannot be multiplied.` | `"a" * 3` | build the text in a loop |
| `Cannot share by zero!` | `x / 0` | check before dividing |
| `Cannot find the remainder with zero.` | `x % 0` | check the divisor |
| `I can only line up numbers with numbers, or text with text` | `3 < "four"` | `goop` one side |
| `numba cannot turn "banana" into a number.` | non-number text | ask again, or check with `what` |
| `numba cannot turn gronk/nork into a number.` | boolean passed | use 1 / 0 |
| `numba needs text or a number` | pile passed | `goop` it or pick an item |

### Piles and text

| Message | Cause | Fix |
| --- | --- | --- |
| `There is no spot 9 in that pile (it has 3 spots).` | bad index | spots start at 0 |
| `Only piles can have spots changed` | `word[0] = "C"` | build new text |
| `That pile has no spot 9 yet` | assigning past the end | `plop` new items |
| `The pile is empty, so there is nothing to yoink!` | yoink on empty | check `nom(pile) > 0` |
| `nom counts piles and text` | `nom(5)` | give it a pile or text |
| `skoop reaches into piles and text` | `skoop(5, 0)` | give it a pile or text |
| `'in' can look inside text, but only for other text.` | `7 in "abc"` | `goop(7)` |
| `'in' looks inside piles and text` | `x in 5` | make a pile |
| `split needs something to cut on, and an empty text cuts nothing.` | `split(t, "")` | give a separator |
| `'zoop' walks through piles and text` | `zoop x in 5` | make a pile |

### Drawing

| Message | Cause | Fix |
| --- | --- | --- |
| `'skrib size' needs a whole number` | decimals in a size | `round` it |
| `skrib` messages about whole numbers | decimal coordinates where whole is needed | `round` them |

### Runaway protection

| Message | Cause | Fix |
| --- | --- | --- |
| `This program ran for a very long time. Maybe a loop never stops?` | `zug` check never turns nork | change the checked box inside |
| `'f' called itself too many times and went too deep.` | recursion without a stopping case | add a `binga` that returns |

### The stop button

| Message | Cause | Fix |
| --- | --- | --- |
| `Stopped!` | the person pressed STOP / Esc / Ctrl+C | nothing -- this is on purpose |

## How errors reach you

| Where you ran | What you see |
| --- | --- |
| OGABOOGA CODER | red text in the output panel, with a line number |
| `cmc run` | the error on stderr, exit code 1 |
| `cmc` REPL | the error printed, and the session continues |
| C++ API | a `cmc::CmcError` exception with `.message`, `.line`, `.hint` |

## Writing tests for errors

```cpp
try {
    cmc::Interpreter interpreter;
    interpreter.run("oga 1 / 0");
} catch (cmc::CmcError& error) {
    assert(error.message.find("share by zero") != std::string::npos);
    assert(!error.hint.empty());
}
```