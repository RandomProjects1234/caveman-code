#include "builtins.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <unordered_map>
#include <utility>

#include "errors.hpp"
#include "interpreter.hpp"
#include "textutil.hpp"
#include "words.hpp"

namespace cmc {

namespace {

using BuiltinFn = Value (*)(Interpreter&, const std::vector<Value>&, int);

std::mt19937& rng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

Value need_text(const Value& v, int line, const std::string& extra) {
    if (!v.is_text()) {
        throw CmcRuntimeError(
            extra + " needs text (goop), but got " + type_word(v) + ".",
            line,
            "put quotes around it, like \"hello\".");
    }
    return v;
}

Value need_pile(const Value& v, int line, const std::string& extra) {
    if (!v.is_pile()) {
        throw CmcRuntimeError(
            extra + " needs a pile (snorf), but got " + type_word(v) + ".",
            line,
            "make a pile like this: grunk xs = snorf(1, 2, 3)");
    }
    return v;
}

long long spot(const Value& v, int line, const std::string& extra) {
    return whole(v, line, extra).as_int();
}

Value bi_goop(Interpreter&, const std::vector<Value>& args, int) {
    return Value::text(show(args[0]));
}

Value bi_snorf(Interpreter&, const std::vector<Value>& args, int) {
    Value pile = Value::pile();
    pile.as_pile()->items = args;
    return pile;
}

Value bi_nom(Interpreter&, const std::vector<Value>& args, int line) {
    const Value& v = args[0];
    if (v.is_text()) return Value::integer(static_cast<long long>(utf8_length(v.as_text())));
    if (v.is_pile()) return Value::integer(static_cast<long long>(v.as_pile()->items.size()));
    throw CmcRuntimeError(
        "nom counts piles and text, but got " + type_word(v) + ".",
        line,
        "nom(pets) counts a pile. nom(\"cave\") counts letters.");
}

Value bi_skoop(Interpreter&, const std::vector<Value>& args, int line) {
    const Value& v = args[0];
    long long idx = spot(args[1], line, "skoop");
    if (!v.is_text() && !v.is_pile()) {
        throw CmcRuntimeError(
            "skoop reaches into piles and text, but got " + type_word(v) + ".",
            line,
            "make a pile like this: grunk xs = snorf(1, 2, 3)");
    }
    long long length = v.is_text() ? static_cast<long long>(utf8_length(v.as_text()))
                                   : static_cast<long long>(v.as_pile()->items.size());
    if (idx < 0) idx += length;
    if (idx < 0 || idx >= length) {
        throw CmcRuntimeError(
            "There is no spot " + show(Value::integer(idx)) + " in that "
                + (v.is_text() ? "text" : "pile") + " (it has " + std::to_string(length)
                + " spots).",
            line,
            "spots start at 0, so the last spot is " + std::to_string(length - 1) + ".");
    }
    if (v.is_text()) return Value::text(utf8_at(v.as_text(), idx));
    return v.as_pile()->items[static_cast<std::size_t>(idx)];
}

Value bi_yoink(Interpreter&, const std::vector<Value>& args, int line) {
    Value pile = need_pile(args[0], line, "yoink");
    auto& items = pile.as_pile()->items;
    if (items.empty()) {
        throw CmcRuntimeError(
            "The pile is empty, so there is nothing to yoink!",
            line,
            "check nom(pile) > 0 before yoinking.");
    }
    Value top = items.back();
    items.pop_back();
    return top;
}

Value bi_plop(Interpreter&, const std::vector<Value>& args, int line) {
    Value pile = need_pile(args[0], line, "plop");
    pile.as_pile()->items.push_back(args[1]);
    return Value::plop();
}

Value bi_shout(Interpreter&, const std::vector<Value>& args, int line) {
    return Value::text(text_upper(need_text(args[0], line, "shout").as_text()));
}

Value bi_whisper(Interpreter&, const std::vector<Value>& args, int line) {
    return Value::text(text_lower(need_text(args[0], line, "whisper").as_text()));
}

Value bi_flip(Interpreter&, const std::vector<Value>& args, int line) {
    return Value::text(utf8_reverse(need_text(args[0], line, "flip").as_text()));
}

Value bi_find(Interpreter&, const std::vector<Value>& args, int line) {
    std::string text = need_text(args[0], line, "find").as_text();
    std::string piece = need_text(args[1], line, "find").as_text();
    return Value::integer(utf8_find(text, piece));
}

Value bi_split(Interpreter&, const std::vector<Value>& args, int line) {
    std::string text = need_text(args[0], line, "split").as_text();
    std::string sep = need_text(args[1], line, "split").as_text();
    if (sep.empty()) {
        throw CmcRuntimeError(
            "split needs something to cut on, and an empty text cuts nothing.", line);
    }
    Value pile = Value::pile();
    std::size_t start = 0;
    while (true) {
        std::size_t found = text.find(sep, start);
        if (found == std::string::npos) {
            pile.as_pile()->items.push_back(Value::text(text.substr(start)));
            break;
        }
        pile.as_pile()->items.push_back(Value::text(text.substr(start, found - start)));
        start = found + sep.size();
    }
    return pile;
}

Value bi_join(Interpreter&, const std::vector<Value>& args, int line) {
    Value pile = need_pile(args[0], line, "join");
    std::string sep = args[1].is_text() ? args[1].as_text() : show(args[1]);
    std::string out;
    const auto& items = pile.as_pile()->items;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) out += sep;
        out += show(items[i]);
    }
    return Value::text(out);
}

