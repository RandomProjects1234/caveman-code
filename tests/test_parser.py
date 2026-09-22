import unittest

from tests.context import SRC  # noqa: F401

from cmc.parser import parse_source, parse_expression_source
from cmc.errors import CmcParseError
from cmc import ast_nodes as A


class TestParser(unittest.TestCase):
    def test_hello(self):
        program = parse_source('oga "hi"')
        self.assertEqual(len(program.statements), 1)
        say = program.statements[0]
        self.assertIsInstance(say, A.Say)
        self.assertIsInstance(say.exprs[0], A.Str)

    def test_say_many(self):
        program = parse_source('oga 1, "two", x')
        say = program.statements[0]
        self.assertEqual(len(say.exprs), 3)
        self.assertIsInstance(say.exprs[2], A.Var)

    def test_grunk_declare(self):
        statement = parse_source("grunk x = 5").statements[0]
        self.assertIsInstance(statement, A.Assign)
        self.assertTrue(statement.declare)
        self.assertEqual(statement.name, "x")

    def test_assign_change(self):
        statement = parse_source("x = 5").statements[0]
        self.assertIsInstance(statement, A.Assign)
        self.assertFalse(statement.declare)

    def test_if_else(self):
        program = parse_source(
            "binga x > 1\n"
            '    oga "a"\n'
            "wonga\n"
            '    oga "b"\n'
            "unga"
        )
        node = program.statements[0]
        self.assertIsInstance(node, A.If)
        self.assertEqual(len(node.body), 1)
        self.assertEqual(len(node.else_body), 1)
        self.assertIsInstance(node.else_body[0], A.Say)

    def test_wonga_binga_chain(self):
        program = parse_source(
            "binga a == 1\n"
            '    oga "one"\n'
            "wonga binga a == 2\n"
            '    oga "two"\n'
            "wonga\n"
            '    oga "many"\n'
            "unga"
        )
        node = program.statements[0]
        self.assertIsInstance(node, A.If)
        self.assertEqual(len(node.else_body), 1)
        inner = node.else_body[0]
        self.assertIsInstance(inner, A.If)
        self.assertEqual(len(inner.else_body), 1)

    def test_repeat_and_while(self):
        program = parse_source("booga 3\n    oga 1\nunga")
        self.assertIsInstance(program.statements[0], A.Repeat)
        program = parse_source("zug gronk\n    oga 1\nunga")
        self.assertIsInstance(program.statements[0], A.While)

    def test_foreach(self):
        program = parse_source("zoop pet in pets\n    oga pet\nunga")
        node = program.statements[0]
        self.assertIsInstance(node, A.ForEach)
        self.assertEqual(node.name, "pet")

    def test_function(self):
        program = parse_source("clump add(a, b)\n    ork a + b\nunga")
        node = program.statements[0]
        self.assertIsInstance(node, A.FuncDef)
        self.assertEqual(node.params, ["a", "b"])
        self.assertIsInstance(node.body[0], A.Return)

    def test_return_without_value(self):
        program = parse_source("clump f()\n    ork\nunga")
        node = program.statements[0].body[0]
        self.assertIsNone(node.expr)

    def test_call_statement(self):
        program = parse_source("greet()")
        statement = program.statements[0]
        self.assertIsInstance(statement, A.ExprStmt)
        self.assertIsInstance(statement.expr, A.Call)

    def test_unga_without_open(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("unga")
        self.assertIn("nothing was open", caught.exception.message)

    def test_missing_unga(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("binga gronk\n    oga 1")
        self.assertIn("never closed", caught.exception.message)

    def test_extra_words_on_line(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("oga 1 2")
        self.assertIn("extra things", caught.exception.hint + caught.exception.message)

    def test_forbidden_name(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("grunk nom = 5")
        self.assertIn("already means something", caught.exception.message)

    def test_grunk_needs_equals(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("grunk x 5")
        self.assertIn("needs a '='", caught.exception.message)

    def test_expression_line_without_use(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("5 + 5")
        self.assertIn("does nothing with it", caught.exception.message)

    def test_one_compare_at_a_time(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("binga 1 < 2 < 3\nunga")
        self.assertIn("One compare", caught.exception.message)

    def test_skrib_arg_count(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("skrib circle 1, 2")
        self.assertIn("needs 3", caught.exception.message)

    def test_skrib_unknown_shape(self):
        with self.assertRaises(CmcParseError) as caught:
            parse_source("skrib dragon 1")
        self.assertIn("does not know how to draw", caught.exception.message)

    def test_skrib_ok(self):
        node = parse_source("skrib line 0, 0, 1, 1").statements[0]
        self.assertIsInstance(node, A.Skrib)
        self.assertEqual(len(node.args), 4)

    def test_index_and_setindex(self):
        program = parse_source("pets[0] = 5")
        node = program.statements[0]
        self.assertIsInstance(node, A.SetIndex)

    def test_index_expression(self):
        program = parse_source("oga pets[1]")
        node = program.statements[0].exprs[0]
        self.assertIsInstance(node, A.Index)

    def test_lists_literal(self):
        program = parse_source("grunk xs = snorf(1, 2, 3)")
        call = program.statements[0].expr
        self.assertIsInstance(call, A.Call)
        self.assertEqual(len(call.args), 3)

    def test_ask(self):
        program = parse_source('grunk name = blorp "Who?"')
        self.assertIsInstance(program.statements[0].expr, A.Ask)
        program = parse_source("grunk name = blorp()")
        self.assertIsNone(program.statements[0].expr.prompt)

    def test_logic_precedence(self):
        program = parse_source("binga 1 == 1 or 2 == 3 and 4 == 4\nunga")
        cond = program.statements[0].cond
        self.assertIsInstance(cond, A.LogicOp)
        self.assertEqual(cond.op, "or")
        self.assertIsInstance(cond.right, A.LogicOp)
        self.assertEqual(cond.right.op, "and")

    def test_in_operator(self):
        program = parse_source('binga "cat" in pets\nunga')
        cond = program.statements[0].cond
        self.assertIsInstance(cond, A.InOp)
        self.assertFalse(cond.negated)
        program = parse_source('binga "cat" not in pets\nunga')
        self.assertTrue(program.statements[0].cond.negated)

    def test_parse_expression_source(self):
        expr = parse_expression_source("1 + munga(2, 3)")
        self.assertIsInstance(expr, A.BinOp)
        with self.assertRaises(CmcParseError):
            parse_expression_source("1 + 2 extra")

    def test_empty_program(self):
        self.assertEqual(parse_source("").statements, [])
        self.assertEqual(parse_source("\n\nugg hi\n").statements, [])

    def test_comments_inside_blocks(self):
        program = parse_source("booga 2\n    ugg a note\n    oga 1\nunga")
        self.assertEqual(len(program.statements[0].body), 1)


if __name__ == "__main__":
    unittest.main()