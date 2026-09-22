# Text (goop)

Text is anything inside quotes. `goop` is the word that squishes other kinds
of things into text.

```cmc
grunk name = "Ugg"
oga "Hello " + name
oga "I have " + goop(7) + " rocks"
```

## Making text

| Write | You get |
| --- | --- |
| `"hello"` | hello |
| `'hello'` | hello (single quotes work too) |
| `"line\nbreak"` | a new line inside the text |
| `""` | empty text |
| `goop(42)` | "42" |
| `goop(snorf(1, 2))` | "[1, 2]" |
| `join(snorf(1, 2), "-")` | "1-2" |
| `shout("hi")` | "HI" |
| `whisper("HI")` | "hi" |
| `flip("cave")` | "evac" |

## Looking inside text

```cmc
grunk word = "caveman"

oga nom(word)          ugg 7, how many letters
oga word[0]            ugg c
oga word[-1]           ugg n
oga find(word, "man")  ugg 4
oga find(word, "z")    ugg -1 (not there)
oga "man" in word      ugg gronk
oga "cave" in word     ugg gronk
```

Text spots work like pile spots: spot 0 is the first letter, `-1` the last.
You **cannot** change a letter in place (`word[0] = "C"` is an error) because
text is not a pile. Build new text instead:

```cmc
grunk word = "caveman"
word = whisper(word)      ugg "caveman"
word = shout(word[0]) + word[1:]
```

Oops -- CMC has no slicing (`word[1:]`)! Use `skoop` in a loop or build a pile
of letters and `join` it:

```cmc
clump capitalize(text)
    grunk first = shout(skoop(text, 0))
    grunk rest = ""
    grunk i = 1
    zug i < nom(text)
        rest = rest + skoop(text, i)
        i = i + 1
    unga
    ork first + rest
unga

oga capitalize("caveman")     ugg Caveman
```

## Chopping and gluing

```cmc
grunk words = split("rock,paper,scissors", ",")
oga words                ugg [rock, paper, scissors]
oga nom(words)           ugg 3

grunk sentence = join(words, " and ")
oga sentence             ugg rock and paper and scissors
```

`split` always gives a pile; `join` always gives text. Numbers inside a pile
are automatically gooped by `join`.

## Comparing text

```cmc
oga "apple" == "apple"       ugg gronk
oga "apple" == "Apple"       ugg nork (capital letters matter)
oga "apple" < "banana"       ugg gronk (alphabetical)
```

To compare without caring about big/small letters, whisper both sides:

```cmc
grunk a = whisper(blorp "type a word")
grunk b = whisper(blorp "type another")
binga a == b
    oga "same word!"
unga
```

## Ideas

- A word counter: `oga nom(split(sentence, " "))`
- A shouting machine: `oga shout(blorp "what should I shout?")`
- A backwards poem: `oga flip("roses are red")`

Next: [Numbers and random](math-and-random.md).