Value bi_what(Interpreter&, const std::vector<Value>& args, int) {
    return Value::text(type_name(args[0]));
}

Value bi_numba(Interpreter&, const std::vector<Value>& args, int line) {
    const Value& v = args[0];
    if (v.is_bool()) {
        throw CmcRuntimeError(
            "numba cannot turn gronk/nork into a number.",
            line,
            "use 1 for gronk or 0 for nork.");
    }
    if (v.is_number()) return v;
    if (v.is_text()) {
        std::string text = trim_ascii(v.as_text());
        long long as_int = 0;
        if (parse_python_int(text, as_int)) return Value::integer(as_int);
        double as_float = 0.0;
        if (parse_python_float(text, as_float)) return Value::real(as_float);
        throw CmcRuntimeError(
            "numba cannot turn \"" + v.as_text() + "\" into a number.",
            line,
            "answers that are numbers look like \"42\" or \"3.5\".");
    }
    throw CmcRuntimeError(
        "numba needs text or a number, but got " + type_word(v) + ".",
        line,
        "use it like this: numba(\"42\")");
}

Value bi_munga(Interpreter&, const std::vector<Value>& args, int line) {
    Value lo = number(args[0], line, "munga");
    Value hi = number(args[1], line, "munga");
    bool ints = lo.is_int() && hi.is_int();
    if (ints) {
        long long a = lo.as_int();
        long long b = hi.as_int();
        if (a > b) std::swap(a, b);
        std::uniform_int_distribution<long long> dist(a, b);
        return Value::integer(dist(rng()));
    }
    double a = lo.as_number();
    double b = hi.as_number();
    if (a > b) std::swap(a, b);
    std::uniform_real_distribution<double> dist(a, b);
    return Value::real(dist(rng()));
}

Value bi_round(Interpreter&, const std::vector<Value>& args, int line) {
    Value v = number(args[0], line, "round");
    double d = v.as_number();
    if (d >= 0) return Value::integer(static_cast<long long>(std::floor(d + 0.5)));
    return Value::integer(-static_cast<long long>(std::floor(-d + 0.5)));
}

Value bi_flat(Interpreter&, const std::vector<Value>& args, int line) {
    return Value::integer(static_cast<long long>(std::floor(number(args[0], line, "flat").as_number())));
}

Value bi_roof(Interpreter&, const std::vector<Value>& args, int line) {
    return Value::integer(static_cast<long long>(std::ceil(number(args[0], line, "roof").as_number())));
}

