"""Compile a CMC program into readable Python.

This is the 'compiler' half of Cave Man Code: `cmc compile hello.cmc` makes a
real .py file you can run anywhere Python runs.  The generated file carries
its own little runtime (the helpers below) so it stands alone.
"""

from . import ast_nodes as A
from .parser import parse_source
from .words import VERSION

PRELUDE = '''\
# ---------------------------------------------------------------------
# Cave Man Code helpers -- the same little runtime lives in every file.
# Made by CMC v{version}
# ---------------------------------------------------------------------
import math as _math
import random as _random
import time as _time

gronk = True
nork = False
plop = None
pi = _math.pi


class _CmcOops(Exception):
    pass


def _cmc_show(v):
    if v is None:
        return "plop"
    if v is True:
        return "gronk"
    if v is False:
        return "nork"
    if isinstance(v, list):
        return "[" + ", ".join(_cmc_show(x) for x in v) + "]"
    if isinstance(v, float) and v.is_integer():
        return str(int(v))
    return str(v)


def _cmc_say(*things):
    print(" ".join(_cmc_show(t) for t in things))


def _cmc_true(v):
    if v is None:
        return False
    if isinstance(v, bool):
        return v
    if isinstance(v, (int, float)):
        return v != 0
    if isinstance(v, (str, list)):
        return len(v) > 0
    return True


def _cmc_num(v):
    if isinstance(v, bool) or not isinstance(v, (int, float)):
        raise _CmcOops("needs a number, but got " + _cmc_show(v))
    return v


def _cmc_whole(v):
    n = _cmc_num(v)
    if isinstance(n, float) and not n.is_integer():
        raise _CmcOops("needs a whole number, but got " + _cmc_show(v))
    return int(n)


def _cmc_text(v):
    if not isinstance(v, str):
        raise _CmcOops("needs text, but got " + _cmc_show(v))
    return v


def _cmc_plus(a, b):
    if isinstance(a, str) and isinstance(b, str):
        return a + b
    if isinstance(a, str) or isinstance(b, str):
        raise _CmcOops("text and numbers need goop() to stick together")
    return _cmc_num(a) + _cmc_num(b)


def _cmc_minus(a, b):
    return _cmc_num(a) - _cmc_num(b)


def _cmc_times(a, b):
    if isinstance(a, str) or isinstance(b, str):
        raise _CmcOops("text cannot be multiplied")
    return _cmc_num(a) * _cmc_num(b)


def _cmc_div(a, b):
    a, b = _cmc_num(a), _cmc_num(b)
    if b == 0:
        raise _CmcOops("cannot share by zero")
    if isinstance(a, int) and isinstance(b, int) and a % b == 0:
        return a // b
    return a / b


def _cmc_mod(a, b):
    a, b = _cmc_num(a), _cmc_num(b)
    if b == 0:
        raise _CmcOops("cannot find the remainder with zero")
    return a % b


def _cmc_eq(a, b):
    if isinstance(a, bool) or isinstance(b, bool):
        return isinstance(a, bool) and isinstance(b, bool) and a == b
    if a is None or b is None:
        return a is None and b is None
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return a == b
    if isinstance(a, str) and isinstance(b, str):
        return a == b
    if isinstance(a, list) and isinstance(b, list):
        return len(a) == len(b) and all(_cmc_eq(x, y) for x, y in zip(a, b))
    return False


def _cmc_cmp(op, a, b):
    a_num = isinstance(a, (int, float)) and not isinstance(a, bool)
    b_num = isinstance(b, (int, float)) and not isinstance(b, bool)
    if not ((a_num and b_num) or (isinstance(a, str) and isinstance(b, str))):
        raise _CmcOops("can only line up numbers with numbers, or text with text")
    if op == "<":
        return a < b
    if op == "<=":
        return a <= b
    if op == ">":
        return a > b
    return a >= b


def _cmc_in(item, container, negated):
    if isinstance(container, list):
        found = any(_cmc_eq(item, x) for x in container)
    elif isinstance(container, str):
        if not isinstance(item, str):
            raise _CmcOops("'in' can only look for text inside text")
        found = item in container
    else:
        raise _CmcOops("'in' looks inside piles and text, not that")
    return (not found) if negated else found


def _cmc_index(v, i):
    i = _cmc_whole(i)
    if not isinstance(v, (list, str)):
        raise _CmcOops("only piles and text have spots")
    if i < 0:
        i += len(v)
    if i < 0 or i >= len(v):
        raise _CmcOops("no spot " + str(i) + " in that " + ("text" if isinstance(v, str) else "pile"))
    return v[i]


def _cmc_set_index(v, i, x):
    if not isinstance(v, list):
        raise _CmcOops("only piles can have spots changed")
    i = _cmc_whole(i)
    if i < 0:
        i += len(v)
    if i < 0 or i >= len(v):
        raise _CmcOops("no spot " + str(i) + " in that pile")
    v[i] = x


def _cmc_goop(v):
    return _cmc_show(v)


def _cmc_snorf(*things):
    return list(things)


def _cmc_nom(v):
    if isinstance(v, (str, list)):
        return len(v)
    raise _CmcOops("nom counts piles and text, not that")


def _cmc_skoop(v, i):
    return _cmc_index(v, i)


def _cmc_yoink(v):
    if not isinstance(v, list):
        raise _CmcOops("yoink needs a pile")
    if not v:
        raise _CmcOops("the pile is empty, nothing to yoink")
    return v.pop()


def _cmc_plop(v, x):
    if not isinstance(v, list):
        raise _CmcOops("plop needs a pile")
    v.append(x)


def _cmc_shout(v):
    return _cmc_text(v).upper()


def _cmc_whisper(v):
    return _cmc_text(v).lower()


def _cmc_flip(v):
    return _cmc_text(v)[::-1]


def _cmc_find(v, p):
    return _cmc_text(v).find(_cmc_text(p))


def _cmc_split(v, s):
    s = _cmc_text(s)
    if s == "":
        raise _CmcOops("split needs something to cut on")
    return _cmc_text(v).split(s)


def _cmc_join(v, s):
    if not isinstance(v, list):
        raise _CmcOops("join needs a pile")
    return _cmc_text(s).join(_cmc_show(x) for x in v)


def _cmc_what(v):
    if v is None:
        return "plop"
    if isinstance(v, bool):
        return "truth"
    if isinstance(v, (int, float)):
        return "number"
    if isinstance(v, str):
        return "text"
    if isinstance(v, list):
        return "pile"
    return "thing"


def _cmc_numba(v):
    if isinstance(v, bool):
        raise _CmcOops("numba cannot turn gronk/nork into a number")
    if isinstance(v, (int, float)):
        return v
    if isinstance(v, str):
        text = v.strip()
        try:
            return int(text)
        except ValueError:
            pass
        try:
            return float(text)
        except ValueError:
            raise _CmcOops("numba cannot turn " + _cmc_show(v) + " into a number")
    raise _CmcOops("numba needs text or a number")


def _cmc_munga(a, b):
    a, b = _cmc_whole(a), _cmc_whole(b)
    if a > b:
        a, b = b, a
    return _random.randint(a, b)


def _cmc_round(v):
    v = _cmc_num(v)
    if v >= 0:
        return int(_math.floor(v + 0.5))
    return -int(_math.floor(-v + 0.5))


def _cmc_flat(v):
    return _math.floor(_cmc_num(v))


def _cmc_roof(v):
    return _math.ceil(_cmc_num(v))


def _cmc_abs(v):
    return abs(_cmc_num(v))


def _cmc_small(a, b):
    return min(_cmc_num(a), _cmc_num(b))


def _cmc_big(a, b):
    return max(_cmc_num(a), _cmc_num(b))


def _cmc_root(v):
    v = _cmc_num(v)
    if v < 0:
        raise _CmcOops("root cannot work on negative numbers")
    r = _math.sqrt(v)
    return int(r) if float(r).is_integer() else r


def _cmc_pow(a, b):
    r = _cmc_num(a) ** _cmc_num(b)
    return int(r) if isinstance(r, float) and r.is_integer() else r


def _cmc_wait(ms):
    _time.sleep(max(0, _cmc_num(ms)) / 1000.0)


def _cmc_ask(prompt):
    try:
        return input((prompt + " ") if prompt else "")
    except EOFError:
        raise _CmcOops("blorp asked a question but no answer came in")


def _cmc_laps(n):
    return range(1, max(0, _cmc_whole(n)) + 1)


def _cmc_iter(v):
    if isinstance(v, (list, str)):
        return v
    raise _CmcOops("zoop walks through piles and text, not that")


class _CmcPen:
    def __init__(self):
        self.used = False
        self.root = None
        self.canvas = None
        self.color = "black"
        self.width = 400
        self.height = 400

    def _open(self):
        if self.root is None:
            import tkinter as tk
            self.root = tk.Tk()
            self.root.title("CMC picture")
            self.canvas = tk.Canvas(self.root, width=self.width, height=self.height, bg="white")
            self.canvas.pack()

    def skrib(self, shape, args):
        self.used = True
        self._open()
        if shape == "clear":
            self.canvas.delete("all")
        elif shape == "size":
            self.width, self.height = int(args[0]), int(args[1])
            self.canvas.config(width=self.width, height=self.height)
        elif shape == "color":
            self.color = args[0]
        elif shape == "dot":
            x, y, r = args
            self.canvas.create_oval(x - r, y - r, x + r, y + r, fill=self.color, outline=self.color)
        elif shape == "circle":
            x, y, r = args
            self.canvas.create_oval(x - r, y - r, x + r, y + r, outline=self.color, width=2)
        elif shape == "line":
            x1, y1, x2, y2 = args
            self.canvas.create_line(x1, y1, x2, y2, fill=self.color, width=2)
        elif shape == "box":
            x, y, w, h = args
            self.canvas.create_rectangle(x, y, x + w, y + h, outline=self.color, width=2)
        elif shape == "blob":
            x, y, w, h = args
            self.canvas.create_rectangle(x, y, x + w, y + h, fill=self.color, outline=self.color)
        elif shape == "write":
            text, x, y = args
            self.canvas.create_text(x, y, text=str(text), fill=self.color, anchor="nw")

    def hold(self):
        if self.root is not None:
            print("(the picture window is open -- close it to finish)")
            self.root.mainloop()


_PEN = _CmcPen()


def _cmc_skrib(shape, args):
    _PEN.skrib(shape, args)


def _cmc_hold():
    _PEN.hold()
# ---------------------------------------------------------------------
'''.format(version=VERSION)


