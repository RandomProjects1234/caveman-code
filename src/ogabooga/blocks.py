"""The block model behind the drag-and-drop editor.

Blocks are little boxes that know how to turns themselves back into CMC text,
and any CMC program can be turned into blocks too (as long as it uses the
blocks that exist so far).
"""

import copy
import itertools
import re

from .boot import cmc

_uid_counter = itertools.count(1)
NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

FORBIDDEN = set(cmc.words.BUILTIN_NAMES) | {"gronk", "nork", "plop", "pi"}

SKRIB_SHAPES = [
    ("clear", []),
    ("size", ["w", "h"]),
    ("color", ["color"]),
    ("dot", ["x", "y", "r"]),
    ("circle", ["x", "y", "r"]),
    ("line", ["x1", "y1", "x2", "y2"]),
    ("box", ["x", "y", "w", "h"]),
    ("blob", ["x", "y", "w", "h"]),
    ("write", ["words", "x", "y"]),
]


def _skrib_specs():
    specs = {}
    defaults = {
        "size": {"w": "400", "h": "400"},
        "color": {"color": '"red"'},
        "dot": {"x": "100", "y": "100", "r": "10"},
        "circle": {"x": "100", "y": "100", "r": "30"},
        "line": {"x1": "0", "y1": "0", "x2": "200", "y2": "200"},
        "box": {"x": "10", "y": "10", "w": "50", "h": "50"},
        "blob": {"x": "10", "y": "10", "w": "50", "h": "50"},
        "write": {"words": '"hi"', "x": "10", "y": "10"},
    }
    for shape, args in SKRIB_SHAPES:
        fields = []
        for arg in args:
            width = 120 if arg == "words" or arg == "color" else 46
            fields.append((arg, arg, "expr", width))
        specs["skrib_" + shape] = {
            "title": "skrib " + shape,
            "category": "Drawing",
            "fields": fields,
            "defaults": defaults.get(shape, {}),
        }
    return specs


SPECS = {
    "say": {
        "title": "oga",
        "category": "Talking",
        "fields": [("expr", "say", "expr", 190)],
        "defaults": {"expr": '"hello cave!"'},
    },
    "ask": {
        "title": "ask",
        "category": "Talking",
        "fields": [("name", "into", "name", 80), ("prompt", "question", "expr", 170)],
        "defaults": {"name": "answer", "prompt": '"What?"'},
    },
    "grunk": {
        "title": "make box",
        "category": "Boxes",
        "fields": [("name", "name", "name", 80), ("expr", "=", "expr", 150)],
        "defaults": {"name": "x", "expr": "5"},
    },
    "set": {
        "title": "change box",
        "category": "Boxes",
        "fields": [("name", "name", "name", 80), ("expr", "=", "expr", 150)],
        "defaults": {"name": "x", "expr": "6"},
    },
    "set_index": {
        "title": "change a spot",
        "category": "Boxes",
        "fields": [("target", "pile", "expr", 80), ("index", "spot", "expr", 44), ("value", "=", "expr", 110)],
        "defaults": {"target": "pets", "index": "0", "value": "1"},
    },
    "if": {
        "title": "binga",
        "category": "Choices",
        "fields": [("cond", "check", "expr", 190)],
        "container": "if",
        "defaults": {"cond": "gronk"},
    },
    "repeat": {
        "title": "booga",
        "category": "Loops",
        "fields": [("count", "times", "expr", 56)],
        "container": "body",
        "defaults": {"count": "3"},
    },
    "while": {
        "title": "zug",
        "category": "Loops",
        "fields": [("cond", "while", "expr", 180)],
        "container": "body",
        "defaults": {"cond": "gronk"},
    },
    "foreach": {
        "title": "zoop each",
        "category": "Loops",
        "fields": [("name", "box", "name", 80), ("iterable", "in", "expr", 150)],
        "container": "body",
        "defaults": {"name": "thing", "iterable": "pets"},
    },
    "clump": {
        "title": "clump",
        "category": "Clumps",
        "fields": [("name", "name", "name", 90), ("params", "boxes", "params", 130)],
        "container": "body",
        "defaults": {"name": "myword", "params": ""},
    },
    "ork": {
        "title": "give back",
        "category": "Clumps",
        "fields": [("expr", "answer", "expr", 150)],
        "defaults": {"expr": ""},
    },
    "call": {
        "title": "call clump",
        "category": "Clumps",
        "fields": [("name", "name", "name", 90), ("args", "with", "args", 150)],
        "defaults": {"name": "myword", "args": ""},
    },
    "pile_make": {
        "title": "make pile",
        "category": "Piles",
        "fields": [("name", "name", "name", 80), ("items", "things", "args", 150)],
        "defaults": {"name": "pets", "items": '"dog", "cat"'},
    },
    "pile_plop": {
        "title": "plop into pile",
        "category": "Piles",
        "fields": [("pile", "pile", "expr", 70), ("item", "thing", "expr", 130)],
        "defaults": {"pile": "pets", "item": '"fish"'},
    },
    "pile_yoink": {
        "title": "yoink from pile",
        "category": "Piles",
        "fields": [("name", "into", "name", 80), ("pile", "pile", "expr", 110)],
        "defaults": {"name": "taken", "pile": "pets"},
    },
    "value_number": {
        "title": "number",
        "category": "Values",
        "value": True,
        "fields": [("value", "", "number", 60)],
        "defaults": {"value": "1"},
    },
    "value_text": {
        "title": "text",
        "category": "Values",
        "value": True,
        "fields": [("value", "", "text", 130)],
        "defaults": {"value": "hello"},
    },
    "value_var": {
        "title": "box value",
        "category": "Values",
        "value": True,
        "fields": [("name", "", "name", 90)],
        "defaults": {"name": "x"},
    },
    "value_random": {
        "title": "random",
        "category": "Values",
        "value": True,
        "fields": [("low", "", "expr", 46), ("high", "", "expr", 46)],
        "defaults": {"low": "1", "high": "6"},
    },
}

