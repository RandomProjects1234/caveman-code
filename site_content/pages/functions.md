# Clumps (functions)

A `clump` is a piece of code with a name, so you can use it again and again.

```cmc
clump greet(name)
    oga "Hello " + name + "!"
unga

greet("Ugg")
greet("Oona")
greet("Grog")
```

## The parts

```cmc
clump double(n)          ugg name is double, one box called n
    ork n * 2            ugg give the answer back
unga                     ugg the clump ends here

oga double(21)           ugg use it: prints 42
```

- The names in parentheses are the clump's own boxes, called **boxes** or
  **parameters**.
- `ork` hands a value back and stops the clump right there.
- A clump without `ork` gives back `plop`.
- Make the clump **before** you call it. The computer reads top to bottom.

## Calling clumps

```cmc
clump add(a, b)
    ork a + b
unga

oga add(2, 3)                    ugg 5
oga add(add(1, 1), add(3, 4))    ugg 9
```

You can also call a clump without using its answer, if the clump does
something by itself:

```cmc
clump cheer()
    oga "BOOGA BOOGA BOOGA"
unga

cheer()
```

## Private boxes

Boxes made inside a clump are private. Changing them does not touch outside
boxes unless you change an outside box's value in place (like adding to a
pile):

```cmc
grunk total = 0

clump add_to_total(n)
    total = total + n        ugg changes the OUTSIDE box
unga

clump makes_own(n)
    grunk total = 100        ugg makes a NEW private box
unga

add_to_total(5)
makes_own(999)
oga total                    ugg 5
```

## Recursion: a clump that calls itself

This is the famous Fibonacci sequence:

```cmc
clump fib(n)
    binga n < 2
        ork n
    unga
    ork fib(n - 1) + fib(n - 2)
unga

oga fib(10)      ugg 55
```

Every recursion needs a way to stop. Here, the `binga n < 2` check is the
stopping point. Without it, CMC catches the runaway and says:

```text
OOGA! 'fib' called itself too many times and went too deep.
Hint: a clump can call itself, but it needs a way to stop. Use 'binga' to check.
```

The limit is about 200 levels deep.

## Mistakes CMC catches for you

| Mistake | What CMC says |
| --- | --- |
| calling with the wrong number of things | `'add' wants 2 things, but got 3.` |
| calling a box | `'x' is a box, not a clump, so it cannot be called with ( ).` |
| calling a name that does not exist | `There is no clump named 'gret'.` |
| `ork` outside a clump | `'ork' can only live inside a clump.` |

## A worked example

```cmc
clump temperature_name(celsius)
    binga celsius >= 30
        ork "hot"
    wonga binga celsius >= 15
        ork "nice"
    wonga
        ork "cold"
    unga
unga

zoop c in snorf(0, 20, 35)
    oga c, "degrees is", temperature_name(c)
unga
```

```text
0 degrees is cold
20 degrees is nice
35 degrees is hot
```

Next: [Piles](lists.md).