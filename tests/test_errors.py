import unittest

from tests.context import SRC  # noqa: F401

from cmc.interpreter import Interpreter
from cmc.errors import CmcError


def error_of(source, **kwargs):
    lines = []
    interp = Interpreter(output=lines.append, **kwargs)
    try:
        interp.run(source)
    except CmcError as caught:
        return caught
    raise AssertionError("no error for: " + source)


class TestErrorFormat(unittest.TestCase):
    def test_line_and_ooga(self):
        caught = error_of("oga x")
        text = caught.format()
        self.assertTrue(text.startswith("OOGA!"))
        self.assertIn("Line 1", text)
        self.assertIn("Hint:", text)

    def test_every_error_has_a_hint(self):
        cases = [
            "unga",
            "oga 1 +",
            'oga "a" + 1',
            "oga 1 / 0",
            "grunk xs = snorf(1)\noga xs[9]",
            "grunk xs = snorf()\noga yoink(xs)",
            "zoop x in 3\nunga",
            "no_clump()",
            "oga nope",
            "grunk x = 5\nx()",
            "clump f(a)\n    ork a\nunga\nf()",
            "ork 1",
        ]
        for source in cases:
            caught = error_of(source)
            self.assertIsNotNone(caught.hint, "missing hint for: " + source)
            self.assertTrue(caught.hint.strip(), "empty hint for: " + source)

    def test_kid_friendly_wording(self):
        caught = error_of("oga 1 / 0")
        self.assertIn("share by zero", caught.format())


if __name__ == "__main__":
    unittest.main()