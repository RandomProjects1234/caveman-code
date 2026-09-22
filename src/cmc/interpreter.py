"""The engine that actually runs a CMC tree.

It walks the tree, keeps the boxes (variables), and does the math.  It also
protects little programs from big oopsies: runaway loops, too-deep clumps,
and divider-by-zero.
"""

import time

from .errors import (
    CmcRuntimeError,
    CmcStopped,
    CmcStepLimit,
    CmcTooDeep,
)
from .parser import parse_source
from . import ast_nodes as A
from . import builtins as B
from .values import show, truthy, equal, type_word, number, whole, isnum

DEFAULT_STEP_LIMIT = 5_000_000
DEFAULT_MAX_DEPTH = 200


class ReturnSignal(Exception):
    def __init__(self, value):
        self.value = value


class Env:
    def __init__(self, parent=None):
        self.vars = {}
        self.parent = parent

    def find(self, name):
        env = self
        while env is not None:
            if name in env.vars:
                return env
            env = env.parent
        return None

    def get(self, name, line):
        env = self.find(name)
        if env is None:
            raise CmcRuntimeError(
                "There is no box named '" + name + "'.",
                line=line,
                hint="make it first with: grunk " + name + " = ...",
            )
        return env.vars[name]

    def define(self, name, value):
        self.vars[name] = value

    def assign(self, name, value, line):
        env = self.find(name)
        if env is None:
            raise CmcRuntimeError(
                "There is no box named '" + name + "' to put that into.",
                line=line,
                hint="make it first with: grunk " + name + " = ...",
            )
        env.vars[name] = value


class UserFunction:
    def __init__(self, node, closure):
        self.node = node
        self.closure = closure


