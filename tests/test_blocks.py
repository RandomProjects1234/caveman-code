import os
import unittest

from tests.context import SRC, EXAMPLES  # noqa: F401

from cmc.interpreter import Interpreter
from cmc.errors import CmcError

from ogabooga import blocks as B


def program_files():
    if not os.path.isdir(EXAMPLES):
        return []
    return sorted(f for f in os.listdir(EXAMPLES) if f.endswith(".cmc"))


def run_source(source, answers=None):
    answer_iter = iter(answers or [])

    def input_func(prompt, line):
        try:
            return next(answer_iter)
        except StopIteration:
            return ""

    lines = []
    Interpreter(output=lines.append, input_func=input_func).run(source)
    return lines


class TestBlockRoundTrip(unittest.TestCase):
    def test_every_example_becomes_blocks_and_back(self):
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

            try:
                loaded = B.source_to_blocks(source)
                generated = B.blocks_to_source(loaded)
            except B.BlockError as error:
                self.fail(name + " could not become blocks: " + str(error))

            expected = run_source(source, answers)
            actual = run_source(generated, answers)
            self.assertEqual(actual, expected, "block round trip changed " + name)

    def test_parse_errors_are_friendly(self):
        with self.assertRaises(B.BlockError) as caught:
            B.source_to_blocks("binga oops")
        self.assertIn("OOGA", str(caught.exception))


class TestBlockCodegen(unittest.TestCase):
    def test_if_else_chain(self):
        source = (
            "binga a == 1\n"
            '    oga "one"\n'
            "wonga binga a == 2\n"
            '    oga "two"\n'
            "wonga\n"
            '    oga "many"\n'
            "unga"
        )
        generated = B.blocks_to_source(B.source_to_blocks(source))
        self.assertIn("wonga binga a == 2", generated)
        self.assertEqual(generated.count("unga"), 1)

    def test_nested_containers(self):
        source = "booga 2\n    binga gronk\n        oga lap\n    unga\nunga"
        generated = B.blocks_to_source(B.source_to_blocks(source))
        lines = [line for line in generated.splitlines() if line.strip()]
        self.assertEqual(lines[0], "booga 2")
        self.assertEqual(lines[1], "    binga gronk")
        self.assertEqual(lines[2], "        oga lap")
        self.assertEqual(lines[3], "    unga")
        self.assertEqual(lines[4], "unga")

    def test_ask_without_prompt(self):
        loaded = B.source_to_blocks("grunk n = blorp()")
        self.assertEqual(B.blocks_to_source(loaded).strip(), "grunk n = blorp()")

    def test_value_expression(self):
        text_block = B.new_block("value_text")
        text_block.fields["value"] = "hi there"
        self.assertEqual(B.value_expression(text_block), '"hi there"')
        number_block = B.new_block("value_number")
        self.assertEqual(B.value_expression(number_block), "1")
        random_block = B.new_block("value_random")
        self.assertEqual(B.value_expression(random_block), "munga(1, 6)")
        var_block = B.new_block("value_var")
        var_block.fields["name"] = "x"
        self.assertEqual(B.value_expression(var_block), "x")

    def test_check_blocks_finds_bad_names(self):
        block = B.new_block("grunk")
        block.fields["name"] = "nom"
        problems = B.check_blocks([block])
        self.assertTrue(any("already means something" in p for p in problems))
        block.fields["name"] = "3bad"
        self.assertTrue(B.check_blocks([block]))

    def test_validate_source(self):
        self.assertIsNone(B.validate_source("oga 1"))
        self.assertIsNotNone(B.validate_source("oga ("))


if __name__ == "__main__":
    unittest.main()