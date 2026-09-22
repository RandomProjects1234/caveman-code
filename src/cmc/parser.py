"""Turn tokens into the CMC tree.

Reads one line at a time, keeps track of open blocks, and shouts OOGA! with a
kind hint whenever the code does not make sense.
"""

from .errors import CmcParseError
from .lexer import Lexer
from . import ast_nodes as A
from . import words

SKRIB_SHAPES = {
    "clear": 0,
    "size": 2,
    "color": 1,
    "dot": 3,
    "circle": 3,
    "line": 4,
    "box": 4,
    "blob": 4,
    "write": 3,
}

FORBIDDEN_NAMES = set(words.BUILTIN_NAMES) | {"gronk", "nork", "plop", "pi"}

COMPARE_OPS = ("==", "!=", "<", "<=", ">", ">=")


class Parser:
    def __init__(self, tokens, filename="<cmc>"):
        self.tokens = tokens
        self.pos = 0
        self.filename = filename
        self.open_blocks = 0

    # ------------------------------------------------------------ helpers

    def peek(self, ahead=0):
        spot = self.pos + ahead
        if spot >= len(self.tokens):
            return self.tokens[-1]
        return self.tokens[spot]

    def advance(self):
        tok = self.peek()
        if tok.kind != "eof":
            self.pos += 1
        return tok

    def at_kw(self, *names):
        tok = self.peek()
        return tok.kind == "kw" and tok.value in names

    def match_kw(self, *names):
        if self.at_kw(*names):
            return self.advance()
        return None

    def expect_kw(self, name, message=None, hint=None):
        tok = self.peek()
        if tok.kind == "kw" and tok.value == name:
            return self.advance()
        if message is None:
            message = "I expected '" + name + "' here, but found " + tok.describe() + "."
        raise CmcParseError(message, line=tok.line, hint=hint)

    def expect_eol(self, what="this line"):
        tok = self.peek()
        if tok.kind in ("nl", "eof"):
            return
        raise CmcParseError(
            "There are extra things on " + what + " that I do not understand: " + tok.describe() + ".",
            line=tok.line,
            hint="one thing per line. Press Enter and try the next part there.",
        )

    def expect_name(self, message, hint=None):
        tok = self.peek()
        if tok.kind == "name":
            return self.advance()
        raise CmcParseError(message, line=tok.line, hint=hint)

    def expect_kind(self, kind, message, hint=None):
        tok = self.peek()
        if tok.kind == kind:
            return self.advance()
        raise CmcParseError(message, line=tok.line, hint=hint)

    def check_name_ok(self, name, line):
        if name.lower() in FORBIDDEN_NAMES:
            raise CmcParseError(
                "'" + name + "' already means something in CMC, so it cannot be a name you make up.",
                line=line,
                hint="try adding a letter or number: " + name + "1",
            )

    def error(self, tok, message, hint=None):
        raise CmcParseError(message, line=tok.line, hint=hint)

    def skip_newlines(self):
        while self.peek().kind == "nl":
            self.advance()

    # ------------------------------------------------------------ program

    def parse_program(self):
        statements = []
        self.skip_newlines()
        while self.peek().kind != "eof":
            statements.append(self.parse_statement())
            self.expect_eol()
            self.skip_newlines()
        return A.Program(statements)

    # --------------------------------------------------------- statements

    def parse_statement(self):
        tok = self.peek()

        if tok.kind == "kw":
            value = tok.value
            if value == "oga":
                return self.parse_say()
            if value == "grunk":
                return self.parse_grunk()
            if value == "binga":
                return self.parse_if()
            if value == "booga":
                return self.parse_repeat()
            if value == "zug":
                return self.parse_while()
            if value == "zoop":
                return self.parse_foreach()
            if value == "clump":
                return self.parse_clump()
            if value == "ork":
                return self.parse_return()
            if value == "skrib":
                return self.parse_skrib()
            if value in ("unga", "wonga"):
                self.error(
                    tok,
                    "You said '" + value + "', but nothing was open for it to close.",
                    hint="every 'unga' closes one 'binga', 'booga', 'zug', 'zoop', or 'clump'.",
                )
            if value in ("and", "or", "in"):
                self.error(
                    tok,
                    "'" + value + "' is a joining word, so a line cannot start with it.",
                    hint="put something in front of it, like: apples > 2 " + value + " pears > 1",
                )

        # A name followed by '=' is changing a box.
        if tok.kind == "name" and self.peek(1).kind == "op" and self.peek(1).value == "=":
            return self.parse_assignment()

        # Otherwise it is an expression used as a line: usually calling a clump.
        expr = self.parse_expression()
        if self.peek().kind == "op" and self.peek().value == "=":
            if isinstance(expr, A.Index):
                self.advance()
                value = self.parse_expression()
                return A.SetIndex(expr.target, expr.index, value, line=expr.line)
        if isinstance(expr, (A.Call, A.Ask)):
            return A.ExprStmt(expr, line=expr.line)
        self.error(
            tok,
            "This line makes an answer (" + tok.describe() + ") but does nothing with it.",
            hint="use 'oga' to show it, or 'grunk x = ...' to keep it in a box.",
        )

    def parse_say(self):
        line = self.advance().line
        exprs = [self.parse_expression()]
        while self.peek().kind == "comma":
            self.advance()
            exprs.append(self.parse_expression())
        return A.Say(exprs, line=line)

    def parse_grunk(self):
        line = self.advance().line
        name_tok = self.expect_name(
            "After 'grunk' comes the name of the new box.",
            hint="like this: grunk x = 5",
        )
        self.check_name_ok(name_tok.value, name_tok.line)
        op = self.peek()
        if not (op.kind == "op" and op.value == "="):
            self.error(
                op,
                "The box '" + name_tok.value + "' needs a '=' and something to put in it.",
                hint="like this: grunk " + name_tok.value + " = 5",
            )
        self.advance()
        expr = self.parse_expression()
        return A.Assign(name_tok.value, expr, True, line=line)

    def parse_assignment(self):
        name_tok = self.advance()
        self.advance()  # the '='
        expr = self.parse_expression()
        return A.Assign(name_tok.value, expr, False, line=name_tok.line)

    def parse_if(self):
        line = self.advance().line
        cond = self.parse_expression()
        self.expect_eol("the 'binga' line")
        body = self.parse_block({"wonga", "unga"})
        else_body = []
        if self.match_kw("wonga"):
            if self.at_kw("binga"):
                nested = self.parse_if()  # consumes its own unga
                else_body = [nested]
                return A.If(cond, body, else_body, line=line)
            self.expect_eol("the 'wonga' line")
            else_body = self.parse_block({"unga"})
        self.expect_kw("unga")
        return A.If(cond, body, else_body, line=line)

    def parse_repeat(self):
        line = self.advance().line
        count = self.parse_expression()
        self.expect_eol("the 'booga' line")
        body = self.parse_block({"unga"})
        self.expect_kw("unga")
        return A.Repeat(count, body, line=line)

    def parse_while(self):
        line = self.advance().line
        cond = self.parse_expression()
        self.expect_eol("the 'zug' line")
        body = self.parse_block({"unga"})
        self.expect_kw("unga")
        return A.While(cond, body, line=line)

    def parse_foreach(self):
        line = self.advance().line
        name_tok = self.expect_name(
            "A 'zoop' needs a name for the loop box, like: zoop pet in pets",
            hint="the loop box holds one thing each lap.",
        )
        self.check_name_ok(name_tok.value, name_tok.line)
        self.expect_kw(
            "in",
            "A 'zoop' needs the little word 'in', like: zoop pet in pets",
        )
        iterable = self.parse_expression()
        self.expect_eol("the 'zoop' line")
        body = self.parse_block({"unga"})
        self.expect_kw("unga")
        return A.ForEach(name_tok.value, iterable, body, line=line)

    def parse_clump(self):
        line = self.advance().line
        name_tok = self.expect_name(
            "After 'clump' comes the name of your new word.",
            hint="like this: clump double(n)",
        )
        self.check_name_ok(name_tok.value, name_tok.line)
        self.expect_kind("lparen", "A clump name needs '(' after it, like: clump double(n)")
        params = []
        if self.peek().kind != "rparen":
            while True:
                p = self.expect_name(
                    "Inside the parentheses come box names, separated by commas.",
                    hint="like this: clump add(a, b)",
                )
                self.check_name_ok(p.value, p.line)
                params.append(p.value)
                if self.peek().kind == "comma":
                    self.advance()
                    continue
                break
        self.expect_kind("rparen", "This clump's box list needs a ')' to close it.")
        self.expect_eol("the 'clump' line")
        body = self.parse_block({"unga"})
        self.expect_kw("unga")
        return A.FuncDef(name_tok.value, params, body, line=line)

    def parse_return(self):
        tok = self.advance()
        if self.peek().kind in ("nl", "eof"):
            return A.Return(None, line=tok.line)
        expr = self.parse_expression()
        return A.Return(expr, line=tok.line)

    def parse_skrib(self):
        line = self.advance().line
        shape_tok = self.peek()
        if shape_tok.kind != "name":
            self.error(
                shape_tok,
                "After 'skrib' comes what to draw, but I found " + shape_tok.describe() + ".",
                hint="try: skrib circle 100, 100, 30",
            )
        self.advance()
        shape = shape_tok.value.lower()
        if shape not in SKRIB_SHAPES:
            self.error(
                shape_tok,
                "'skrib' does not know how to draw '" + shape_tok.value + "'.",
                hint="it knows: circle, dot, line, box, blob, write, color, size, clear.",
            )
        args = []
        while self.peek().kind not in ("nl", "eof"):
            if self.peek().kind == "comma":
                self.advance()
                continue
            args.append(self.parse_expression())
        need = SKRIB_SHAPES[shape]
        if len(args) != need:
            self.error(
                shape_tok,
                "'skrib " + shape + "' needs " + str(need) + " number" + ("s" if need != 1 else "")
                + " after it, but I counted " + str(len(args)) + ".",
                hint="put commas between them: " + self._skrib_hint(shape),
            )
        return A.Skrib(shape, args, line=line)

    @staticmethod
    def _skrib_hint(shape):
        examples = {
            "clear": "skrib clear",
            "size": "skrib size 400, 400",
            "color": 'skrib color "red"',
            "dot": "skrib dot 100, 100, 10",
            "circle": "skrib circle 100, 100, 30",
            "line": "skrib line 0, 0, 200, 200",
            "box": "skrib box 10, 10, 50, 50",
            "blob": "skrib blob 10, 10, 50, 50",
            "write": 'skrib write "hi", 10, 10',
        }
        return examples.get(shape, "")

    def parse_block(self, terminators):
        body = []
        self.skip_newlines()
        open_tok = self.peek()
        while True:
            tok = self.peek()
            if tok.kind == "kw" and tok.value in terminators:
                return body
            if tok.kind == "eof":
                raise CmcParseError(
                    "A block was opened but never closed.",
                    line=open_tok.line,
                    hint="every 'binga', 'booga', 'zug', 'zoop', and 'clump' block ends with 'unga'.",
                )
            body.append(self.parse_statement())
            self.expect_eol()
            self.skip_newlines()

    # -------------------------------------------------------- expressions

    def parse_expression(self):
        return self.parse_or()

    def parse_or(self):
        left = self.parse_and()
        while self.at_kw("or"):
            self.advance()
            right = self.parse_and()
            left = A.LogicOp("or", left, right, line=left.line)
        return left

    def parse_and(self):
        left = self.parse_not()
        while self.at_kw("and"):
            self.advance()
            right = self.parse_not()
            left = A.LogicOp("and", left, right, line=left.line)
        return left

    def parse_not(self):
        tok = self.peek()
        if tok.kind == "kw" and tok.value == "not":
            self.advance()
            operand = self.parse_not()
            return A.Not(operand, line=tok.line)
        return self.parse_comparison()

    def parse_comparison(self):
        left = self.parse_additive()

        if self.at_kw("not") and self.peek(1).kind == "kw" and self.peek(1).value == "in":
            self.advance()
            self.advance()
            container = self.parse_additive()
            return A.InOp(left, container, True, line=left.line)

        tok = self.peek()
        if tok.kind == "op" and tok.value in COMPARE_OPS:
            self.advance()
            right = self.parse_additive()
            node = A.Compare(tok.value, left, right, line=tok.line)
            nxt = self.peek()
            if nxt.kind == "op" and nxt.value in COMPARE_OPS:
                raise CmcParseError(
                    "One compare at a time, please!",
                    line=nxt.line,
                    hint="compare two things, then join checks with 'and' or 'or'.",
                )
            return node

        if self.at_kw("in"):
            self.advance()
            container = self.parse_additive()
            return A.InOp(left, container, False, line=left.line)

        return left

    def parse_additive(self):
        left = self.parse_multiplicative()
        while True:
            tok = self.peek()
            if tok.kind == "op" and tok.value in ("+", "-"):
                self.advance()
                right = self.parse_multiplicative()
                left = A.BinOp(tok.value, left, right, line=tok.line)
            else:
                return left

    def parse_multiplicative(self):
        left = self.parse_unary()
        while True:
            tok = self.peek()
            if tok.kind == "op" and tok.value in ("*", "/", "%"):
                self.advance()
                right = self.parse_unary()
                left = A.BinOp(tok.value, left, right, line=tok.line)
            else:
                return left

    def parse_unary(self):
        tok = self.peek()
        if tok.kind == "op" and tok.value == "-":
            self.advance()
            operand = self.parse_unary()
            return A.Neg(operand, line=tok.line)
        return self.parse_postfix()

    def parse_postfix(self):
        expr = self.parse_primary()
        while self.peek().kind == "lbracket":
            self.advance()
            index = self.parse_expression()
            close = self.peek()
            if close.kind != "rbracket":
                self.error(
                    close,
                    "A spot number needs a ']' after it.",
                    hint="like this: pets[0]",
                )
            self.advance()
            expr = A.Index(expr, index, line=expr.line)
        return expr

    def parse_args(self):
        self.advance()  # the '('
        args = []
        if self.peek().kind == "rparen":
            self.advance()
            return args
        while True:
            args.append(self.parse_expression())
            if self.peek().kind == "comma":
                self.advance()
                continue
            break
        close = self.peek()
        if close.kind != "rparen":
            self.error(
                close,
                "This list of things needs a ')' to close it.",
                hint="count the parentheses -- every '(' needs a ')'.",
            )
        self.advance()
        return args

    def parse_primary(self):
        tok = self.advance()

        if tok.kind == "num":
            return A.Num(tok.value, line=tok.line)
        if tok.kind == "str":
            return A.Str(tok.value, line=tok.line)
        if tok.kind == "lparen":
            expr = self.parse_expression()
            close = self.peek()
            if close.kind != "rparen":
                self.error(
                    close,
                    "This '(' needs a ')' to close it.",
                    hint="count the parentheses!",
                )
            self.advance()
            return expr
        if tok.kind == "name":
            if self.peek().kind == "lparen":
                args = self.parse_args()
                return A.Call(tok.value, args, line=tok.line)
            return A.Var(tok.value, line=tok.line)
        if tok.kind == "kw" and tok.value == "blorp":
            if self.peek().kind == "lparen":
                args = self.parse_args()
                if len(args) > 1:
                    self.error(
                        tok,
                        "'blorp' wants one question, not " + str(len(args)) + ".",
                        hint='like this: blorp "What is your name?"',
                    )
                return A.Ask(args[0] if args else None, line=tok.line)
            if self.peek().kind in ("nl", "eof", "comma", "rparen", "rbracket"):
                return A.Ask(None, line=tok.line)
            prompt = self.parse_expression_limit()
            return A.Ask(prompt, line=tok.line)

        if tok.kind == "kw":
            if tok.value in ("unga", "wonga", "booga", "binga", "zug", "zoop", "clump", "grunk", "oga", "skrib"):
                self.error(
                    tok,
                    "'" + tok.value + "' is a doing word for starting a line, not a thing to use here.",
                    hint="write a number, some text, or a box name instead.",
                )
            self.error(tok, "I do not know how to use '" + tok.value + "' here.")

        self.error(
            tok,
            "I expected something to use here, but found " + tok.describe() + ".",
            hint="try a number like 5, text like \"hi\", or a box name.",
        )

    def parse_expression_limit(self):
        """Parse an expression but stop at a low precedence so calling code
        can keep going (used for the blorp question)."""
        return self.parse_expression()


def parse_source(source, filename="<cmc>"):
    tokens = Lexer(source, filename).tokenize()
    return Parser(tokens, filename).parse_program()


def parse_expression_source(source, filename="<cmc>"):
    """Parse a single expression (used by the IDE's block editor)."""
    tokens = Lexer(source, filename).tokenize()
    parser = Parser(tokens, filename)
    expr = parser.parse_expression()
    tok = parser.peek()
    if tok.kind not in ("nl", "eof"):
        raise CmcParseError(
            "There are extra things after the expression: " + tok.describe() + ".",
            line=tok.line,
        )
    return expr