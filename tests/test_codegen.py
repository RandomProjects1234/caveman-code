import contextlib
import io
import os
import unittest

from tests.context import SRC, EXAMPLES  # noqa: F401

from cmc.interpreter import Interpreter
from cmc.codegen import to_python_source

SKIP_MARKERS = ("no-check", "skrib", "blorp", "munga", "wait(")


def program_files():
    if not os.path.isdir(EXAMPLES):
        return []
    return sorted(f for f in os.listdir(EXAMPLES) if f.endswith(".cmc"))


class TestCodegen(unittest.TestCase):
    def test_compiles_and_matches_for_simple_programs(self):
        for name in program_files():
            path = os.path.join(EXAMPLES, name)
            with open(path, "r", encoding="utf-8") as handle:
                source = handle.read()
            if any(marker in source for marker in SKIP_MARKERS):
                continue

            lines = []
            Interpreter(output=lines.append).run(source, filename=name)
            expected = "\n".join(lines)

            python_source = to_python_source(source, filename=name)
            self.assertIn("def _cmc_say", python_source)
            self.assertTrue(python_source.startswith("# "))

            captured = io.StringIO()
            with contextlib.redirect_stdout(captured):
                exec(compile(python_source, name + ".py", "exec"), {"__name__": "__main__"})
            self.assertEqual(captured.getvalue().strip("\n"), expected, "compiled mismatch for " + name)

    def test_compiled_file_is_plain_python(self):
        source = 'oga "hi"\ngrunk x = 2\noga x + 3'
        python_source = to_python_source(source)
        namespace = {"__name__": "__main__"}
        captured = io.StringIO()
        with contextlib.redirect_stdout(captured):
            exec(compile(python_source, "<test>", "exec"), namespace)
        self.assertEqual(captured.getvalue().splitlines(), ["hi", "5"])
        self.assertEqual(namespace["x"], 2)

    def test_else_if_chain_codegen(self):
        source = (
            "grunk n = 2\n"
            "binga n == 1\n"
            '    oga "one"\n'
            "wonga binga n == 2\n"
            '    oga "two"\n'
            "wonga\n"
            '    oga "other"\n'
            "unga"
        )
        python_source = to_python_source(source)
        self.assertIn("elif", python_source)
        captured = io.StringIO()
        with contextlib.redirect_stdout(captured):
            exec(compile(python_source, "<test>", "exec"), {"__name__": "__main__"})
        self.assertEqual(captured.getvalue().strip(), "two")


if __name__ == "__main__":
    unittest.main()