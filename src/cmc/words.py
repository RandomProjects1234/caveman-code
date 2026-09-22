"""The caveman dictionary.

One place that knows every CMC word, what it means, how it is written, and a
small example.  Used by:

  * the compiler (aliases -> canonical words)
  * OGABOOGA CODER (the dictionary popup)
  * the website builder (one wiki page per word)
"""

VERSION = "0.1.0"

CATEGORIES = [
    "Talking",
    "Boxes",
    "Choices",
    "Logic",
    "Loops",
    "Clumps",
    "Values",
    "Piles",
    "Text",
    "Numbers",
    "Drawing",
    "Notes",
]

SYNTAX_WORDS = [
    {
        "name": "oga",
        "aliases": ["print", "say"],
        "category": "Talking",
        "summary": "Say something on the screen.",
        "syntax": "oga <thing>",
        "extra_syntax": [
            "oga <thing>, <thing>, <thing>",
        ],
        "doc": (
            "`oga` is how a CMC program talks to you. Everything after it gets "
            "printed on one line, with spaces between the things.\n\n"
            "You can `oga` numbers, text, piles, `gronk`, `nork`, `plop`, and "
            "boxes. A box shows what is *inside* it."
        ),
        "example": 'oga "Hello cave!"\n\n'
        'grunk name = "Ugg"\n'
        'oga "Hi", name, "you are", 3, "years old"',
        "tips": [
            "`oga` with nothing after it prints an empty line.",
            "Quotes make text. `oga hi` looks for a box named `hi`; `oga \"hi\"` prints the word.",
        ],
        "related": ["goop", "blorp", "unga"],
    },
    {
        "name": "blorp",
        "aliases": ["ask"],
        "category": "Talking",
        "summary": "Ask the user a question and get their answer as text.",
        "syntax": 'blorp "What is your name?"',
        "extra_syntax": ["blorp()"],
        "doc": (
            "`blorp` shows a question and waits for the person to type an "
            "answer. The answer comes back as text (goop).\n\n"
            "In OGABOOGA CODER a friendly question window pops up. In the "
            "terminal the person types into the console."
        ),
        "example": 'grunk name = blorp "What is your name?"\noga "Ooga", name, "!"',
        "tips": [
            "The answer is always text, even if it looks like a number. Use a number box if you need math.",
            "You can blorp with no question: `grunk answer = blorp()`",
        ],
        "related": ["grunk", "goop", "oga"],
    },
    {
        "name": "grunk",
        "aliases": ["let", "make"],
        "category": "Boxes",
        "summary": "Make a box and put something in it.",
        "syntax": "grunk <name> = <thing>",
        "doc": (
            "A *box* remembers something for later. `grunk` makes a new box.\n\n"
            "Box names are made of letters, numbers, and `_`, and they cannot "
            "start with a number. `grunk x = 5` makes a box named `x` holding 5.\n\n"
            "To put something new into a box that already exists, just write "
            "`x = 6` without `grunk`. That is called *changing* the box."
        ),
        "example": 'grunk age = 4\ngrunk name = "Ugg"\noga name, "is", age\n\n'
        'age = age + 1   ugg a year passed\n'
        'oga name, "is now", age',
        "tips": [
            "Using `grunk` again on the same name makes a fresh box (the old thing is forgotten).",
            "Changing a box that was never made says OOGA! -- make it first.",
        ],
        "related": ["oga", "clump", "plop"],
    },
    {
        "name": "binga",
        "aliases": ["if"],
        "category": "Choices",
        "summary": "Do something only when a check is gronk.",
        "syntax": "binga <check>\n    <things to do>\nunga",
        "extra_syntax": [
            "binga <check>\n    <things>\nwonga\n    <other things>\nunga",
            "binga <check>\n    <things>\nwonga binga <check2>\n    <more>\nunga",
        ],
        "doc": (
            "`binga` asks a yes/no question and only does the things inside "
            "when the answer is `gronk` (true).\n\n"
            "Use `wonga` to say what to do when it is not gronk. Use "
            "`wonga binga` to check something else first, like 'else if'.\n\n"
            "Every `binga` needs its own `unga` at the end."
        ),
        "example": 'grunk apples = 3\n\n'
        "binga apples > 2\n"
        '    oga "That is a lot of apples!"\n'
        "wonga\n"
        '    oga "Just a few apples."\n'
        "unga",
        "tips": [
            "Checks use `==`, `!=`, `<`, `>`, `<=`, `>=`.",
            "You can stick checks together with `and`, `or`, and `not`.",
        ],
        "related": ["wonga", "unga", "gronk", "nork", "and", "or", "not"],
    },
    {
        "name": "wonga",
        "aliases": ["else"],
        "category": "Choices",
        "summary": "What to do when the binga check was not gronk.",
        "syntax": "binga <check>\n    <things>\nwonga\n    <other things>\nunga",
        "extra_syntax": [
            "wonga binga <check2>   ugg check something else",
        ],
        "doc": (
            "`wonga` lives inside a `binga`. The things after it happen only "
            "when the `binga` check was `nork` (false).\n\n"
            "`wonga binga` checks something else before giving up."
        ),
        "example": 'grunk door = "open"\n\n'
        'binga door == "open"\n'
        '    oga "Walk in!"\n'
        "wonga\n"
        '    oga "Knock first."\n'
        "unga",
        "tips": [
            "`wonga` never has its own `unga`; the `unga` belongs to the whole `binga`.",
        ],
        "related": ["binga", "unga"],
    },
    {
        "name": "unga",
        "aliases": ["end", "done", "finish"],
        "category": "Choices",
        "summary": "Close a block of code.",
        "syntax": "unga",
        "doc": (
            "`unga` means 'that block is done'. Every `binga`, `booga`, "
            "`zug`, `zoop`, and `clump` block ends with `unga`.\n\n"
            "Think of it like a cave door closing behind the things you put "
            "inside."
        ),
        "example": "booga 3\n"
        '    oga "unga!"\n'
        "unga",
        "tips": [
            "Blocks can live inside blocks. Each one needs its own `unga`.",
            "An extra `unga` with nothing open makes a friendly OOGA! error.",
        ],
        "related": ["binga", "booga", "zug", "zoop", "clump"],
    },
    {
        "name": "booga",
        "aliases": ["repeat"],
        "category": "Loops",
        "summary": "Do something again and again, a set number of times.",
        "syntax": "booga <count>\n    <things to do>\nunga",
        "doc": (
            "`booga 5` does the things inside five times.\n\n"
            "The count must be a number. `booga 0` does nothing.\n\n"
            "Inside the loop there is a magic box named `lap` that remembers "
            "which lap you are on: 1 for the first lap, 2 for the second, and "
            "so on."
        ),
        "example": 'booga 3\n    oga "BOOGA!"\nunga\n\noga "done"',
        "tips": [
            "Use the magic box `lap` to do different things each time: `oga \"lap\", lap`.",
            "For 'keep going until something changes' use `zug`.",
        ],
        "related": ["zug", "zoop", "unga"],
    },
    {
        "name": "zug",
        "aliases": ["while"],
        "category": "Loops",
        "summary": "Keep doing something while a check stays gronk.",
        "syntax": "zug <check>\n    <things to do>\nunga",
        "doc": (
            "`zug` checks before every lap. If the check is `gronk`, it does "
            "the things inside and checks again.\n\n"
            "When the check becomes `nork`, the loop stops."
        ),
        "example": "grunk fuel = 3\n\n"
        "zug fuel > 0\n"
        '    oga "Zoom! fuel left:", fuel\n'
        "    fuel = fuel - 1\n"
        "unga\n\n"
        'oga "Out of fuel."',
        "tips": [
            "If the check never becomes nork you get the 'ran for a very long time' error.",
            "Change a box inside the loop so the check can finish.",
        ],
        "related": ["booga", "zoop", "unga"],
    },
    {
        "name": "zoop",
        "aliases": ["for"],
        "category": "Loops",
        "summary": "Do something once for each thing in a pile or text.",
        "syntax": "zoop <name> in <pile or text>\n    <things to do>\nunga",
        "doc": (
            "`zoop` walks through a pile (or the letters of a text) one item "
            "at a time. The box you name holds the item for that lap."
        ),
        "example": 'grunk pets = snorf("dog", "cat", "rock")\n\n'
        "zoop pet in pets\n"
        '    oga "I love my", pet\n'
        "unga",
        "tips": [
            "For text, each lap gives you one letter.",
            "The loop box is a normal box: you can oga it, do math with it, and so on.",
        ],
        "related": ["snorf", "booga", "zug"],
    },
    {
        "name": "clump",
        "aliases": ["fn", "function", "def"],
        "category": "Clumps",
        "summary": "Make your own word that does a bunch of things.",
        "syntax": "clump <name>(<boxes>)\n    <things to do>\nunga",
        "extra_syntax": [
            "clump <name>()          ugg no boxes",
            "clump <name>(a, b, c)   ugg many boxes",
        ],
        "doc": (
            "A `clump` is a bunch of code with a name. Run it any time by "
            "writing its name with parentheses.\n\n"
            "The names in parentheses become boxes filled with whatever you "
            "pass in. Use `ork` to give an answer back."
        ),
        "example": 'clump yell(word)\n'
        '    oga shout(word), "!!!"\n'
        "unga\n\n"
        'yell("hello")\n'
        'yell("booga")',
        "tips": [
            "Make a clump before you use it (the computer reads top to bottom).",
            "A clump with an `ork` gives something back: `grunk x = double(4)`",
            "A clump that uses itself is called recursion -- powerful and fun.",
        ],
        "related": ["ork", "unga", "grunk"],
    },
    {
        "name": "ork",
        "aliases": ["return", "give"],
        "category": "Clumps",
        "summary": "Give an answer back from a clump.",
        "syntax": "ork <thing>",
        "doc": (
            "`ork` hands a value back to whoever called the clump and stops "
            "the clump right there.\n\n"
            "A clump without `ork` gives back `plop` (nothing)."
        ),
        "example": "clump double(n)\n"
        "    ork n * 2\n"
        "unga\n\n"
        "oga double(21)",
        "tips": [
            "`ork` with nothing after it gives back `plop`.",
            "`ork` outside of a clump is an OOGA! error.",
        ],
        "related": ["clump", "plop"],
    },
    {
        "name": "skrib",
        "aliases": ["draw"],
        "category": "Drawing",
        "summary": "Draw on the picture window (OGABOOGA CODER's Draw tab).",
        "syntax": "skrib circle <x>, <y>, <r>",
        "extra_syntax": [
            "skrib dot <x>, <y>, <r>      ugg filled circle",
            "skrib line <x1>, <y1>, <x2>, <y2>",
            "skrib box <x>, <y>, <w>, <h> ugg outline rectangle",
            "skrib blob <x>, <y>, <w>, <h> ugg filled rectangle",
            'skrib write "words", <x>, <y>',
            'skrib color "red"            ugg names or "#ff8800"',
            "skrib size <w>, <h>          ugg picture size",
            "skrib clear                  ugg wipe the picture",
        ],
        "doc": (
            "`skrib` (caveman for *scribble*) draws on the picture. The spot "
            "`0, 0` is the top-left corner. `x` goes right, `y` goes down.\n\n"
            "Colors can be names like `\"red\"`, `\"blue\"`, `\"gold\"` or "
            "hex colors like `\"#ff8800\"`."
        ),
        "example": 'skrib size 400, 400\n'
        'skrib color "orange"\n'
        "skrib circle 200, 200, 100\n"
        'skrib color "black"\n'
        "skrib dot 200, 200, 10\n"
        'skrib write "SUN!", 175, 320',
        "tips": [
            "The picture window opens by itself the first time you draw.",
            "Use `wait(ms)` between drawings to make a simple animation.",
            "In the terminal build, close the picture window when the program is done.",
        ],
        "related": ["wait", "booga", "munga"],
    },
    {
        "name": "ugg",
        "aliases": ["comment"],
        "category": "Notes",
        "summary": "A note for humans. The computer ignores it.",
        "syntax": "ugg <anything at all>",
        "doc": (
            "`ugg` starts a note (comment). Everything after it on that line "
            "is ignored by the computer.\n\n"
            "Notes are how you explain your code to future-you."
        ),
        "example": 'ugg this whole line is a note\noga "hi"   ugg this note comes after code',
        "tips": ["Notes do not slow the program down at all."],
        "related": [],
    },
    {
        "name": "and",
        "aliases": [],
        "category": "Logic",
        "summary": "Gronk only when both sides are gronk.",
        "syntax": "<check> and <check>",
        "doc": (
            "`and` joins two checks. The answer is `gronk` only when both "
            "sides are gronk."
        ),
        "example": 'grunk age = 7\n\n'
        "binga age > 4 and age < 10\n"
        '    oga "You are a big kid!"\n'
        "unga",
        "tips": ["`and` stops early if the left side is nork."],
        "related": ["or", "not", "binga"],
    },
    {
        "name": "or",
        "aliases": [],
        "category": "Logic",
        "summary": "Gronk when at least one side is gronk.",
        "syntax": "<check> or <check>",
        "doc": "`or` joins two checks. The answer is `gronk` when either side is gronk.",
        "example": 'grunk day = "sunday"\n\n'
        'binga day == "saturday" or day == "sunday"\n'
        '    oga "No school!"\n'
        "unga",
        "tips": ["`or` stops early if the left side is gronk."],
        "related": ["and", "not", "binga"],
    },
    {
        "name": "not",
        "aliases": [],
        "category": "Logic",
        "summary": "Flip gronk to nork and nork to gronk.",
        "syntax": "not <check>",
        "doc": "`not` flips a check. `not gronk` is `nork`.",
        "example": "grunk hungry = nork\n\n"
        "binga not hungry\n"
        '    oga "Then let us go play!"\n'
        "unga",
        "tips": ["Double flip: `not not gronk` is `gronk` again."],
        "related": ["and", "or", "gronk", "nork"],
    },
    {
        "name": "in",
        "aliases": [],
        "category": "Loops",
        "summary": "Am I inside this pile / text?  Also used by zoop.",
        "syntax": "<thing> in <pile or text>",
        "extra_syntax": ["zoop <name> in <pile> ... unga"],
        "doc": (
            "`in` asks: does this pile contain that thing? For text it asks "
            "about smaller pieces of text.\n\n"
            "`in` is also the little word inside every `zoop` loop."
        ),
        "example": 'grunk pets = snorf("dog", "cat")\n\n'
        "binga \"cat\" in pets\n"
        '    oga "There is a cat in the pile!"\n'
        "unga",
        "tips": ["`not in` works too: `binga \"fish\" not in pets`"],
        "related": ["zoop", "snorf"],
    },
    {
        "name": "gronk",
        "aliases": ["true"],
        "category": "Values",
        "summary": "The truth value for YES.",
        "syntax": "gronk",
        "doc": (
            "`gronk` means yes/on/true. It is what checks give back when "
            "they are happy."
        ),
        "example": "grunk light_on = gronk\noga light_on   ugg prints gronk",
        "tips": ["`oga gronk` prints the word gronk."],
        "related": ["nork", "binga"],
    },
    {
        "name": "nork",
        "aliases": ["false"],
        "category": "Values",
        "summary": "The truth value for NO.",
        "syntax": "nork",
        "doc": "`nork` means no/off/false.",
        "example": "grunk light_on = nork\nbinga not light_on\n"
        '    oga "It is dark in here."\n'
        "unga",
        "tips": ["Numbers can be truthy too: 0 is like nork, other numbers are like gronk."],
        "related": ["gronk", "binga"],
    },
    {
        "name": "plop",
        "aliases": ["nothing", "none"],
        "category": "Values",
        "summary": "Nothing at all. An empty spot.",
        "syntax": "plop",
        "doc": (
            "`plop` is the value of nothing. A clump without `ork` gives back "
            "`plop`. A fresh empty box holds `plop`.\n\n"
            "There is also the pile helper `plop(pile, thing)` -- see its page."
        ),
        "example": "grunk treasure = plop\noga treasure   ugg prints: plop",
        "tips": ["`plop` is a fine way to say 'no answer yet'."],
        "related": ["ork", "grunk", "plop_call"],
    },
    {
        "name": "pi",
        "aliases": [],
        "category": "Values",
        "summary": "The circle number, about 3.14159.",
        "syntax": "pi",
        "doc": "`pi` is the magic number of circles. It is already there, ready to use.",
        "example": 'oga "A circle with radius 2 has around area", pi * 2 * 2',
        "tips": ["It is really 3.141592653589793 -- CMC remembers the rest."],
        "related": ["root", "round"],
    },
]

