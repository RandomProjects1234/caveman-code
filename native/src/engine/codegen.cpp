#include "codegen.hpp"

#include <unordered_map>
#include <vector>

#include "parser.hpp"
#include "textutil.hpp"
#include "words.hpp"

namespace cmc {

namespace {

const char* PRELUDE = R"CMC_PRELUDE(# ---------------------------------------------------------------------
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
)CMC_PRELUDE";

std::string prelude_text() {
    std::string text = PRELUDE;
    std::string needle = "{version}";
    std::size_t spot = text.find(needle);
    if (spot != std::string::npos) {
        text.replace(spot, needle.size(), words::VERSION);
    }
    return text;
}

class PythonWriter {
public:
    std::string write(const Program& program, const std::string& filename) {
        lines_.clear();
        indent_ = 0;
        emit("# " + std::string(68, '='));
        emit("# This file was made by Cave Man Code (CMC) v" + std::string(words::VERSION));
        if (!filename.empty() && filename != "<cmc>") {
            emit("# From: " + filename);
        }
        emit("# Feel free to read it and change it -- it is normal Python!");
        emit("# " + std::string(68, '='));
        emit("");
        emit(prelude_text());
        emit("");
        emit("# ---------------------- your program starts here ---------------------");
        emit("");
        indent_ = 0;
        for (const auto& stmt : program.statements) {
            statement(stmt.get());
        }
        emit("");
        emit("# keep a picture window open if the program drew something");
        emit("if _PEN.used:");
        emit("    _cmc_hold()");
        std::string out;
        for (std::size_t i = 0; i < lines_.size(); ++i) {
            if (i > 0) out += "\n";
            out += lines_[i];
        }
        out += "\n";
        return out;
    }

private:
    void emit(const std::string& text) {
        if (text.empty()) {
            lines_.push_back("");
        } else {
            lines_.push_back(std::string(static_cast<std::size_t>(indent_) * 4, ' ') + text);
        }
    }

    void statement(const Stmt* node) {
        if (auto* n = dynamic_cast<const Say*>(node)) {
            std::string text = "_cmc_say(";
            for (std::size_t i = 0; i < n->exprs.size(); ++i) {
                if (i > 0) text += ", ";
                text += expr(n->exprs[i].get());
            }
            emit(text + ")");
            return;
        }
        if (auto* n = dynamic_cast<const Assign*>(node)) {
            emit(n->name + " = " + expr(n->expr.get()));
            return;
        }
        if (auto* n = dynamic_cast<const SetIndex*>(node)) {
            emit("_cmc_set_index(" + expr(n->target.get()) + ", " + expr(n->index.get()) + ", "
                 + expr(n->value.get()) + ")");
            return;
        }
        if (auto* n = dynamic_cast<const If*>(node)) {
            emit_if(n);
            return;
        }
        if (auto* n = dynamic_cast<const Repeat*>(node)) {
            emit("for lap in _cmc_laps(" + expr(n->count.get()) + "):");
            body(n->body);
            return;
        }
        if (auto* n = dynamic_cast<const While*>(node)) {
            emit("while _cmc_true(" + expr(n->cond.get()) + "):");
            body(n->body);
            return;
        }
        if (auto* n = dynamic_cast<const ForEach*>(node)) {
            emit("for " + n->name + " in _cmc_iter(" + expr(n->iterable.get()) + "):");
            body(n->body);
            return;
        }
        if (auto* n = dynamic_cast<const FuncDef*>(node)) {
            std::string params;
            for (std::size_t i = 0; i < n->params.size(); ++i) {
                if (i > 0) params += ", ";
                params += n->params[i];
            }
            emit("def " + n->name + "(" + params + "):");
            body(n->body, true);
            return;
        }
        if (auto* n = dynamic_cast<const Return*>(node)) {
            if (n->expr) {
                emit("return " + expr(n->expr.get()));
            } else {
                emit("return None");
            }
            return;
        }
        if (auto* n = dynamic_cast<const ExprStmt*>(node)) {
            emit(expr(n->expr.get()));
            return;
        }
        if (auto* n = dynamic_cast<const Skrib*>(node)) {
            std::string args;
            for (std::size_t i = 0; i < n->args.size(); ++i) {
                if (i > 0) args += ", ";
                args += expr(n->args[i].get());
            }
            emit("_cmc_skrib(\"" + n->shape + "\", [" + args + "])");
            return;
        }
        emit("# (CMC cannot write this line yet)");
    }

