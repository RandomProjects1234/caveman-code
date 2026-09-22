"""Turn CMC source text into tokens.

The lexer is the cave's front door: it walks through the source one squiggle
at a time and hands back friendly little tokens.
"""

from .errors import CmcLexError
from . import words

# Canonical words that belong to the grammar itself.
KEYWORDS = {
    "oga",
    "grunk",
    "binga",
    "wonga",
    "unga",
    "booga",
    "zug",
    "zoop",
    "clump",
    "ork",
    "skrib",
    "blorp",
    "and",
    "or",
    "not",
    "in",
    "ugg",
}

# alias -> canonical keyword (print -> oga, if -> binga, ...)
ALIAS_KEYWORDS = {}
for _alias, _canon in words.ALIASES.items():
    if _canon in KEYWORDS:
        ALIAS_KEYWORDS[_alias] = _canon

# alias -> canonical constant name (true -> gronk, ...)
ALIAS_CONSTANTS = {}
for _alias, _canon in words.ALIASES.items():
    if _canon in ("gronk", "nork", "plop") and _alias != _canon:
        ALIAS_CONSTANTS[_alias] = _canon

COMMENT_WORDS = {"ugg", "comment"}

TWO_CHAR_OPS = {"==", "!=", "<=", ">="}
ONE_CHAR_OPS = {"=", "<", ">", "+", "-", "*", "/", "%"}

PUNCT = {
    "(": "lparen",
    ")": "rparen",
    "[": "lbracket",
    "]": "rbracket",
    ",": "comma",
}

SIMPLE_ESCAPES = {
    "n": "\n",
    "t": "\t",
    "r": "\r",
    "\\": "\\",
    '"': '"',
    "'": "'",
    "0": "\0",
}


class Token:
    __slots__ = ("kind", "value", "line", "raw")

    def __init__(self, kind, value, line, raw=None):
        self.kind = kind
        self.value = value
        self.line = line
        self.raw = raw if raw is not None else str(value)

    def __repr__(self):
        return "Token(" + self.kind + ", " + repr(self.value) + ", line " + str(self.line) + ")"

    def describe(self):
        """How to talk about this token to a child."""
        if self.kind == "nl":
            return "the end of the line"
        if self.kind == "eof":
            return "the end of the program"
        if self.kind == "str":
            return "text " + repr(self.raw)
        return "'" + self.raw + "'"