class PythonWriter:
    def __init__(self):
        self.lines = []
        self.indent = 0

    def emit(self, text=""):
        if text:
            self.lines.append("    " * self.indent + text)
        else:
            self.lines.append("")

    def write(self, program, filename="<cmc>", version=VERSION):
        self.lines = []
        self.emit("# " + "=" * 68)
        self.emit("# This file was made by Cave Man Code (CMC) v" + version)
        if filename and filename != "<cmc>":
            self.emit("# From: " + filename)
        self.emit("# Feel free to read it and change it -- it is normal Python!")
        self.emit("# " + "=" * 68)
        self.emit("")
        self.emit(PRELUDE)
        self.emit("")
        self.emit("# ---------------------- your program starts here ---------------------")
        self.emit("")
        self.indent = 0
        for stmt in program.statements:
            self.statement(stmt)
        self.emit("")
        self.emit("# keep a picture window open if the program drew something")
        self.emit("if _PEN.used:")
        self.emit("    _cmc_hold()")
        return "\n".join(self.lines) + "\n"

    # --------------------------------------------------------- statements

    def statement(self, node):
        if isinstance(node, A.Say):
            self.emit("_cmc_say(" + ", ".join(self.expr(e) for e in node.exprs) + ")")
            return
        if isinstance(node, A.Assign):
            self.emit(node.name + " = " + self.expr(node.expr))
            return
        if isinstance(node, A.SetIndex):
            self.emit(
                "_cmc_set_index(" + self.expr(node.target) + ", " + self.expr(node.index)
                + ", " + self.expr(node.value) + ")"
            )
            return
        if isinstance(node, A.If):
            self.emit_if(node)
            return
        if isinstance(node, A.Repeat):
            self.emit("for lap in _cmc_laps(" + self.expr(node.count) + "):")
            self.body(node.body)
            return
        if isinstance(node, A.While):
            self.emit("while _cmc_true(" + self.expr(node.cond) + "):")
            self.body(node.body)
            return
        if isinstance(node, A.ForEach):
            self.emit("for " + node.name + " in _cmc_iter(" + self.expr(node.iterable) + "):")
            self.body(node.body)
            return
        if isinstance(node, A.FuncDef):
            params = ", ".join(node.params)
            self.emit("def " + node.name + "(" + params + "):")
            self.body(node.body, empty_pass=True)
            return
        if isinstance(node, A.Return):
            if node.expr is None:
                self.emit("return None")
            else:
                self.emit("return " + self.expr(node.expr))
            return
        if isinstance(node, A.ExprStmt):
            self.emit(self.expr(node.expr))
            return
        if isinstance(node, A.Skrib):
            args = ", ".join(self.expr(a) for a in node.args)
            self.emit('_cmc_skrib("' + node.shape + '", [' + args + "])")
            return
        self.emit("# (CMC cannot write this line yet)")

    def emit_if(self, node):
        self.emit("if _cmc_true(" + self.expr(node.cond) + "):")
        self.body(node.body)
        else_body = node.else_body
        while else_body:
            if len(else_body) == 1 and isinstance(else_body[0], A.If):
                inner = else_body[0]
                self.emit("elif _cmc_true(" + self.expr(inner.cond) + "):")
                self.body(inner.body)
                else_body = inner.else_body
                continue
            self.emit("else:")
            self.body(else_body)
            else_body = []

    def body(self, statements, empty_pass=False):
        self.indent += 1
        if not statements:
            if empty_pass:
                self.emit("pass")
        else:
            for stmt in statements:
                self.statement(stmt)
        self.indent -= 1

    # -------------------------------------------------------- expressions

    def expr(self, node):
        if isinstance(node, A.Num):
            return repr(node.value)
        if isinstance(node, A.Str):
            return repr(node.value)
        if isinstance(node, A.Var):
            return node.name
        if isinstance(node, A.Neg):
            return "(-" + self.expr(node.operand) + ")"
        if isinstance(node, A.Not):
            return "(not _cmc_true(" + self.expr(node.operand) + "))"
        if isinstance(node, A.LogicOp):
            left = "_cmc_true(" + self.expr(node.left) + ")"
            right = "_cmc_true(" + self.expr(node.right) + ")"
            return "(" + left + " " + node.op + " " + right + ")"
        if isinstance(node, A.Compare):
            if node.op == "==":
                return "_cmc_eq(" + self.expr(node.left) + ", " + self.expr(node.right) + ")"
            if node.op == "!=":
                return "(not _cmc_eq(" + self.expr(node.left) + ", " + self.expr(node.right) + "))"
            return (
                '_cmc_cmp("' + node.op + '", ' + self.expr(node.left) + ", " + self.expr(node.right) + ")"
            )
        if isinstance(node, A.InOp):
            return (
                "_cmc_in(" + self.expr(node.item) + ", " + self.expr(node.container)
                + ", " + ("True" if node.negated else "False") + ")"
            )
        if isinstance(node, A.BinOp):
            helper = {"+": "_cmc_plus", "-": "_cmc_minus", "*": "_cmc_times",
                      "/": "_cmc_div", "%": "_cmc_mod"}[node.op]
            return helper + "(" + self.expr(node.left) + ", " + self.expr(node.right) + ")"
        if isinstance(node, A.Index):
            return "_cmc_index(" + self.expr(node.target) + ", " + self.expr(node.index) + ")"
        if isinstance(node, A.Ask):
            if node.prompt is None:
                return "_cmc_ask(None)"
            return "_cmc_ask(" + self.expr(node.prompt) + ")"
        if isinstance(node, A.Snorf):
            return "[" + ", ".join(self.expr(i) for i in node.items) + "]"
        if isinstance(node, A.Call):
            from . import builtins as B

            args = ", ".join(self.expr(a) for a in node.args)
            if node.name in B.IMPLS:
                return "_cmc_" + node.name + "(" + args + ")"
            return node.name + "(" + args + ")"
        return "None"