    void emit_if(const If* node) {
        emit("if _cmc_true(" + expr(node->cond.get()) + "):");
        body(node->body);
        const std::vector<StmtPtr>* else_body = &node->else_body;
        while (!else_body->empty()) {
            if (else_body->size() == 1) {
                if (auto* inner = dynamic_cast<const If*>(else_body->at(0).get())) {
                    emit("elif _cmc_true(" + expr(inner->cond.get()) + "):");
                    body(inner->body);
                    else_body = &inner->else_body;
                    continue;
                }
            }
            emit("else:");
            body(*else_body);
            break;
        }
    }

    void body(const std::vector<StmtPtr>& statements, bool empty_pass = false) {
        ++indent_;
        if (statements.empty()) {
            if (empty_pass) emit("pass");
        } else {
            for (const auto& stmt : statements) {
                statement(stmt.get());
            }
        }
        --indent_;
    }

    std::string expr(const Expr* node) {
        if (auto* n = dynamic_cast<const Num*>(node)) {
            return n->is_float ? py_repr_double(n->f) : std::to_string(n->i);
        }
        if (auto* n = dynamic_cast<const Str*>(node)) {
            return py_repr_string(n->value);
        }
        if (auto* n = dynamic_cast<const Var*>(node)) {
            return n->name;
        }
        if (auto* n = dynamic_cast<const Neg*>(node)) {
            return "(-" + expr(n->operand.get()) + ")";
        }
        if (auto* n = dynamic_cast<const Not*>(node)) {
            return "(not _cmc_true(" + expr(n->operand.get()) + "))";
        }
        if (auto* n = dynamic_cast<const LogicOp*>(node)) {
            std::string left = "_cmc_true(" + expr(n->left.get()) + ")";
            std::string right = "_cmc_true(" + expr(n->right.get()) + ")";
            return "(" + left + " " + n->op + " " + right + ")";
        }
        if (auto* n = dynamic_cast<const Compare*>(node)) {
            if (n->op == "==") {
                return "_cmc_eq(" + expr(n->left.get()) + ", " + expr(n->right.get()) + ")";
            }
            if (n->op == "!=") {
                return "(not _cmc_eq(" + expr(n->left.get()) + ", " + expr(n->right.get()) + "))";
            }
            return "_cmc_cmp(\"" + n->op + "\", " + expr(n->left.get()) + ", "
                   + expr(n->right.get()) + ")";
        }
        if (auto* n = dynamic_cast<const InOp*>(node)) {
            return "_cmc_in(" + expr(n->item.get()) + ", " + expr(n->container.get()) + ", "
                   + (n->negated ? "True" : "False") + ")";
        }
        if (auto* n = dynamic_cast<const BinOp*>(node)) {
            static const std::unordered_map<std::string, std::string> helpers = {
                {"+", "_cmc_plus"}, {"-", "_cmc_minus"}, {"*", "_cmc_times"},
                {"/", "_cmc_div"},  {"%", "_cmc_mod"},
            };
            return helpers.at(n->op) + "(" + expr(n->left.get()) + ", " + expr(n->right.get())
                   + ")";
        }
        if (auto* n = dynamic_cast<const Index*>(node)) {
            return "_cmc_index(" + expr(n->target.get()) + ", " + expr(n->index.get()) + ")";
        }
        if (auto* n = dynamic_cast<const Ask*>(node)) {
            if (!n->prompt) return "_cmc_ask(None)";
            return "_cmc_ask(" + expr(n->prompt.get()) + ")";
        }
        if (auto* n = dynamic_cast<const Snorf*>(node)) {
            std::string text = "[";
            for (std::size_t i = 0; i < n->items.size(); ++i) {
                if (i > 0) text += ", ";
                text += expr(n->items[i].get());
            }
            return text + "]";
        }
        if (auto* n = dynamic_cast<const Call*>(node)) {
            std::string args;
            for (std::size_t i = 0; i < n->args.size(); ++i) {
                if (i > 0) args += ", ";
                args += expr(n->args[i].get());
            }
            if (is_builtin_name(n->name)) {
                return "_cmc_" + n->name + "(" + args + ")";
            }
            return n->name + "(" + args + ")";
        }
        return "None";
    }