SPECS.update(_skrib_specs())

CATEGORY_ORDER = ["Talking", "Boxes", "Choices", "Loops", "Clumps", "Values", "Piles", "Drawing"]


class BlockError(Exception):
    """Something cannot become a block (yet)."""


class Block:
    def __init__(self, kind, fields=None, body=None, else_body=None):
        self.kind = kind
        self.fields = dict(fields or {})
        self.body = list(body or [])
        self.else_body = list(else_body or [])
        self.uid = next(_uid_counter)

    def field(self, key):
        return self.fields.get(key, "")

    def spec(self):
        return SPECS[self.kind]

    def is_value(self):
        return bool(SPECS[self.kind].get("value"))

    def color(self):
        from . import theme

        return theme.block_color(self.kind)

    def title(self):
        return SPECS[self.kind]["title"]

    def __repr__(self):
        return "Block(" + self.kind + ", " + repr(self.fields) + ")"


def new_block(kind):
    spec = SPECS[kind]
    defaults = spec.get("defaults", {})
    fields = {key: defaults.get(key, "") for key, _label, _ftype, _w in spec["fields"]}
    return Block(kind, fields)


def clone(block):
    copied = Block(block.kind, dict(block.fields))
    copied.body = [clone(child) for child in block.body]
    copied.else_body = [clone(child) for child in block.else_body]
    return copied


def clone_all(blocks):
    return [clone(block) for block in blocks]


def all_blocks(blocks):
    for block in blocks:
        yield block
        for child in all_blocks(block.body):
            yield child
        for child in all_blocks(block.else_body):
            yield child


def value_expression(block):
    """The expression text a value block becomes when dropped in a slot."""
    if block.kind == "value_number":
        return block.field("value").strip() or "0"
    if block.kind == "value_text":
        text = block.field("value")
        stripped = text.strip()
        if len(stripped) >= 2 and stripped[0] in "\"'" and stripped[-1] == stripped[0]:
            return stripped
        return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'
    if block.kind == "value_var":
        return block.field("name").strip()
    if block.kind == "value_random":
        return "munga(" + (block.field("low").strip() or "1") + ", " + (block.field("high").strip() or "6") + ")"
    return ""


# -------------------------------------------------------------------- text