def to_python_source(source, filename="<cmc>"):
    program = parse_source(source, filename)
    return PythonWriter().write(program, filename=filename)


# ---------------------------------------------------------------------------
# Turning a tree back into CMC text (used by the block editor and tools)
# ---------------------------------------------------------------------------

def expr_to_cmc(node):
    return _expr_text(node, 0)


_PREC = {"or": 1, "and": 2, "not": 3, "cmp": 4, "add": 5, "mul": 6, "unary": 7}


def _maybe_parens(text, prec, parent):
    if prec < parent:
        return "(" + text + ")"
    return text


def _expr_text(node, parent):
    if isinstance(node, A.Num):
        return repr(node.value)
    if isinstance(node, A.Str):
        return '"' + node.value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n") + '"'
    if isinstance(node, A.Var):
        return node.name
    if isinstance(node, A.Neg):
        return _maybe_parens("-" + _expr_text(node.operand, _PREC["unary"]), _PREC["unary"], parent)
    if isinstance(node, A.Not):
        return _maybe_parens("not " + _expr_text(node.operand, _PREC["not"]), _PREC["not"], parent)
    if isinstance(node, A.LogicOp):
        prec = _PREC[node.op]
        text = _expr_text(node.left, prec) + " " + node.op + " " + _expr_text(node.right, prec + 1)
        return _maybe_parens(text, prec, parent)
    if isinstance(node, A.Compare):
        text = _expr_text(node.left, _PREC["add"]) + " " + node.op + " " + _expr_text(node.right, _PREC["add"])
        return _maybe_parens(text, _PREC["cmp"], parent)
    if isinstance(node, A.InOp):
        middle = " not in " if node.negated else " in "
        text = _expr_text(node.item, _PREC["add"]) + middle + _expr_text(node.container, _PREC["add"])
        return _maybe_parens(text, _PREC["cmp"], parent)
    if isinstance(node, A.BinOp):
        prec = _PREC["add"] if node.op in ("+", "-") else _PREC["mul"]
        text = _expr_text(node.left, prec) + " " + node.op + " " + _expr_text(node.right, prec + 1)
        return _maybe_parens(text, prec, parent)
    if isinstance(node, A.Index):
        return _expr_text(node.target, 0) + "[" + _expr_text(node.index, 0) + "]"
    if isinstance(node, A.Ask):
        if node.prompt is None:
            return "blorp()"
        return "blorp " + _expr_text(node.prompt, 0)
    if isinstance(node, A.Snorf):
        return "snorf(" + ", ".join(_expr_text(i, 0) for i in node.items) + ")"
    if isinstance(node, A.Call):
        return node.name + "(" + ", ".join(_expr_text(a, 0) for a in node.args) + ")"
    return "plop"