BUILTIN_WORDS = [
    {
        "name": "snorf",
        "category": "Piles",
        "summary": "Make a pile of things.",
        "syntax": "snorf(<thing>, <thing>, ...)",
        "arity": (0, 99),
        "doc": (
            "`snorf` builds a *pile* (a list). Piles keep things in order and "
            "can grow and shrink.\n\n"
            "A pile can hold any mix of things: numbers, text, even other piles."
        ),
        "example": 'grunk pets = snorf("dog", "cat", "rock")\n'
        "oga pets\n"
        "oga nom(pets)   ugg how many",
        "tips": [
            "`snorf()` makes an empty pile.",
            "Spot numbers start at 0: the first thing is `pets[0]`.",
        ],
        "related": ["nom", "skoop", "yoink", "plop_call", "zoop"],
    },
    {
        "name": "plop_call",
        "display": "plop",
        "category": "Piles",
        "summary": "Plop a thing onto the end of a pile.",
        "syntax": "plop(<pile>, <thing>)",
        "arity": (2, 2),
        "doc": (
            "`plop(pile, thing)` adds a thing to the end of a pile.\n\n"
            "The same word `plop` by itself means *nothing* -- this is the "
            "doing version."
        ),
        "example": 'grunk bag = snorf()\n'
        'plop(bag, "apple")\n'
        'plop(bag, "rock")\n'
        "oga bag",
        "tips": ["A pile grows as big as you like. Keep plopping!"],
        "related": ["snorf", "yoink", "nom"],
    },
    {
        "name": "nom",
        "category": "Piles",
        "summary": "How many things? (length of a pile or text)",
        "syntax": "nom(<pile or text>)",
        "arity": (1, 1),
        "doc": (
            "`nom` counts. For a pile it counts the things. For text it "
            "counts the letters."
        ),
        "example": 'grunk word = "cave"\noga nom(word)      ugg 4\n\n'
        'grunk xs = snorf(1, 2, 3)\noga nom(xs)        ugg 3',
        "tips": ["Empty pile: nom is 0. Empty text \"\": nom is 0."],
        "related": ["snorf", "skoop", "goop"],
    },
    {
        "name": "skoop",
        "category": "Piles",
        "summary": "Scoop out one thing from a pile or text by its spot number.",
        "syntax": "skoop(<pile or text>, <spot>)",
        "arity": (2, 2),
        "doc": (
            "`skoop(pile, 2)` gives you the thing at spot 2. Spots start at 0.\n\n"
            "`pile[2]` does exactly the same thing. Negative spots count from "
            "the end: `-1` is the last thing."
        ),
        "example": 'grunk pets = snorf("dog", "cat", "fish")\n'
        "oga skoop(pets, 0)   ugg dog\n"
        "oga pets[2]          ugg fish\n"
        "oga pets[-1]         ugg fish too",
        "tips": ["Asking for a spot that is not there gives an OOGA! error."],
        "related": ["snorf", "nom", "yoink"],
    },
    {
        "name": "yoink",
        "category": "Piles",
        "summary": "Take the last thing off a pile.",
        "syntax": "yoink(<pile>)",
        "arity": (1, 1),
        "doc": (
            "`yoink(pile)` removes the last thing and hands it back to you, "
            "like pulling the top rock off a stack."
        ),
        "example": 'grunk stack = snorf(1, 2, 3)\n'
        "grunk top = yoink(stack)\n"
        'oga "I yoinked", top\n'
        "oga stack   ugg [1, 2]",
        "tips": ["Yoinking an empty pile gives a friendly OOGA! error."],
        "related": ["plop_call", "snorf", "nom"],
    },
    {
        "name": "goop",
        "category": "Text",
        "summary": "Squish anything into text.",
        "syntax": "goop(<thing>)",
        "arity": (1, 1),
        "doc": (
            "`goop` turns a number, pile, truth, or plop into text (a string).\n\n"
            "This is how you stick numbers and words together: `\"I am \" + goop(7)`."
        ),
        "example": 'grunk age = 7\n'
        'oga "I am " + goop(age) + " years old"',
        "tips": ["`oga` already goops things for you. `goop` is for joining."],
        "related": ["oga", "shout", "whisper", "split", "join"],
    },
    {
        "name": "shout",
        "category": "Text",
        "summary": "Make text ALL CAPS.",
        "syntax": "shout(<text>)",
        "arity": (1, 1),
        "doc": "`shout(text)` turns every letter big: `shout(\"hi\")` is `\"HI\"`.",
        "example": 'oga shout("hello cave")',
        "tips": ["Alien caves: `shout` only knows the letters it knows."],
        "related": ["whisper", "goop"],
    },
    {
        "name": "whisper",
        "category": "Text",
        "summary": "Make text all small letters.",
        "syntax": "whisper(<text>)",
        "arity": (1, 1),
        "doc": "`whisper(text)` turns every letter small.",
        "example": 'oga whisper("SHHH VERY LOUD")',
        "tips": ["Shout then whisper to make text calm again."],
        "related": ["shout", "goop"],
    },
    {
        "name": "flip",
        "category": "Text",
        "summary": "Turn text backwards.",
        "syntax": "flip(<text>)",
        "arity": (1, 1),
        "doc": "`flip(text)` turns the text around: `flip(\"cave\")` is `\"evac\"`.",
        "example": 'oga flip("booga")   ugg agoob',
        "tips": ["flip works on any text, even one letter."],
        "related": ["goop", "nom"],
    },
    {
        "name": "find",
        "category": "Text",
        "summary": "Where does a smaller text start inside a bigger text?",
        "syntax": "find(<text>, <piece>)",
        "arity": (2, 2),
        "doc": (
            "`find(text, piece)` gives the spot number where `piece` starts "
            "inside `text`, or `-1` when it is not there at all."
        ),
        "example": 'grunk word = "caveman"\n'
        'oga find(word, "man")   ugg 4\n'
        'oga find(word, "z")     ugg -1',
        "tips": ["Spots start at 0, so the first letter is spot 0."],
        "related": ["skoop", "split", "nom"],
    },
    {
        "name": "split",
        "category": "Text",
        "summary": "Chop text into a pile of pieces.",
        "syntax": "split(<text>, <separator>)",
        "arity": (2, 2),
        "doc": (
            "`split(text, \",\")` cuts text wherever the separator is and "
            "gives back a pile of the pieces."
        ),
        "example": 'grunk words = split("rock,paper,scissors", ",")\n'
        "oga words\n"
        "oga nom(words)",
        "tips": ['Split on a space to get words: split(sentence, " ")'],
        "related": ["join", "snorf", "nom"],
    },
    {
        "name": "join",
        "category": "Text",
        "summary": "Stick a pile of things together into one text.",
        "syntax": "join(<pile>, <separator>)",
        "arity": (2, 2),
        "doc": (
            "`join(pile, \"-\")` turns a pile into one text, putting the "
            "separator between the items."
        ),
        "example": 'grunk xs = snorf("a", "b", "c")\n'
        'oga join(xs, " + ")   ugg a + b + c',
        "tips": ["Numbers get gooped automatically."],
        "related": ["split", "snorf", "goop"],
    },
    {
        "name": "what",
        "category": "Text",
        "summary": "Ask what kind of thing something is.",
        "syntax": "what(<thing>)",
        "arity": (1, 1),
        "doc": (
            "`what` gives back a word: `\"number\"`, `\"text\"`, `\"truth\"`, "
            "`\"pile\"`, or `\"plop\"`."
        ),
        "example": 'oga what(3)        ugg number\n'
        'oga what("hi")     ugg text\n'
        "oga what(snorf())  ugg pile",
        "tips": ["Telling apart numbers and text is a very useful trick."],
        "related": ["goop", "nom"],
    },
    {
        "name": "munga",
        "category": "Numbers",
        "summary": "Magic random number between two numbers.",
        "syntax": "munga(<low>, <high>)",
        "arity": (2, 2),
        "doc": (
            "`munga(1, 6)` gives a random whole number from 1 to 6 -- both "
            "ends included. Perfect for dice and games.\n\n"
            "`munga(1, 1)` is always 1, which is handy for testing."
        ),
        "example": "grunk dice = munga(1, 6)\n"
        'oga "You rolled a", dice',
        "tips": ["Run the program again for a different number."],
        "related": ["round", "small", "big", "skrib"],
    },
    {
        "name": "numba",
        "category": "Numbers",
        "summary": "Turn text into a number.",
        "syntax": "numba(<text>)",
        "arity": (1, 1),
        "doc": (
            "`blorp` always gives back text, even when someone types `7`. "
            '`numba("7")` turns that text into a real number so math works.\n\n'
            "It also cleans up extra spaces people type."
        ),
        "example": 'grunk answer = blorp "Pick a number!"\n'
        "grunk n = numba(answer)\n"
        'oga n, "plus one is", n + 1',
        "tips": [
            "If the text is not a number, numba says OOGA! with a kind hint.",
            "A number passed to numba comes back unchanged.",
        ],
        "related": ["blorp", "goop", "what", "round"],
    },
    {
        "name": "round",
        "category": "Numbers",
        "summary": "Make a decimal into the closest whole number.",
        "syntax": "round(<number>)",
        "arity": (1, 1),
        "doc": "`round(2.6)` is 3. `round(2.4)` is 2.",
        "example": "oga round(2.6)\noga round(2.4)",
        "tips": ["`flat` always goes down, `roof` always goes up."],
        "related": ["flat", "roof"],
    },
    {
        "name": "flat",
        "category": "Numbers",
        "summary": "Smash a decimal down to the whole number below it.",
        "syntax": "flat(<number>)",
        "arity": (1, 1),
        "doc": "`flat(2.9)` is 2. It always goes down, like water.",
        "example": "oga flat(2.9)\noga flat(2.0)",
        "tips": ["flat is also called 'floor' in other languages."],
        "related": ["round", "roof"],
    },
    {
        "name": "roof",
        "category": "Numbers",
        "summary": "Lift a decimal up to the whole number above it.",
        "syntax": "roof(<number>)",
        "arity": (1, 1),
        "doc": "`roof(2.1)` is 3. It always goes up, like a roof.",
        "example": "oga roof(2.1)\noga roof(2.0)",
        "tips": ["roof is also called 'ceil' in other languages."],
        "related": ["round", "flat"],
    },
    {
        "name": "abs",
        "category": "Numbers",
        "summary": "How far from zero? (make negative numbers positive)",
        "syntax": "abs(<number>)",
        "arity": (1, 1),
        "doc": "`abs(-7)` is 7. `abs(7)` is 7. It measures distance from zero.",
        "example": "oga abs(-7)\noga abs(3 - 10)",
        "tips": ["Great for finding how far apart two things are."],
        "related": ["small", "big"],
    },
    {
        "name": "small",
        "category": "Numbers",
        "summary": "The smaller of two numbers.",
        "syntax": "small(<a>, <b>)",
        "arity": (2, 2),
        "doc": "`small(3, 9)` is 3.",
        "example": "oga small(3, 9)",
        "tips": ["The opposite twin is `big`."],
        "related": ["big", "abs"],
    },
    {
        "name": "big",
        "category": "Numbers",
        "summary": "The bigger of two numbers.",
        "syntax": "big(<a>, <b>)",
        "arity": (2, 2),
        "doc": "`big(3, 9)` is 9.",
        "example": "oga big(3, 9)",
        "tips": ["Chain them: big(big(1, 5), 3) is 5."],
        "related": ["small", "abs"],
    },
    {
        "name": "root",
        "category": "Numbers",
        "summary": "The square root: what number times itself makes this?",
        "syntax": "root(<number>)",
        "arity": (1, 1),
        "doc": "`root(9)` is 3. Negative numbers have no normal root, so CMC says OOGA!",
        "example": "oga root(9)\noga root(2)",
        "tips": ["root(2) is a wiggly decimal -- `round` it if you like."],
        "related": ["pow", "round", "pi"],
    },
    {
        "name": "pow",
        "category": "Numbers",
        "summary": "A number times itself many times.",
        "syntax": "pow(<number>, <times>)",
        "arity": (2, 2),
        "doc": "`pow(2, 3)` is 2 * 2 * 2 = 8.",
        "example": "oga pow(2, 3)\noga pow(10, 2)",
        "tips": ["pow(x, 2) is x squared."],
        "related": ["root"],
    },
    {
        "name": "wait",
        "category": "Drawing",
        "summary": "Pause the program for some milliseconds.",
        "syntax": "wait(<milliseconds>)",
        "arity": (1, 1),
        "doc": (
            "`wait(1000)` pauses for one second. 1000 milliseconds = 1 second.\n\n"
            "Waiting between drawings makes animation: draw, wait, clear, "
            "draw again."
        ),
        "example": 'skrib color "red"\n'
        "skrib dot 50, 50, 10\n"
        "wait(500)\n"
        "skrib clear\n"
        'skrib color "blue"\n'
        "skrib dot 100, 50, 10",
        "tips": ["The Stop button still works while waiting."],
        "related": ["skrib", "booga"],
    },
]