    static bool is_builtin_name(const std::string& name) {
        static const std::vector<std::string> names = {
            "goop", "snorf", "nom", "skoop", "yoink", "plop", "shout", "whisper",
            "flip", "find", "split", "join", "what", "numba", "munga", "round",
            "flat", "roof", "abs", "small", "big", "root", "pow", "wait",
        };
        for (const auto& n : names) {
            if (n == name) return true;
        }
        return false;
    }

    std::vector<std::string> lines_;
    int indent_ = 0;
};

}

std::string to_python_source(const std::string& source, const std::string& filename) {
    std::unique_ptr<Program> program = parse_source(source, filename);
    return PythonWriter().write(*program, filename);
}

namespace {

const std::unordered_map<std::string, int>& cmc_prec() {
    static const std::unordered_map<std::string, int> table = {
        {"or", 1}, {"and", 2}, {"not", 3}, {"cmp", 4}, {"add", 5}, {"mul", 6}, {"unary", 7},
    };
    return table;
}

std::string maybe_parens(const std::string& text, int prec, int parent) {
    if (prec < parent) return "(" + text + ")";
    return text;
}

std::string expr_text(const Expr* node, int parent);

std::string expr_text(const Expr* node, int parent) {
    if (auto* n = dynamic_cast<const Num*>(node)) {
        return n->is_float ? py_repr_double(n->f) : std::to_string(n->i);
    }
    if (auto* n = dynamic_cast<const Str*>(node)) {
        return py_repr_string(n->value);
    }
    if (auto* n = dynamic_cast<const Var*>(node)) {
        return n->name;
    }
    if (auto* n = dynamic_cast<const Neg*>(node)) {
        return maybe_parens("-" + expr_text(n->operand.get(), cmc_prec().at("unary")),
                            cmc_prec().at("unary"), parent);
    }
    if (auto* n = dynamic_cast<const Not*>(node)) {
        return maybe_parens("not " + expr_text(n->operand.get(), cmc_prec().at("not")),
                            cmc_prec().at("not"), parent);
    }
    if (auto* n = dynamic_cast<const LogicOp*>(node)) {
        int prec = cmc_prec().at(n->op);
        std::string text = expr_text(n->left.get(), prec) + " " + n->op + " "
                           + expr_text(n->right.get(), prec + 1);
        return maybe_parens(text, prec, parent);
    }
    if (auto* n = dynamic_cast<const Compare*>(node)) {
        std::string text = expr_text(n->left.get(), cmc_prec().at("add")) + " " + n->op + " "
                           + expr_text(n->right.get(), cmc_prec().at("add"));
        return maybe_parens(text, cmc_prec().at("cmp"), parent);
    }
    if (auto* n = dynamic_cast<const InOp*>(node)) {
        std::string middle = n->negated ? " not in " : " in ";
        std::string text = expr_text(n->item.get(), cmc_prec().at("add")) + middle
                           + expr_text(n->container.get(), cmc_prec().at("add"));
        return maybe_parens(text, cmc_prec().at("cmp"), parent);
    }
    if (auto* n = dynamic_cast<const BinOp*>(node)) {
        int prec = (n->op == "+" || n->op == "-") ? cmc_prec().at("add") : cmc_prec().at("mul");
        std::string text = expr_text(n->left.get(), prec) + " " + n->op + " "
                           + expr_text(n->right.get(), prec + 1);
        return maybe_parens(text, prec, parent);
    }
    if (auto* n = dynamic_cast<const Index*>(node)) {
        return expr_text(n->target.get(), 0) + "[" + expr_text(n->index.get(), 0) + "]";
    }
    if (auto* n = dynamic_cast<const Ask*>(node)) {
        if (!n->prompt) return "blorp()";
        return "blorp " + expr_text(n->prompt.get(), 0);
    }
    if (auto* n = dynamic_cast<const Snorf*>(node)) {
        std::string text = "snorf(";
        for (std::size_t i = 0; i < n->items.size(); ++i) {
            if (i > 0) text += ", ";
            text += expr_text(n->items[i].get(), 0);
        }
        return text + ")";
    }
    if (auto* n = dynamic_cast<const Call*>(node)) {
        std::string args;
        for (std::size_t i = 0; i < n->args.size(); ++i) {
            if (i > 0) args += ", ";
            args += expr_text(n->args[i].get(), 0);
        }
        return n->name + "(" + args + ")";
    }
    return "plop";
}

}

std::string expr_to_cmc(const Expr* node) {
    return expr_text(node, 0);
}

}