class Lexer:
    def __init__(self, source, filename="<cmc>"):
        self.source = source
        self.filename = filename
        self.pos = 0
        self.line = 1

    # -- helpers ---------------------------------------------------------

    def _peek(self, ahead=1):
        spot = self.pos + ahead
        if spot < len(self.source):
            return self.source[spot]
        return ""

    def _at_end(self):
        return self.pos >= len(self.source)

    def _error(self, message, hint=None):
        raise CmcLexError(message, line=self.line, hint=hint)

    # -- main loop -------------------------------------------------------

    def tokenize(self):
        tokens = []
        while not self._at_end():
            ch = self.source[self.pos]

            if ch == "\r":
                self.pos += 1
                continue
            if ch == "\n":
                tokens.append(Token("nl", "\n", self.line))
                self.line += 1
                self.pos += 1
                continue
            if ch in " \t":
                self.pos += 1
                continue

            if ch.isalpha() or ch == "_":
                self._read_word(tokens)
                continue

            if ch.isdigit():
                self._read_number(tokens)
                continue

            if ch in "\"'":
                self._read_string(tokens)
                continue

            if ch in PUNCT:
                tokens.append(Token(PUNCT[ch], ch, self.line))
                self.pos += 1
                continue

            two = ch + self._peek()
            if two in TWO_CHAR_OPS:
                tokens.append(Token("op", two, self.line))
                self.pos += 2
                continue

            if ch in ONE_CHAR_OPS:
                tokens.append(Token("op", ch, self.line))
                self.pos += 1
                continue

            if ch == "!":
                self._error(
                    "I found a single '!' but I do not know what it means.",
                    hint="to say 'not equal' write '!='. To flip a check use the word 'not'.",
                )

            if ch in "&|":
                self._error(
                    "I found '" + ch + "' but CMC uses words for that.",
                    hint="use 'and' or 'or' between checks.",
                )

            if ch == ";":
                self._error(
                    "I found a ';'. CMC does not use those.",
                    hint="one thing per line is plenty. Just press Enter.",
                )

            self._error(
                "I do not know the squiggle '" + ch + "'.",
                hint="CMC words are made of letters, numbers, and _",
            )

        if not tokens or tokens[-1].kind != "nl":
            tokens.append(Token("nl", "\n", self.line))
        tokens.append(Token("eof", "", self.line))
        return tokens

    # -- pieces ----------------------------------------------------------

    def _read_word(self, tokens):
        start = self.pos
        while not self._at_end():
            ch = self.source[self.pos]
            if ch.isalnum() or ch == "_":
                self.pos += 1
            else:
                break
        raw = self.source[start:self.pos]
        lower = raw.lower()

        if lower in COMMENT_WORDS:
            self._skip_comment()
            return

        if lower in ALIAS_KEYWORDS:
            tokens.append(Token("kw", ALIAS_KEYWORDS[lower], self.line, raw=lower))
            return

        if lower in ALIAS_CONSTANTS:
            tokens.append(Token("name", ALIAS_CONSTANTS[lower], self.line, raw=lower))
            return

        tokens.append(Token("name", raw, self.line, raw=raw))

    def _skip_comment(self):
        while not self._at_end() and self.source[self.pos] != "\n":
            self.pos += 1

    def _read_number(self, tokens):
        start = self.pos
        seen_dot = False
        while not self._at_end():
            ch = self.source[self.pos]
            if ch.isdigit():
                self.pos += 1
            elif ch == "." and not seen_dot:
                seen_dot = True
                self.pos += 1
            else:
                break
        raw = self.source[start:self.pos]
        if raw.endswith("."):
            self._error(
                "The number " + raw + " is missing the numbers after the dot.",
                hint="write something like 3.5 instead.",
            )
        if self._peek() == ".":
            self._error(
                "There are too many dots in that number.",
                hint="a number can only have one dot, like 3.5",
            )
        value = float(raw) if seen_dot else int(raw)
        tokens.append(Token("num", value, self.line, raw=raw))

    def _read_string(self, tokens):
        quote = self.source[self.pos]
        start_line = self.line
        self.pos += 1
        pieces = []
        while True:
            if self._at_end():
                raise CmcLexError(
                    "Some text started with a " + quote + " but never finished.",
                    line=start_line,
                    hint="add a matching " + quote + " at the end of the text.",
                )
            ch = self.source[self.pos]
            if ch == "\\":
                nxt = self._peek()
                if nxt == "":
                    raise CmcLexError(
                        "The text ends with a lonely backslash.",
                        line=self.line,
                        hint="write \\\\ if you really want a backslash.",
                    )
                if nxt in SIMPLE_ESCAPES:
                    pieces.append(SIMPLE_ESCAPES[nxt])
                    self.pos += 2
                    continue
                if nxt == "u":
                    hexdigits = self.source[self.pos + 2:self.pos + 6]
                    if len(hexdigits) == 4:
                        try:
                            pieces.append(chr(int(hexdigits, 16)))
                            self.pos += 6
                            continue
                        except ValueError:
                            pass
                    raise CmcLexError(
                        "That \\u escape needs four numbers after it, like \\u2764.",
                        line=self.line,
                    )
                pieces.append(nxt)
                self.pos += 2
                continue
            if ch == quote:
                self.pos += 1
                break
            if ch == "\n":
                raise CmcLexError(
                    "Text cannot go over more than one line.",
                    line=start_line,
                    hint="close the text with " + quote + " before pressing Enter.",
                )
            pieces.append(ch)
            self.pos += 1
        tokens.append(Token("str", "".join(pieces), self.line))


def tokenize(source, filename="<cmc>"):
    return Lexer(source, filename).tokenize()