# ---------------------------------------------------------------------------

_ALIAS_BUILD = {"true": "gronk", "false": "nork", "nothing": "plop"}


def _build_aliases():
    aliases = dict(_ALIAS_BUILD)
    for word in SYNTAX_WORDS + BUILTIN_WORDS:
        aliases[word["name"].lower()] = word["name"]
        if word["name"] == "plop_call":
            aliases["plop_call"] = "plop_call"
            continue
        for alias in word.get("aliases", []):
            aliases[alias.lower()] = word["name"]
    # plop as a *call* is resolved by the parser, not the lexer, so make sure
    # the plain name maps to itself.
    aliases["plop"] = "plop"
    return aliases


ALIASES = _build_aliases()

# Canonical syntax words the parser knows about.
SYNTAX_NAMES = [w["name"] for w in SYNTAX_WORDS if w["name"] != "plop"]
RESERVED = set(SYNTAX_NAMES) | {"and", "or", "not", "in", "ugg"}

BUILTIN_NAMES = [w["name"] for w in BUILTIN_WORDS]

WORD_PAGES = {}
for _w in SYNTAX_WORDS + BUILTIN_WORDS:
    WORD_PAGES[_w["name"]] = _w


def word_info(name):
    return WORD_PAGES.get(name)


def all_words():
    return list(SYNTAX_WORDS) + list(BUILTIN_WORDS)


def alias_of(name):
    """Return the English aliases for a canonical word."""
    info = WORD_PAGES.get(name)
    if not info:
        return []
    return list(info.get("aliases", []))


def find_alias(english_word):
    """Return the CMC word for an English word, or None."""
    return ALIASES.get(english_word.lower())