def _line(block):
    kind = block.kind
    f = block.field
    if kind == "say":
        return "oga " + f("expr")
    if kind == "ask":
        prompt = f("prompt").strip()
        if prompt:
            return "grunk " + f("name") + " = blorp " + prompt
        return "grunk " + f("name") + " = blorp()"
    if kind == "grunk":
        return "grunk " + f("name") + " = " + f("expr")
    if kind == "set":
        return f("name") + " = " + f("expr")
    if kind == "set_index":
        return f("target") + "[" + f("index") + "] = " + f("value")
    if kind == "repeat":
        return "booga " + f("count")
    if kind == "while":
        return "zug " + f("cond")
    if kind == "foreach":
        return "zoop " + f("name") + " in " + f("iterable")
    if kind == "clump":
        return "clump " + f("name") + "(" + f("params") + ")"
    if kind == "ork":
        answer = f("expr").strip()
        return "ork" + (" " + answer if answer else "")
    if kind == "call":
        return f("name") + "(" + f("args") + ")"
    if kind == "pile_make":
        return "grunk " + f("name") + " = snorf(" + f("items") + ")"
    if kind == "pile_plop":
        return "plop(" + f("pile") + ", " + f("item") + ")"
    if kind == "pile_yoink":
        return "grunk " + f("name") + " = yoink(" + f("pile") + ")"
    if kind.startswith("skrib_"):
        shape = kind[len("skrib_"):]
        args = [f(key).strip() for key, _l, _t, _w in SPECS[kind]["fields"]]
        if shape == "clear" or not args:
            return "skrib clear" if shape == "clear" else "skrib " + shape
        return "skrib " + shape + " " + ", ".join(args)
    raise BlockError("I do not know how to write the block '" + kind + "' yet.")


def _if_lines(block, indent, chained=False):
    pad = "    " * indent
    lines = [pad + ("wonga binga " if chained else "binga ") + block.field("cond")]
    lines.extend(_block_lines(block.body, indent + 1))
    tail = block.else_body
    if len(tail) == 1 and tail[0].kind == "if":
        lines.extend(_if_lines(tail[0], indent, chained=True))
    elif tail:
        lines.append(pad + "wonga")
        lines.extend(_block_lines(tail, indent + 1))
    if not chained:
        lines.append(pad + "unga")
    return lines


def _block_lines(blocks, indent):
    lines = []
    for block in blocks:
        pad = "    " * indent
        if block.kind == "if":
            lines.extend(_if_lines(block, indent))
            continue
        if block.kind in ("repeat", "while", "foreach", "clump"):
            lines.append(pad + _line(block))
            lines.extend(_block_lines(block.body, indent + 1))
            lines.append(pad + "unga")
            continue
        if block.is_value():
            raise BlockError("A value block like 'number' cannot sit on its own. Put it inside a slot.")
        lines.append(pad + _line(block))
    return lines


def blocks_to_source(blocks):
    lines = _block_lines(blocks, 0)
    return ("\n".join(lines) + "\n") if lines else ""


# --------------------------------------------------------------- checks

def _check_name(block, key, label, problems):
    name = block.field(key).strip()
    if not name:
        problems.append("A " + block.title() + " block is missing its " + label + ".")
    elif not NAME_RE.match(name):
        problems.append("'" + name + "' cannot be a " + label + " (use letters, numbers, _).")
    elif name.lower() in FORBIDDEN:
        problems.append("'" + name + "' already means something in CMC.")
    elif name in ("lap",):
        problems.append("'lap' is the magic repeat box. Pick another name.")


def check_blocks(blocks):
    """Return a list of friendly complaints (empty when all good)."""
    problems = []
    for block in all_blocks(blocks):
        if block.kind in ("grunk", "set", "ask", "foreach", "pile_make", "pile_yoink"):
            _check_name(block, "name", "box name", problems)
        if block.kind == "clump":
            _check_name(block, "name", "clump name", problems)
            params = block.field("params").strip()
            if params:
                for piece in params.split(","):
                    piece = piece.strip()
                    if not NAME_RE.match(piece) or piece.lower() in FORBIDDEN:
                        problems.append("'" + piece + "' is not a good box name in clump boxes.")
        if block.kind == "call":
            _check_name(block, "name", "clump name", problems)
        if block.kind == "set_index":
            if not block.field("target").strip():
                problems.append("A 'change a spot' block needs a pile name.")
    return problems


def validate_source(source):
    """Try to parse generated text; return an error message or None."""
    try:
        cmc.parse_source(source)
    except cmc.CmcError as error:
        return error.format()
    return None


# ------------------------------------------------------- text -> blocks

def _expr_text(node):
    return cmc.expr_to_cmc(node)


