"""The CMC tree shapes (AST).

The parser builds these, the interpreter walks them, and the code writer
turns them into Python.  Every node carries the line it came from so errors
can point at the right cave wall.
"""


class Node:
    line = 0


# ---------------------------------------------------------------- program

class Program(Node):
    def __init__(self, statements, line=0):
        self.statements = statements
        self.line = line


# ------------------------------------------------------------- statements

class Say(Node):
    """oga <things>"""

    def __init__(self, exprs, line=0):
        self.exprs = exprs
        self.line = line


class Assign(Node):
    """grunk x = thing   (declare=True)   /   x = thing (declare=False)"""

    def __init__(self, name, expr, declare, line=0):
        self.name = name
        self.expr = expr
        self.declare = declare
        self.line = line


class SetIndex(Node):
    """pile[2] = thing"""

    def __init__(self, target, index, value, line=0):
        self.target = target
        self.index = index
        self.value = value
        self.line = line


class If(Node):
    def __init__(self, cond, body, else_body, line=0):
        self.cond = cond
        self.body = body
        self.else_body = else_body  # list (possibly empty)
        self.line = line


class Repeat(Node):
    """booga <count> ... unga"""

    def __init__(self, count, body, line=0):
        self.count = count
        self.body = body
        self.line = line


class While(Node):
    def __init__(self, cond, body, line=0):
        self.cond = cond
        self.body = body
        self.line = line


class ForEach(Node):
    """zoop name in pile ... unga"""

    def __init__(self, name, iterable, body, line=0):
        self.name = name
        self.iterable = iterable
        self.body = body
        self.line = line


class FuncDef(Node):
    def __init__(self, name, params, body, line=0):
        self.name = name
        self.params = params
        self.body = body
        self.line = line


class Return(Node):
    def __init__(self, expr, line=0):
        self.expr = expr  # may be None
        self.line = line


class ExprStmt(Node):
    def __init__(self, expr, line=0):
        self.expr = expr
        self.line = line


class Skrib(Node):
    """skrib <shape> <args...>"""

    def __init__(self, shape, args, line=0):
        self.shape = shape
        self.args = args
        self.line = line


# ------------------------------------------------------------- expressions

class Num(Node):
    def __init__(self, value, line=0):
        self.value = value
        self.line = line


class Str(Node):
    def __init__(self, value, line=0):
        self.value = value
        self.line = line


class Var(Node):
    def __init__(self, name, line=0):
        self.name = name
        self.line = line


class BinOp(Node):
    def __init__(self, op, left, right, line=0):
        self.op = op
        self.left = left
        self.right = right
        self.line = line


class Neg(Node):
    def __init__(self, operand, line=0):
        self.operand = operand
        self.line = line


class LogicOp(Node):
    """and / or"""

    def __init__(self, op, left, right, line=0):
        self.op = op
        self.left = left
        self.right = right
        self.line = line


class Not(Node):
    def __init__(self, operand, line=0):
        self.operand = operand
        self.line = line


class Compare(Node):
    def __init__(self, op, left, right, line=0):
        self.op = op
        self.left = left
        self.right = right
        self.line = line


class InOp(Node):
    def __init__(self, item, container, negated=False, line=0):
        self.item = item
        self.container = container
        self.negated = negated
        self.line = line


class Call(Node):
    def __init__(self, name, args, line=0):
        self.name = name
        self.args = args
        self.line = line


class Index(Node):
    def __init__(self, target, index, line=0):
        self.target = target
        self.index = index
        self.line = line


class Ask(Node):
    """blorp <prompt>"""

    def __init__(self, prompt, line=0):
        self.prompt = prompt  # expression or None
        self.line = line


class Snorf(Node):
    def __init__(self, items, line=0):
        self.items = items
        self.line = line