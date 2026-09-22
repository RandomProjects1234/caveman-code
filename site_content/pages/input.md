# Asking questions (blorp)

`blorp` shows a question and waits for the person to answer. The answer comes
back as **text**.

```cmc
grunk name = blorp "What is your name?"
oga "Hello " + name + "!"
```

In OGABOOGA CODER, a friendly question window pops up. In the terminal build,
the person types into the console.

## The answer is text

Even if they type `7`, you get the text `"7"`. Use `numba` for math:

```cmc
grunk answer = blorp "Pick a number"
grunk n = numba(answer)
oga n, "plus one is", n + 1
```

## Kinds of questions

```cmc
grunk name = blorp "What is your name?"
grunk color = blorp "Favorite color?"
grunk word = blorp "Give me a word and I will shout it"
oga shout(word)
```

No question at all:

```cmc
grunk anything = blorp()
```

## A conversation

```cmc
oga "Ooga! Me CMC. Who you?"

grunk name = blorp "Your name?"
grunk age = numba(blorp "How many winters old?")

oga "Hello " + name + "."
oga "You are", age, "winters old."

binga age > 100
    oga "You are older than the cave!"
wonga
    oga "You have many winters ahead."
unga
```

## Guessing game (the classic)

```cmc
grunk secret = munga(1, 10)
grunk guess = 0
grunk tries = 0

oga "Me think of number from 1 to 10!"

zug guess != secret
    guess = numba(blorp "Your guess?")
    tries = tries + 1
    binga guess < secret
        oga "Bigger!"
    wonga binga guess > secret
        oga "Smaller!"
    unga
unga

oga "Yes! You got it in", tries, "tries."
```

Notice `guess` starts at 0 so the `zug` check runs at least once. If `secret`
was 0 the loop would not run at all -- a 1-in-10 chance! Can you fix that?
(Hint: make `guess` start at `plop` and check `guess != secret or tries == 0`.)

## Ideas

- A silly interview machine
- A fake pet that asks what it should eat
- A story where the player picks the hero's name

Next: [Drawing](drawing.md).