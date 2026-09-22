import unittest

from tests.context import SRC  # noqa: F401

from cmc.interpreter import Interpreter
from cmc.errors import CmcRuntimeError, CmcStepLimit, CmcStopped, CmcTooDeep


def run(source, inputs=None, **kwargs):
    lines = []
    input_func = None
    if inputs is not None:
        answers = iter(inputs)
        input_func = lambda prompt, line: next(answers)
    interp = Interpreter(output=lines.append, input_func=input_func, **kwargs)
    interp.run(source)
    return lines


class TestValues(unittest.TestCase):
    def test_show_forms(self):
        self.assertEqual(run("oga gronk, nork, plop"), ["gronk nork plop"])
        self.assertEqual(run("oga 2.5, 2.0, 2"), ["2.5 2 2"])
        self.assertEqual(run("oga snorf(1, gronk, plop, \"x\")"), ["[1, gronk, plop, x]"])

    def test_truthiness(self):
        self.assertEqual(run("binga 0\n    oga \"yes\"\nwonga\n    oga \"no\"\nunga"), ["no"])
        self.assertEqual(run("binga 5\n    oga \"yes\"\nwonga\n    oga \"no\"\nunga"), ["yes"])
        self.assertEqual(run('binga ""\n    oga "yes"\nwonga\n    oga "no"\nunga'), ["no"])
        self.assertEqual(run("binga snorf()\n    oga \"yes\"\nwonga\n    oga \"no\"\nunga"), ["no"])

    def test_equality_never_crashes(self):
        self.assertEqual(run('oga 1 == 1, 1 == "1", gronk == 1, plop == plop'), ["gronk nork nork gronk"])

    def test_list_equality(self):
        self.assertEqual(run("oga snorf(1, 2) == snorf(1, 2)"), ["gronk"])


