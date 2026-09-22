import os
import unittest

from tests.context import SRC, EXAMPLES  # noqa: F401

from cmc.interpreter import Interpreter
from cmc.errors import CmcError


def program_files():
    if not os.path.isdir(EXAMPLES):
        return []
    return sorted(f for f in os.listdir(EXAMPLES) if f.endswith(".cmc"))


class TestExamples(unittest.TestCase):
    def test_examples_exist(self):
        self.assertGreaterEqual(len(program_files()), 25)

    def test_examples_run(self):
        for name in program_files():
            path = os.path.join(EXAMPLES, name)
            with open(path, "r", encoding="utf-8") as handle:
                source = handle.read()
            if "no-check" in source:
                continue

            in_path = os.path.splitext(path)[0] + ".in"
            answers = []
            if os.path.exists(in_path):
                with open(in_path, "r", encoding="utf-8") as handle:
                    answers = handle.read().splitlines()
            answer_iter = iter(answers)

            def input_func(prompt, line, _answers=answer_iter):
                try:
                    return next(_answers)
                except StopIteration:
                    return ""

            lines = []
            interp = Interpreter(output=lines.append, input_func=input_func, draw=None)
            try:
                interp.run(source, filename=name)
            except CmcError as error:
                self.fail(name + " failed: " + error.format())

            out_path = os.path.splitext(path)[0] + ".out"
            self.assertTrue(os.path.exists(out_path), "missing .out for " + name)
            with open(out_path, "r", encoding="utf-8") as handle:
                expected = handle.read()
            actual = "\n".join(lines)
            self.assertEqual(actual.strip("\n"), expected.strip("\n"), "wrong output for " + name)


if __name__ == "__main__":
    unittest.main()