Value bi_abs(Interpreter&, const std::vector<Value>& args, int line) {
    Value v = number(args[0], line, "abs");
    if (v.is_int()) {
        long long n = v.as_int();
        if (n == std::numeric_limits<long long>::min()) return Value::real(std::fabs(v.as_number()));
        return Value::integer(n < 0 ? -n : n);
    }
    return Value::real(std::fabs(v.as_float()));
}

Value bi_small(Interpreter&, const std::vector<Value>& args, int line) {
    Value a = number(args[0], line, "small");
    Value b = number(args[1], line, "small");
    if (a.is_int() && b.is_int()) return a.as_int() <= b.as_int() ? a : b;
    return a.as_number() <= b.as_number() ? a : b;
}

Value bi_big(Interpreter&, const std::vector<Value>& args, int line) {
    Value a = number(args[0], line, "big");
    Value b = number(args[1], line, "big");
    if (a.is_int() && b.is_int()) return a.as_int() >= b.as_int() ? a : b;
    return a.as_number() >= b.as_number() ? a : b;
}

Value bi_root(Interpreter&, const std::vector<Value>& args, int line) {
    Value v = number(args[0], line, "root");
    if (v.as_number() < 0) {
        throw CmcRuntimeError(
            "root cannot work on negative numbers (nothing times itself makes " + show(v) + ").",
            line,
            "use abs(value) first if you want the size without the minus.");
    }
    double result = std::sqrt(v.as_number());
    if (std::floor(result) == result) return Value::integer(static_cast<long long>(result));
    return Value::real(result);
}

Value bi_pow(Interpreter&, const std::vector<Value>& args, int line) {
    Value base = number(args[0], line, "pow");
    Value times = number(args[1], line, "pow");
    double result = std::pow(base.as_number(), times.as_number());
    if (std::isfinite(result) && std::floor(result) == result
        && std::fabs(result) < 9.0e18) {
        return Value::integer(static_cast<long long>(result));
    }
    return Value::real(result);
}

Value bi_wait(Interpreter& interp, const std::vector<Value>& args, int line) {
    Value ms = number(args[0], line, "wait");
    double value = ms.as_number();
    if (value < 0) value = 0;
    interp.wait_ms(value);
    return Value::plop();
}

const std::unordered_map<std::string, BuiltinFn>& impls() {
    static const std::unordered_map<std::string, BuiltinFn> table = {
        {"goop", bi_goop},       {"snorf", bi_snorf},   {"nom", bi_nom},
        {"skoop", bi_skoop},     {"yoink", bi_yoink},   {"plop", bi_plop},
        {"shout", bi_shout},     {"whisper", bi_whisper}, {"flip", bi_flip},
        {"find", bi_find},       {"split", bi_split},   {"join", bi_join},
        {"what", bi_what},       {"numba", bi_numba},   {"munga", bi_munga},
        {"round", bi_round},     {"flat", bi_flat},     {"roof", bi_roof},
        {"abs", bi_abs},         {"small", bi_small},   {"big", bi_big},
        {"root", bi_root},       {"pow", bi_pow},       {"wait", bi_wait},
    };
    return table;
}

}

bool is_builtin(const std::string& name) {
    return impls().count(name) > 0;
}

Value call_builtin(Interpreter& interp, const std::string& name, const std::vector<Value>& args,
                   int line) {
    auto it = impls().find(name);
    if (it == impls().end()) return Value::plop();
    auto arity_it = words::arity().find(name);
    int low = 0;
    int high = 0;
    if (arity_it != words::arity().end()) {
        low = arity_it->second.first;
        high = arity_it->second.second;
    }
    if (static_cast<int>(args.size()) < low || static_cast<int>(args.size()) > high) {
        std::string wanted = (low == high) ? std::to_string(low)
                                           : std::to_string(low) + " to " + std::to_string(high);
        std::string plural = (wanted != "1") ? "s" : "";
        throw CmcRuntimeError(
            "'" + name + "' wants " + wanted + " thing" + plural + ", but got "
                + std::to_string(args.size()) + ".",
            line,
            "look at the caveman dictionary (Help menu) for how " + name + " works.");
    }
    return it->second(interp, args, line);
}

}