class TestMath(unittest.TestCase):
    def test_div_keeps_whole_when_it_fits(self):
        self.assertEqual(run("oga 4 / 2, 5 / 2"), ["2 2.5"])

    def test_modulo(self):
        self.assertEqual(run("oga 7 % 3"), ["1"])

    def test_div_zero(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("oga 5 / 0")
        self.assertIn("share by zero", caught.exception.message)

    def test_mod_zero(self):
        with self.assertRaises(CmcRuntimeError):
            run("oga 5 % 0")

    def test_minus_non_number(self):
        with self.assertRaises(CmcRuntimeError):
            run('oga "a" - 1')

    def test_helpers(self):
        self.assertEqual(
            run("oga big(2, 9), small(2, 9), abs(0 - 4), root(16), pow(2, 4), round(2.5), round(0 - 2.5)"),
            ["9 2 4 4 16 3 -3"],
        )


class TestText(unittest.TestCase):
    def test_concat(self):
        self.assertEqual(run('oga "ca" + "ve"'), ["cave"])

    def test_mixed_add_hint(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run('oga "x" + 1')
        self.assertIn("goop", caught.exception.hint)

    def test_helpers(self):
        self.assertEqual(
            run('oga shout("a"), whisper("B"), flip("abc"), find("abc", "c"), nom("abcd")'),
            ["A b cba 2 4"],
        )

    def test_split_join(self):
        self.assertEqual(run('oga split("a,b,c", ","), join(snorf(1, 2), "-")'), ["[a, b, c] 1-2"])

    def test_what(self):
        self.assertEqual(
            run('oga what(1), what("a"), what(gronk), what(plop), what(snorf())'),
            ["number text truth plop pile"],
        )


class TestLists(unittest.TestCase):
    def test_build_and_read(self):
        program = 'grunk xs = snorf(1, 2)\nplop(xs, 3)\noga xs, nom(xs), xs[0], xs[-1], skoop(xs, 1)'
        self.assertEqual(run(program), ["[1, 2, 3] 3 1 3 2"])

    def test_yoink(self):
        self.assertEqual(run("grunk xs = snorf(1, 2, 3)\noga yoink(xs), xs"), ["3 [1, 2]"])

    def test_yoink_empty(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("grunk xs = snorf()\noga yoink(xs)")
        self.assertIn("empty", caught.exception.message)

    def test_index_assign(self):
        self.assertEqual(run("grunk xs = snorf(1, 2)\nxs[0] = 9\noga xs"), ["[9, 2]"])

    def test_index_out_of_range(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("grunk xs = snorf(1)\noga xs[5]")
        self.assertIn("no spot 5", caught.exception.message)

    def test_in(self):
        self.assertEqual(run('oga "cat" in snorf("dog", "cat"), "z" in "cave", "c" not in "cave"'),
                         ["gronk nork nork"])


class TestControl(unittest.TestCase):
    def test_repeat_lap(self):
        self.assertEqual(run("booga 3\n    oga lap\nunga"), ["1", "2", "3"])

    def test_repeat_zero(self):
        self.assertEqual(run("booga 0\n    oga 1\nunga"), [])

    def test_repeat_fraction_error(self):
        with self.assertRaises(CmcRuntimeError):
            run("booga 2.5\n    oga 1\nunga")

    def test_while(self):
        self.assertEqual(run("grunk n = 3\nzug n > 0\n    oga n\n    n = n - 1\nunga"), ["3", "2", "1"])

    def test_foreach_text(self):
        self.assertEqual(run("zoop c in \"ab\"\n    oga c\nunga"), ["a", "b"])

    def test_foreach_wrong_type(self):
        with self.assertRaises(CmcRuntimeError):
            run("zoop x in 5\n    oga x\nunga")

    def test_if_else_chain(self):
        program = (
            "grunk n = 2\n"
            "binga n == 1\n"
            '    oga "one"\n'
            "wonga binga n == 2\n"
            '    oga "two"\n'
            "wonga\n"
            '    oga "other"\n'
            "unga"
        )
        self.assertEqual(run(program), ["two"])


class TestClumps(unittest.TestCase):
    def test_call_and_return(self):
        program = "clump add(a, b)\n    ork a + b\nunga\noga add(2, 3)"
        self.assertEqual(run(program), ["5"])

    def test_no_return_gives_plop(self):
        self.assertEqual(run("clump f()\n    oga 1\nunga\noga f()"), ["1", "plop"])

    def test_wrong_arg_count(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("clump f(a)\n    ork a\nunga\nf(1, 2)")
        self.assertIn("wants 1 thing", caught.exception.message)

    def test_unknown_clump(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("nope()")
        self.assertIn("no clump named", caught.exception.message)

    def test_recursion(self):
        program = "clump fib(n)\n    binga n < 2\n        ork n\n    unga\n    ork fib(n-1) + fib(n-2)\nunga\noga fib(10)"
        self.assertEqual(run(program), ["55"])

    def test_too_deep(self):
        program = "clump loop(n)\n    ork loop(n + 1)\nunga\nloop(1)"
        with self.assertRaises(CmcTooDeep):
            run(program, max_depth=50)

    def test_return_outside_clump(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("ork 5")
        self.assertIn("inside a clump", caught.exception.message)

    def test_variable_is_not_callable(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("grunk x = 5\nx()")
        self.assertIn("not a clump", caught.exception.message)


class TestBoxes(unittest.TestCase):
    def test_undefined_box(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run("oga nope")
        self.assertEqual(caught.exception.hint.split(":")[0], "make it first with")

    def test_change_undefined_box(self):
        with self.assertRaises(CmcRuntimeError):
            run("x = 1")

    def test_change_defined_box(self):
        self.assertEqual(run("grunk x = 1\nx = x + 1\noga x"), ["2"])


class TestRandom(unittest.TestCase):
    def test_deterministic_munga(self):
        self.assertEqual(run("oga munga(1, 1), munga(5, 5), munga(0, 0)"), ["1 5 0"])


class TestNumba(unittest.TestCase):
    def test_convert(self):
        self.assertEqual(run('oga numba("42") + 1, numba("3.5") + 0.5, numba(7)'), ["43 4 7"])

    def test_bad_text(self):
        with self.assertRaises(CmcRuntimeError) as caught:
            run('oga numba("banana")')
        self.assertIn("banana", caught.exception.message)


class TestAsk(unittest.TestCase):
    def test_ask_returns_text(self):
        self.assertEqual(run('grunk n = blorp "Name?"\noga "hi", n', inputs=["Ugg"]), ["hi Ugg"])

    def test_ask_no_prompt(self):
        self.assertEqual(run("oga blorp()", inputs=["7"]), ["7"])


class TestSafety(unittest.TestCase):
    def test_step_limit(self):
        with self.assertRaises(CmcStepLimit):
            run("zug gronk\n    oga 1\nunga", step_limit=100)

    def test_stop_flag(self):
        state = {"calls": 0}

        def should_stop():
            state["calls"] += 1
            return state["calls"] > 5

        with self.assertRaises(CmcStopped):
            run("zug gronk\n    oga 1\nunga", should_stop=should_stop)

    def test_wait_hook(self):
        slept = []
        run("wait(250)", sleep=slept.append)
        self.assertEqual(slept, [0.25])


class TestPythonInterop(unittest.TestCase):
    def test_pi(self):
        self.assertTrue(run("oga pi")[0].startswith("3.14159"))


if __name__ == "__main__":
    unittest.main()