def _skrib_to_block(node):
    shape = node.shape
    kind = "skrib_" + shape
    if kind not in SPECS:
        raise BlockError("skrib " + shape + " has no block yet.")
    block = new_block(kind)
    keys = [key for key, _l, _t, _w in SPECS[kind]["fields"]]
    for key, arg in zip(keys, node.args):
        block.fields[key] = _expr_text(arg)
    return block


def _statement_to_block(statement):
    node = statement
    if type(node).__name__ == "Say":
        block = new_block("say")
        block.fields["expr"] = ", ".join(_expr_text(e) for e in node.exprs)
        return block
    if type(node).__name__ == "Assign":
        if type(node.expr).__name__ == "Ask":
            if not node.declare:
                raise BlockError("A blorp question must be put into a new box with 'grunk'.")
            block = new_block("ask")
            block.fields["name"] = node.name
            block.fields["prompt"] = "" if node.expr.prompt is None else _expr_text(node.expr.prompt)
            return block
        block = new_block("grunk" if node.declare else "set")
        block.fields["name"] = node.name
        block.fields["expr"] = _expr_text(node.expr)
        return block
    if type(node).__name__ == "SetIndex":
        block = new_block("set_index")
        block.fields["target"] = _expr_text(node.target)
        block.fields["index"] = _expr_text(node.index)
        block.fields["value"] = _expr_text(node.value)
        return block
    if type(node).__name__ == "If":
        block = new_block("if")
        block.fields["cond"] = _expr_text(node.cond)
        block.body = [_statement_to_block(s) for s in node.body]
        block.else_body = [_statement_to_block(s) for s in node.else_body]
        return block
    if type(node).__name__ == "Repeat":
        block = new_block("repeat")
        block.fields["count"] = _expr_text(node.count)
        block.body = [_statement_to_block(s) for s in node.body]
        return block
    if type(node).__name__ == "While":
        block = new_block("while")
        block.fields["cond"] = _expr_text(node.cond)
        block.body = [_statement_to_block(s) for s in node.body]
        return block
    if type(node).__name__ == "ForEach":
        block = new_block("foreach")
        block.fields["name"] = node.name
        block.fields["iterable"] = _expr_text(node.iterable)
        block.body = [_statement_to_block(s) for s in node.body]
        return block
    if type(node).__name__ == "FuncDef":
        block = new_block("clump")
        block.fields["name"] = node.name
        block.fields["params"] = ", ".join(node.params)
        block.body = [_statement_to_block(s) for s in node.body]
        return block
    if type(node).__name__ == "Return":
        block = new_block("ork")
        block.fields["expr"] = "" if node.expr is None else _expr_text(node.expr)
        return block
    if type(node).__name__ == "ExprStmt":
        call = node.expr
        if type(call).__name__ != "Call":
            raise BlockError("That line cannot become a block yet.")
        if call.name == "plop" and len(call.args) == 2:
            block = new_block("pile_plop")
            block.fields["pile"] = _expr_text(call.args[0])
            block.fields["item"] = _expr_text(call.args[1])
            return block
        block = new_block("call")
        block.fields["name"] = call.name
        block.fields["args"] = ", ".join(_expr_text(a) for a in call.args)
        return block
    if type(node).__name__ == "Skrib":
        return _skrib_to_block(node)
    raise BlockError("That line cannot become a block yet: " + type(node).__name__)


def source_to_blocks(source):
    """Turn CMC text into blocks, or raise BlockError with a friendly note."""
    try:
        program = cmc.parse_source(source)
    except cmc.CmcError as error:
        raise BlockError("The text has a mistake first:\n" + error.format())
    blocks = []
    for statement in program.statements:
        if type(statement).__name__ == "Assign" and type(statement.expr).__name__ == "Call":
            call = statement.expr
            if call.name == "snorf" and statement.declare:
                block = new_block("pile_make")
                block.fields["name"] = statement.name
                block.fields["items"] = ", ".join(_expr_text(a) for a in call.args)
                blocks.append(block)
                continue
            if call.name == "yoink" and statement.declare and len(call.args) == 1:
                block = new_block("pile_yoink")
                block.fields["name"] = statement.name
                block.fields["pile"] = _expr_text(call.args[0])
                blocks.append(block)
                continue
        blocks.append(_statement_to_block(statement))
    return blocks


def describe_problems(problems):
    if not problems:
        return ""
    return "\n".join("  - " + p for p in problems)