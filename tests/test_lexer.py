import unittest

from tests.context import SRC  # noqa: F401  (sets up the import path)

from cmc.lexer import tokenize
from cmc.errors import CmcLexError


def kinds(source):
    return [tok.kind for tok in tokenize(source)]


def values(source):
    return [tok.value for tok in tokenize(source) if tok.kind not in ("nl", "eof")]


class TestLexer(unittest.TestCase):
    def test_simple_tokens(self):
        self.assertEqual(kinds("oga 5"), ["kw", "num", "nl", "eof"])

    def test_strings_and_punct(self):
        toks = tokenize('snorf("a", 2)')
        self.assertEqual(
            [t.kind for t in toks],
            ["name", "lparen", "str", "comma", "num", "rparen", "nl", "eof"],
        )
        self.assertEqual(toks[2].value, "a")

    def test_operators(self):
        self.assertEqual(values("1 <= 2 != 3 == 4 >= 5"), [1, "<=", 2, "!=", 3, "==", 4, ">=", 5])

    def test_comments_disappear(self):
        toks = tokenize('oga "hi" ugg this is a note\no')
        self.assertEqual([t.value for t in toks if t.kind == "name"], ["o"])

    def test_full_line_comment(self):
        self.assertEqual(kinds("ugg nothing here"), ["nl", "eof"])

    def test_string_escapes(self):
        toks = tokenize(r'"line\nbreak\tand \"quotes\""')
        self.assertEqual(toks[0].value, 'line\nbreak\tand "quotes"')

    def test_single_quotes(self):
        self.assertEqual(tokenize("'hi'")[0].value, "hi")

    def test_alias_print(self):
        toks = tokenize('print "hi"')
        self.assertEqual(toks[0].kind, "kw")
        self.assertEqual(toks[0].value, "oga")

    def test_alias_true_false_nothing(self):
        toks = tokenize("true false nothing")
        self.assertEqual([t.value for t in toks if t.kind == "name"], ["gronk", "nork", "plop"])

    def test_unclosed_string_error(self):
        with self.assertRaises(CmcLexError) as caught:
            tokenize('oga "hello')
        self.assertIn("never finished", caught.exception.message)
        self.assertEqual(caught.exception.line, 1)

    def test_unknown_character_error(self):
        with self.assertRaises(CmcLexError) as caught:
            tokenize("oga $")
        self.assertIn("squiggle", caught.exception.message)

    def test_double_dot_number_error(self):
        with self.assertRaises(CmcLexError):
            tokenize("oga 1.2.3")

    def test_trailing_dot_number(self):
        with self.assertRaises(CmcLexError):
            tokenize("oga 3.")

    def test_line_numbers(self):
        toks = tokenize("oga 1\noga 2\noga 3")
        lines = [t.line for t in toks if t.kind == "kw"]
        self.assertEqual(lines, [1, 2, 3])

    def test_semicolon_hint(self):
        with self.assertRaises(CmcLexError) as caught:
            tokenize("oga 1;")
        self.assertIn("Enter", caught.exception.hint)


if __name__ == "__main__":
    unittest.main()