class Interpreter:
    def __init__(
        self,
        output=None,
        input_func=None,
        draw=None,
        should_stop=None,
        sleep=None,
        step_limit=DEFAULT_STEP_LIMIT,
        max_depth=DEFAULT_MAX_DEPTH,
    ):
        self.output = output if output is not None else (lambda line: print(line))
        self.input_func = input_func
        self.draw = draw
        self.should_stop = should_stop
        self.sleep = sleep
        self.step_limit = step_limit
        self.max_depth = max_depth
        self.steps = 0
        self.depth = 0
        self.globals = Env()
        self._warned_no_draw = False
        self._install_constants()

    # ------------------------------------------------------------ running

    def _install_constants(self):
        for name, value in B.CONSTANTS.items():
            self.globals.define(name, value)

    def run(self, source, filename="<cmc>"):
        program = parse_source(source, filename)
        return self.run_tree(program)

    def run_tree(self, program):
        self.steps = 0
        self.depth = 0
        try:
            for stmt in program.statements:
                self.exec_statement(stmt, self.globals)
        except ReturnSignal:
            raise CmcRuntimeError(
                "'ork' can only live inside a clump.",
                line=None,
                hint="wrap it like this: clump name() ... ork answer ... unga",
            )
        return None

    def tick(self, line=None):
        self.steps += 1
        if self.steps > self.step_limit:
            raise CmcStepLimit(line)
        if self.should_stop is not None and self.should_stop():
            raise CmcStopped()

    def wait_ms(self, ms):
        if self.sleep is not None:
            self.sleep(ms / 1000.0)
            return
        end = time.monotonic() + (ms / 1000.0)
        while time.monotonic() < end:
            self.tick()
            remaining = end - time.monotonic()
            time.sleep(min(0.02, max(0.0, remaining)))

    # --------------------------------------------------------- statements

    def exec_statement(self, node, env):
        self.tick(node.line)

        if isinstance(node, A.Say):
            values = [self.eval(node_expr, env) for node_expr in node.exprs]
            self.output(" ".join(show(v) for v in values))
            return

        if isinstance(node, A.Assign):
            value = self.eval(node.expr, env)
            if node.declare:
                env.define(node.name, value)
            else:
                env.assign(node.name, value, node.line)
            return

        if isinstance(node, A.SetIndex):
            target = self.eval(node.target, env)
            index = whole(self.eval(node.index, env), line=node.line, extra="A spot number")
            value = self.eval(node.value, env)
            if not isinstance(target, list):
                raise CmcRuntimeError(
                    "Only piles can have spots changed, but that is " + type_word(target) + ".",
                    line=node.line,
                    hint="make a pile with snorf(...) and plop things into it.",
                )
            if index < 0:
                index += len(target)
            if index < 0 or index >= len(target):
                raise CmcRuntimeError(
                    "That pile has no spot " + str(index) + " yet (it has " + str(len(target)) + " spots).",
                    line=node.line,
                    hint="add to the end of a pile with plop(pile, thing).",
                )
            target[index] = value
            return

        if isinstance(node, A.If):
            if truthy(self.eval(node.cond, env)):
                self.exec_block(node.body, env)
            else:
                self.exec_block(node.else_body, env)
            return

        if isinstance(node, A.Repeat):
            count = whole(self.eval(node.count, env), line=node.line, extra="'booga'")
            for lap in range(max(0, count)):
                self.tick(node.line)
                env.define("lap", lap + 1)
                self.exec_block(node.body, env)
            return

        if isinstance(node, A.While):
            while truthy(self.eval(node.cond, env)):
                self.tick(node.line)
                self.exec_block(node.body, env)
            return

        if isinstance(node, A.ForEach):
            iterable = self.eval(node.iterable, env)
            if isinstance(iterable, list):
                items = list(iterable)
            elif isinstance(iterable, str):
                items = list(iterable)
            else:
                raise CmcRuntimeError(
                    "'zoop' walks through piles and text, but that is " + type_word(iterable) + ".",
                    line=node.line,
                    hint="make a pile with snorf(...) first.",
                )
            for item in items:
                self.tick(node.line)
                env.define(node.name, item)
                self.exec_block(node.body, env)
            return

        if isinstance(node, A.FuncDef):
            env.define(node.name, UserFunction(node, env))
            return

        if isinstance(node, A.Return):
            value = None if node.expr is None else self.eval(node.expr, env)
            raise ReturnSignal(value)

        if isinstance(node, A.Skrib):
            self.exec_skrib(node, env)
            return

        if isinstance(node, A.ExprStmt):
            self.eval(node.expr, env)
            return

        raise CmcRuntimeError("I do not know how to do that yet.", line=getattr(node, "line", None))

    def exec_block(self, statements, env):
        for stmt in statements:
            self.exec_statement(stmt, env)

    def exec_skrib(self, node, env):
        values = [self.eval(arg, env) for arg in node.args]
        if self.draw is None:
            if not self._warned_no_draw:
                self._warned_no_draw = True
            return
        shape = node.shape
        if shape == "clear":
            self.draw.clear()
        elif shape == "size":
            self.draw.size(whole(values[0], line=node.line, extra="skrib size"),
                           whole(values[1], line=node.line, extra="skrib size"))
        elif shape == "color":
            self.draw.color(show(values[0]) if not isinstance(values[0], str) else values[0])
        elif shape == "dot":
            self.draw.dot(values[0], values[1], values[2])
        elif shape == "circle":
            self.draw.circle(values[0], values[1], values[2])
        elif shape == "line":
            self.draw.line(values[0], values[1], values[2], values[3])
        elif shape == "box":
            self.draw.box(values[0], values[1], values[2], values[3])
        elif shape == "blob":
            self.draw.blob(values[0], values[1], values[2], values[3])
        elif shape == "write":
            self.draw.write(show(values[0]) if not isinstance(values[0], str) else values[0],
                            values[1], values[2])

    # -------------------------------------------------------- expressions

    def eval(self, node, env):
        if isinstance(node, A.Num):
            return node.value
        if isinstance(node, A.Str):
            return node.value
        if isinstance(node, A.Var):
            return env.get(node.name, node.line)
        if isinstance(node, A.Neg):
            value = self.eval(node.operand, env)
            return -number(value, line=node.line)
        if isinstance(node, A.Not):
            return not truthy(self.eval(node.operand, env))
        if isinstance(node, A.LogicOp):
            left = self.eval(node.left, env)
            if node.op == "and":
                if not truthy(left):
                    return False
                return truthy(self.eval(node.right, env))
            if truthy(left):
                return True
            return truthy(self.eval(node.right, env))
        if isinstance(node, A.Compare):
            return self.eval_compare(node, env)
        if isinstance(node, A.InOp):
            return self.eval_in(node, env)
        if isinstance(node, A.BinOp):
            return self.eval_binop(node, env)
        if isinstance(node, A.Index):
            return self.eval_index(node, env)
        if isinstance(node, A.Ask):
            return self.eval_ask(node, env)
        if isinstance(node, A.Snorf):
            return [self.eval(item, env) for item in node.items]
        if isinstance(node, A.Call):
            return self.eval_call(node, env)
        raise CmcRuntimeError("I do not know that expression yet.", line=getattr(node, "line", None))

    def eval_binop(self, node, env):
        left = self.eval(node.left, env)
        right = self.eval(node.right, env)
        op = node.op
        line = node.line

        if op == "+":
            if isnum(left) and isnum(right):
                result = left + right
                return result
            if isinstance(left, str) and isinstance(right, str):
                return left + right
            if isinstance(left, str) or isinstance(right, str):
                raise CmcRuntimeError(
                    "Text and numbers cannot be stuck together directly.",
                    line=line,
                    hint="wrap the number in goop first: \"I am \" + goop(7)",
                )
            raise CmcRuntimeError(
                "Cannot add " + type_word(left) + " and " + type_word(right) + ".",
                line=line,
                hint="+ works on numbers and text.",
            )

        if op == "-":
            a = number(left, line=line, extra="Minus")
            b = number(right, line=line, extra="Minus")
            return a - b

        if op == "*":
            if isinstance(left, str) or isinstance(right, str):
                raise CmcRuntimeError(
                    "Text cannot be multiplied.",
                    line=line,
                    hint="to repeat text, use a booga loop and add it again and again.",
                )
            a = number(left, line=line, extra="Times")
            b = number(right, line=line, extra="Times")
            return a * b

        if op == "/":
            a = number(left, line=line, extra="Divide")
            b = number(right, line=line, extra="Divide")
            if b == 0:
                raise CmcRuntimeError(
                    "Cannot share by zero!",
                    line=line,
                    hint="check the number is not 0 before dividing.",
                )
            result = a / b
            if isinstance(a, int) and isinstance(b, int) and a % b == 0:
                return a // b
            return result

        if op == "%":
            a = number(left, line=line, extra="Remainder")
            b = number(right, line=line, extra="Remainder")
            if b == 0:
                raise CmcRuntimeError(
                    "Cannot find the remainder with zero.",
                    line=line,
                    hint="the remainder % needs a number that is not 0.",
                )
            return a % b

        raise CmcRuntimeError("I do not know the math sign '" + op + "'.", line=line)

    def eval_compare(self, node, env):
        left = self.eval(node.left, env)
        right = self.eval(node.right, env)
        op = node.op
        line = node.line

        if op == "==":
            return equal(left, right)
        if op == "!=":
            return not equal(left, right)

        if isnum(left) and isnum(right):
            pass
        elif isinstance(left, str) and isinstance(right, str):
            pass
        else:
            raise CmcRuntimeError(
                "I can only line up numbers with numbers, or text with text (not "
                + type_word(left) + " with " + type_word(right) + ").",
                line=line,
                hint="use goop() to turn one of them into text first.",
            )

        if op == "<":
            return left < right
        if op == "<=":
            return left <= right
        if op == ">":
            return left > right
        if op == ">=":
            return left >= right
        raise CmcRuntimeError("I do not know the compare sign '" + op + "'.", line=line)

    def eval_in(self, node, env):
        item = self.eval(node.item, env)
        container = self.eval(node.container, env)
        found = None
        if isinstance(container, list):
            found = any(equal(item, x) for x in container)
        elif isinstance(container, str):
            if not isinstance(item, str):
                raise CmcRuntimeError(
                    "'in' can look inside text, but only for other text.",
                    line=node.line,
                    hint='look for "cat" not 7. Use goop(7) to make it text.',
                )
            found = item in container
        else:
            raise CmcRuntimeError(
                "'in' looks inside piles and text, but that is " + type_word(container) + ".",
                line=node.line,
                hint="make a pile with snorf(...) first.",
            )
        return (not found) if node.negated else found

    def eval_index(self, node, env):
        target = self.eval(node.target, env)
        index = whole(self.eval(node.index, env), line=node.line, extra="A spot number")
        if not isinstance(target, (list, str)):
            raise CmcRuntimeError(
                "Only piles and text have spots, but that is " + type_word(target) + ".",
                line=node.line,
                hint="make a pile with snorf(...) first.",
            )
        if index < 0:
            index += len(target)
        if index < 0 or index >= len(target):
            raise CmcRuntimeError(
                "There is no spot " + str(index) + " in that "
                + ("text" if isinstance(target, str) else "pile")
                + " (it has " + str(len(target)) + " spots).",
                line=node.line,
                hint="spots start at 0, so the last spot is " + str(len(target) - 1) + ".",
            )
        return target[index]

    def eval_ask(self, node, env):
        prompt = None
        if node.prompt is not None:
            value = self.eval(node.prompt, env)
            prompt = value if isinstance(value, str) else show(value)
        if self.input_func is None:
            try:
                answer = input(prompt + " " if prompt else "")
            except EOFError:
                raise CmcRuntimeError(
                    "blorp asked a question, but no answer could come in.",
                    line=node.line,
                    hint="make sure someone can type an answer.",
                )
            return answer
        return self.input_func(prompt, node.line)

    def eval_call(self, node, env):
        self.tick(node.line)
        args = [self.eval(arg, env) for arg in node.args]
        line = node.line

        holder = env.find(node.name)
        if holder is not None:
            value = holder.vars[node.name]
            if isinstance(value, UserFunction):
                return self.call_user(value, args, line)
            if node.name in B.IMPLS:
                result, _ = B.call_builtin(self, node.name, args, line)
                return result
            raise CmcRuntimeError(
                "'" + node.name + "' is a box, not a clump, so it cannot be called with ( ).",
                line=line,
                hint="did you mean to use it without parentheses?",
            )

        if node.name in B.IMPLS:
            result, _ = B.call_builtin(self, node.name, args, line)
            return result

        raise CmcRuntimeError(
            "There is no clump named '" + node.name + "'.",
            line=line,
            hint="make it first with: clump " + node.name + "(...)",
        )

    def call_user(self, func, args, line):
        node = func.node
        if len(args) != len(node.params):
            raise CmcRuntimeError(
                "'" + node.name + "' wants " + str(len(node.params)) + " thing"
                + ("s" if len(node.params) != 1 else "")
                + ", but got " + str(len(args)) + ".",
                line=line,
                hint="look at how the clump was made: clump " + node.name + "(" + ", ".join(node.params) + ")",
            )
        self.depth += 1
        if self.depth > self.max_depth:
            self.depth -= 1
            raise CmcTooDeep(node.name, line=line)
        local = Env(parent=func.closure)
        for name, value in zip(node.params, args):
            local.define(name, value)
        try:
            self.exec_block(node.body, local)
        except ReturnSignal as signal:
            return signal.value
        finally:
            self.depth -= 1
        return None


def run_source(source, **kwargs):
    """A tiny convenience for other tools and tests."""
    return Interpreter(**kwargs).run(source)