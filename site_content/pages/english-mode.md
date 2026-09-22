# English mode

You can write CMC with normal English words if you like. They are mixed in
freely with the caveman words -- same language, same meaning.

```cmc
let x = 5
if x > 3
    print "x is big"
else
    print "x is small"
end
```

That is exactly the same program as:

```cmc
grunk x = 5
binga x > 3
    oga "x is big"
wonga
    oga "x is small"
unga
```

## The translation table

| Caveman | English |
| --- | --- |
| `oga` | `print`, `say` |
| `grunk` | `let`, `make` |
| `binga` | `if` |
| `wonga` | `else` |
| `unga` | `end`, `done`, `finish` |
| `booga` | `repeat` |
| `zug` | `while` |
| `zoop` | `for` |
| `clump` | `fn`, `function`, `def` |
| `ork` | `return`, `give` |
| `blorp` | `ask` |
| `skrib` | `draw` |
| `ugg` | `comment` |
| `gronk` | `true` |
| `nork` | `false` |
| `plop` | `nothing` |

Everything else -- `snorf`, `nom`, `goop`, `munga`, `and`, `or`, `not`,
`in` -- is already a word you can read.

## Why have two languages?

- The caveman words are silly, memorable, and short. Kids remember `grunk`.
- The English words help grown-ups and teachers feel at home.
- Both compile to the same thing, so you can switch whenever you want.

## Fun: the same program twice

```cmc
function greet(name)
    print "hello " + name
end

grunk people = snorf("Ugg", "Oona")
for person in people
    greet(person)
end
```

## Style tip

Pick one style and stay with it in a program, mostly. Mixing is allowed but
your code reads best when it is consistent. Many learners start with the
caveman words and slowly drift to English for longer projects -- that is a
good path!

Next: [Style guide](style-guide.md).