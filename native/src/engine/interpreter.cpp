#include "interpreter.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

#include "builtins.hpp"
#include "errors.hpp"
#include "parser.hpp"
#include "textutil.hpp"

namespace cmc {

namespace {

bool add_int(long long a, long long b, long long& out) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_add_overflow(a, b, &out);
#else
    if ((b > 0 && a > std::numeric_limits<long long>::max() - b)
        || (b < 0 && a < std::numeric_limits<long long>::min() - b)) {
        return false;
    }
    out = a + b;
    return true;
#endif
}

bool sub_int(long long a, long long b, long long& out) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_sub_overflow(a, b, &out);
#else
    if ((b < 0 && a > std::numeric_limits<long long>::max() + b)
        || (b > 0 && a < std::numeric_limits<long long>::min() + b)) {
        return false;
    }
    out = a - b;
    return true;
#endif
}

bool mul_int(long long a, long long b, long long& out) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_mul_overflow(a, b, &out);
#else
    if (a == 0 || b == 0) {
        out = 0;
        return true;
    }
    if (a == -1 && b == std::numeric_limits<long long>::min()) return false;
    if (b == -1 && a == std::numeric_limits<long long>::min()) return false;
    long long result = a * b;
    if (result / b != a) return false;
    out = result;
    return true;
#endif
}

bool is_zero(const Value& v) {
    if (v.is_int()) return v.as_int() == 0;
    return v.as_float() == 0.0;
}

Value numeric_add(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        long long result = 0;
        if (add_int(a.as_int(), b.as_int(), result)) return Value::integer(result);
    }
    return Value::real(a.as_number() + b.as_number());
}

Value numeric_sub(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        long long result = 0;
        if (sub_int(a.as_int(), b.as_int(), result)) return Value::integer(result);
    }
    return Value::real(a.as_number() - b.as_number());
}

Value numeric_mul(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        long long result = 0;
        if (mul_int(a.as_int(), b.as_int(), result)) return Value::integer(result);
    }
    return Value::real(a.as_number() * b.as_number());
}

Value numeric_div(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        long long bi = b.as_int();
        if (a.as_int() % bi == 0) return Value::integer(a.as_int() / bi);
    }
    return Value::real(a.as_number() / b.as_number());
}

Value numeric_mod(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        long long bi = b.as_int();
        long long result = a.as_int() % bi;
        if (result != 0 && ((result < 0) != (bi < 0))) result += bi;
        return Value::integer(result);
    }
    double bi = b.as_number();
    double result = std::fmod(a.as_number(), bi);
    if (result != 0.0 && ((result < 0.0) != (bi < 0.0))) result += bi;
    return Value::real(result);
}

int compare_values(const Value& a, const Value& b) {
    if (a.is_int() && b.is_int()) {
        if (a.as_int() < b.as_int()) return -1;
        if (a.as_int() > b.as_int()) return 1;
        return 0;
    }
    if (a.is_number() && b.is_number()) {
        double x = a.as_number();
        double y = b.as_number();
        if (x < y) return -1;
        if (x > y) return 1;
        return 0;
    }
    const std::string& x = a.as_text();
    const std::string& y = b.as_text();
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

}

Env* Env::find(const std::string& name) {
    Env* env = this;
    while (env != nullptr) {
        if (env->vars.count(name) > 0) return env;
        env = env->parent.get();
    }
    return nullptr;
}

Value Env::get(const std::string& name, int line) {
    Env* env = find(name);
    if (env == nullptr) {
        throw CmcRuntimeError(
            "There is no box named '" + name + "'.",
            line,
            "make it first with: grunk " + name + " = ...");
    }
    Binding& binding = env->vars[name];
    if (auto* value = std::get_if<Value>(&binding)) return *value;
    return Value::text("<clump " + name + ">");
}

void Env::define(const std::string& name, Value value) {
    vars[name] = std::move(value);
}

void Env::define(const std::string& name, UserFunction func) {
    vars[name] = std::move(func);
}

void Env::assign(const std::string& name, Value value, int line) {
    Env* env = find(name);
    if (env == nullptr) {
        throw CmcRuntimeError(
            "There is no box named '" + name + "' to put that into.",
            line,
            "make it first with: grunk " + name + " = ...");
    }
    env->vars[name] = std::move(value);
}

Interpreter::Interpreter() {
    output_ = [](const std::string& line) {
        std::cout << line << "\n";
        std::cout.flush();
    };
    globals_ = std::make_shared<Env>();
    install_constants();
}

void Interpreter::install_constants() {
    globals_->define("gronk", Value::truth(true));
    globals_->define("nork", Value::truth(false));
    globals_->define("plop", Value::plop());
    globals_->define("pi", Value::real(3.141592653589793));
}

void Interpreter::set_output(OutputFn fn) {
    if (fn) output_ = std::move(fn);
}

void Interpreter::set_input(InputFn fn) {
    input_ = std::move(fn);
}

void Interpreter::set_draw(DrawSurface* draw) {
    draw_ = draw;
}

void Interpreter::set_should_stop(StopFn fn) {
    should_stop_ = std::move(fn);
}

void Interpreter::set_sleep(SleepFn fn) {
    sleep_ = std::move(fn);
}

void Interpreter::set_step_limit(long long limit) {
    step_limit_ = limit;
}

void Interpreter::set_max_depth(int depth) {
    max_depth_ = depth;
}

void Interpreter::run(const std::string& source, const std::string& filename) {
    std::unique_ptr<Program> program = parse_source(source, filename);
    run_tree(*program);
}

void Interpreter::run_tree(const Program& program) {
    steps_ = 0;
    depth_ = 0;
    try {
        for (const auto& stmt : program.statements) {
            exec_statement(stmt.get(), *globals_);
        }
    } catch (ReturnSignal&) {
        throw CmcRuntimeError(
            "'ork' can only live inside a clump.",
            -1,
            "wrap it like this: clump name() ... ork answer ... unga");
    }
}

void Interpreter::tick(int line) {
    ++steps_;
    if (steps_ > step_limit_) throw CmcStepLimit(line);
    if (should_stop_ && should_stop_()) throw CmcStopped();
}

void Interpreter::wait_ms(double ms) {
    if (sleep_) {
        sleep_(ms / 1000.0);
        return;
    }
    auto end = std::chrono::steady_clock::now()
               + std::chrono::microseconds(static_cast<long long>(ms * 1000.0));
    while (std::chrono::steady_clock::now() < end) {
        tick();
        double remaining =
            std::chrono::duration<double>(end - std::chrono::steady_clock::now()).count();
        double nap = std::min(0.02, std::max(0.0, remaining));
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>(nap * 1e6)));
    }
}

void Interpreter::exec_statement(const Stmt* node, Env& env) {
    tick(node->line);

    if (auto* n = dynamic_cast<const Say*>(node)) {
        std::string out;
        for (std::size_t i = 0; i < n->exprs.size(); ++i) {
            if (i > 0) out += " ";
            out += show(eval(n->exprs[i].get(), env));
        }
        output_(out);
        return;
    }

    if (auto* n = dynamic_cast<const Assign*>(node)) {
        Value value = eval(n->expr.get(), env);
        if (n->declare) {
            env.define(n->name, std::move(value));
        } else {
            env.assign(n->name, std::move(value), n->line);
        }
        return;
    }

    if (auto* n = dynamic_cast<const SetIndex*>(node)) {
        Value target = eval(n->target.get(), env);
        long long index = whole(eval(n->index.get(), env), n->line, "A spot number").as_int();
        Value value = eval(n->value.get(), env);
        if (!target.is_pile()) {
            throw CmcRuntimeError(
                "Only piles can have spots changed, but that is " + type_word(target) + ".",
                n->line,
                "make a pile with snorf(...) and plop things into it.");
        }
        auto& items = target.as_pile()->items;
        long long length = static_cast<long long>(items.size());
        if (index < 0) index += length;
        if (index < 0 || index >= length) {
            throw CmcRuntimeError(
                "That pile has no spot " + std::to_string(index) + " yet (it has "
                    + std::to_string(length) + " spots).",
                n->line,
                "add to the end of a pile with plop(pile, thing).");
        }
        items[static_cast<std::size_t>(index)] = std::move(value);
        return;
    }

    if (auto* n = dynamic_cast<const If*>(node)) {
        if (truthy(eval(n->cond.get(), env))) {
            exec_block(n->body, env);
        } else {
            exec_block(n->else_body, env);
        }
        return;
    }

    if (auto* n = dynamic_cast<const Repeat*>(node)) {
        long long count = whole(eval(n->count.get(), env), n->line, "'booga'").as_int();
        if (count < 0) count = 0;
        for (long long lap = 0; lap < count; ++lap) {
            tick(n->line);
            env.define("lap", Value::integer(lap + 1));
            exec_block(n->body, env);
        }
        return;
    }

    if (auto* n = dynamic_cast<const While*>(node)) {
        while (truthy(eval(n->cond.get(), env))) {
            tick(n->line);
            exec_block(n->body, env);
        }
        return;
    }

    if (auto* n = dynamic_cast<const ForEach*>(node)) {
        Value iterable = eval(n->iterable.get(), env);
        std::vector<Value> items;
        if (iterable.is_pile()) {
            items = iterable.as_pile()->items;
        } else if (iterable.is_text()) {
            const std::string& text = iterable.as_text();
            std::size_t length = utf8_length(text);
            items.reserve(length);
            for (std::size_t i = 0; i < length; ++i) {
                items.push_back(Value::text(utf8_at(text, static_cast<long long>(i))));
            }
        } else {
            throw CmcRuntimeError(
                "'zoop' walks through piles and text, but that is " + type_word(iterable) + ".",
                n->line,
                "make a pile with snorf(...) first.");
        }
        for (auto& item : items) {
            tick(n->line);
            env.define(n->name, item);
            exec_block(n->body, env);
        }
        return;
    }

    if (auto* n = dynamic_cast<const FuncDef*>(node)) {
        UserFunction func;
        func.node = n;
        func.closure = env.shared_from_this();
        env.define(n->name, std::move(func));
        return;
    }

    if (auto* n = dynamic_cast<const Return*>(node)) {
        Value value = n->expr ? eval(n->expr.get(), env) : Value::plop();
        throw ReturnSignal{std::move(value)};
    }

    if (auto* n = dynamic_cast<const Skrib*>(node)) {
        exec_skrib(n, env);
        return;
    }

    if (auto* n = dynamic_cast<const ExprStmt*>(node)) {
        eval(n->expr.get(), env);
        return;
    }

    throw CmcRuntimeError("I do not know how to do that yet.", node->line);
}

void Interpreter::exec_block(const std::vector<StmtPtr>& statements, Env& env) {
    for (const auto& stmt : statements) {
        exec_statement(stmt.get(), env);
    }
}

void Interpreter::exec_skrib(const Skrib* node, Env& env) {
    std::vector<Value> values;
    values.reserve(node->args.size());
    for (const auto& arg : node->args) {
        values.push_back(eval(arg.get(), env));
    }
    if (draw_ == nullptr) {
        warned_no_draw_ = true;
        return;
    }
    const std::string& shape = node->shape;
    if (shape == "clear") {
        draw_->clear();
    } else if (shape == "size") {
        draw_->size(static_cast<int>(whole(values[0], node->line, "skrib size").as_int()),
                    static_cast<int>(whole(values[1], node->line, "skrib size").as_int()));
    } else if (shape == "color") {
        draw_->color(values[0].is_text() ? values[0].as_text() : show(values[0]));
    } else if (shape == "dot") {
        draw_->dot(number(values[0], node->line).as_number(),
                   number(values[1], node->line).as_number(),
                   number(values[2], node->line).as_number());
    } else if (shape == "circle") {
        draw_->circle(number(values[0], node->line).as_number(),
                      number(values[1], node->line).as_number(),
                      number(values[2], node->line).as_number());
    } else if (shape == "line") {
        draw_->line(number(values[0], node->line).as_number(),
                    number(values[1], node->line).as_number(),
                    number(values[2], node->line).as_number(),
                    number(values[3], node->line).as_number());
    } else if (shape == "box") {
        draw_->box(number(values[0], node->line).as_number(),
                   number(values[1], node->line).as_number(),
                   number(values[2], node->line).as_number(),
                   number(values[3], node->line).as_number());
    } else if (shape == "blob") {
        draw_->blob(number(values[0], node->line).as_number(),
                    number(values[1], node->line).as_number(),
                    number(values[2], node->line).as_number(),
                    number(values[3], node->line).as_number());
    } else if (shape == "write") {
        draw_->write(values[0].is_text() ? values[0].as_text() : show(values[0]),
                     number(values[1], node->line).as_number(),
                     number(values[2], node->line).as_number());
    }
}

Value Interpreter::eval(const Expr* node, Env& env) {
    if (auto* n = dynamic_cast<const Num*>(node)) {
        return n->is_float ? Value::real(n->f) : Value::integer(n->i);
    }
    if (auto* n = dynamic_cast<const Str*>(node)) {
        return Value::text(n->value);
    }
    if (auto* n = dynamic_cast<const Var*>(node)) {
        return env.get(n->name, n->line);
    }
    if (auto* n = dynamic_cast<const Neg*>(node)) {
        Value value = number(eval(n->operand.get(), env), n->line);
        if (value.is_int()) {
            long long x = value.as_int();
            if (x != std::numeric_limits<long long>::min()) return Value::integer(-x);
            return Value::real(-static_cast<double>(x));
        }
        return Value::real(-value.as_float());
    }
    if (auto* n = dynamic_cast<const Not*>(node)) {
        return Value::truth(!truthy(eval(n->operand.get(), env)));
    }
    if (auto* n = dynamic_cast<const LogicOp*>(node)) {
        Value left = eval(n->left.get(), env);
        if (n->op == "and") {
            if (!truthy(left)) return Value::truth(false);
            return Value::truth(truthy(eval(n->right.get(), env)));
        }
        if (truthy(left)) return Value::truth(true);
        return Value::truth(truthy(eval(n->right.get(), env)));
    }
    if (auto* n = dynamic_cast<const Compare*>(node)) {
        return eval_compare(n, env);
    }
    if (auto* n = dynamic_cast<const InOp*>(node)) {
        return eval_in(n, env);
    }
    if (auto* n = dynamic_cast<const BinOp*>(node)) {
        return eval_binop(n, env);
    }
    if (auto* n = dynamic_cast<const Index*>(node)) {
        return eval_index(n, env);
    }
    if (auto* n = dynamic_cast<const Ask*>(node)) {
        return eval_ask(n, env);
    }
    if (auto* n = dynamic_cast<const Snorf*>(node)) {
        Value pile = Value::pile();
        for (const auto& item : n->items) {
            pile.as_pile()->items.push_back(eval(item.get(), env));
        }
        return pile;
    }
    if (auto* n = dynamic_cast<const Call*>(node)) {
        return eval_call(n, env);
    }
    throw CmcRuntimeError("I do not know that expression yet.", node->line);
}

Value Interpreter::eval_binop(const BinOp* node, Env& env) {
    Value left = eval(node->left.get(), env);
    Value right = eval(node->right.get(), env);
    const std::string& op = node->op;
    int line = node->line;

    if (op == "+") {
        if (isnum(left) && isnum(right)) return numeric_add(left, right);
        if (left.is_text() && right.is_text()) {
            return Value::text(left.as_text() + right.as_text());
        }
        if (left.is_text() || right.is_text()) {
            throw CmcRuntimeError(
                "Text and numbers cannot be stuck together directly.",
                line,
                "wrap the number in goop first: \"I am \" + goop(7)");
        }
        throw CmcRuntimeError(
            "Cannot add " + type_word(left) + " and " + type_word(right) + ".",
            line,
            "+ works on numbers and text.");
    }

    if (op == "-") {
        Value a = number(left, line, "Minus");
        Value b = number(right, line, "Minus");
        return numeric_sub(a, b);
    }

    if (op == "*") {
        if (left.is_text() || right.is_text()) {
            throw CmcRuntimeError(
                "Text cannot be multiplied.",
                line,
                "to repeat text, use a booga loop and add it again and again.");
        }
        Value a = number(left, line, "Times");
        Value b = number(right, line, "Times");
        return numeric_mul(a, b);
    }

    if (op == "/") {
        Value a = number(left, line, "Divide");
        Value b = number(right, line, "Divide");
        if (is_zero(b)) {
            throw CmcRuntimeError(
                "Cannot share by zero!",
                line,
                "check the number is not 0 before dividing.");
        }
        return numeric_div(a, b);
    }

    if (op == "%") {
        Value a = number(left, line, "Remainder");
        Value b = number(right, line, "Remainder");
        if (is_zero(b)) {
            throw CmcRuntimeError(
                "Cannot find the remainder with zero.",
                line,
                "the remainder % needs a number that is not 0.");
        }
        return numeric_mod(a, b);
    }

    throw CmcRuntimeError("I do not know the math sign '" + op + "'.", line);
}

Value Interpreter::eval_compare(const Compare* node, Env& env) {
    Value left = eval(node->left.get(), env);
    Value right = eval(node->right.get(), env);
    const std::string& op = node->op;
    int line = node->line;

    if (op == "==") return Value::truth(equal(left, right));
    if (op == "!=") return Value::truth(!equal(left, right));

    bool numbers = isnum(left) && isnum(right);
    bool texts = left.is_text() && right.is_text();
    if (!numbers && !texts) {
        throw CmcRuntimeError(
            "I can only line up numbers with numbers, or text with text (not " + type_word(left)
                + " with " + type_word(right) + ").",
            line,
            "use goop() to turn one of them into text first.");
    }

    int cmp = compare_values(left, right);
    if (op == "<") return Value::truth(cmp < 0);
    if (op == "<=") return Value::truth(cmp <= 0);
    if (op == ">") return Value::truth(cmp > 0);
    if (op == ">=") return Value::truth(cmp >= 0);
    throw CmcRuntimeError("I do not know the compare sign '" + op + "'.", line);
}

Value Interpreter::eval_in(const InOp* node, Env& env) {
    Value item = eval(node->item.get(), env);
    Value container = eval(node->container.get(), env);
    bool found = false;
    if (container.is_pile()) {
        for (const auto& x : container.as_pile()->items) {
            if (equal(item, x)) {
                found = true;
                break;
            }
        }
    } else if (container.is_text()) {
        if (!item.is_text()) {
            throw CmcRuntimeError(
                "'in' can look inside text, but only for other text.",
                node->line,
                "look for \"cat\" not 7. Use goop(7) to make it text.");
        }
        found = container.as_text().find(item.as_text()) != std::string::npos;
    } else {
        throw CmcRuntimeError(
            "'in' looks inside piles and text, but that is " + type_word(container) + ".",
            node->line,
            "make a pile with snorf(...) first.");
    }
    return Value::truth(node->negated ? !found : found);
}

Value Interpreter::eval_index(const Index* node, Env& env) {
    Value target = eval(node->target.get(), env);
    long long index = whole(eval(node->index.get(), env), node->line, "A spot number").as_int();
    if (!target.is_pile() && !target.is_text()) {
        throw CmcRuntimeError(
            "Only piles and text have spots, but that is " + type_word(target) + ".",
            node->line,
            "make a pile with snorf(...) first.");
    }
    long long length = target.is_pile() ? static_cast<long long>(target.as_pile()->items.size())
                                        : static_cast<long long>(utf8_length(target.as_text()));
    if (index < 0) index += length;
    if (index < 0 || index >= length) {
        throw CmcRuntimeError(
            "There is no spot " + std::to_string(index) + " in that "
                + (target.is_text() ? "text" : "pile") + " (it has " + std::to_string(length)
                + " spots).",
            node->line,
            "spots start at 0, so the last spot is " + std::to_string(length - 1) + ".");
    }
    if (target.is_text()) return Value::text(utf8_at(target.as_text(), index));
    return target.as_pile()->items[static_cast<std::size_t>(index)];
}

Value Interpreter::eval_ask(const Ask* node, Env& env) {
    std::string prompt;
    bool has_prompt = false;
    if (node->prompt) {
        Value value = eval(node->prompt.get(), env);
        prompt = value.is_text() ? value.as_text() : show(value);
        has_prompt = true;
    }
    if (!input_) {
        if (has_prompt && !prompt.empty()) {
            std::cout << prompt << " ";
            std::cout.flush();
        }
        std::string answer;
        if (!std::getline(std::cin, answer)) {
            throw CmcRuntimeError(
                "blorp asked a question, but no answer could come in.",
                node->line,
                "make sure someone can type an answer.");
        }
        if (!answer.empty() && answer.back() == '\r') answer.pop_back();
        return Value::text(answer);
    }
    return Value::text(input_(has_prompt ? prompt : std::string(), node->line));
}

Value Interpreter::eval_call(const Call* node, Env& env) {
    tick(node->line);
    std::vector<Value> args;
    args.reserve(node->args.size());
    for (const auto& arg : node->args) {
        args.push_back(eval(arg.get(), env));
    }
    int line = node->line;

    Env* holder = env.find(node->name);
    if (holder != nullptr) {
        Binding& binding = holder->vars[node->name];
        if (auto* func = std::get_if<UserFunction>(&binding)) {
            return call_user(*func, args, line);
        }
        if (is_builtin(node->name)) {
            return call_builtin(*this, node->name, args, line);
        }
        throw CmcRuntimeError(
            "'" + node->name + "' is a box, not a clump, so it cannot be called with ( ).",
            line,
            "did you mean to use it without parentheses?");
    }

    if (is_builtin(node->name)) {
        return call_builtin(*this, node->name, args, line);
    }

    throw CmcRuntimeError(
        "There is no clump named '" + node->name + "'.",
        line,
        "make it first with: clump " + node->name + "(...)");
}

Value Interpreter::call_user(UserFunction& func, const std::vector<Value>& args, int line) {
    const FuncDef* node = func.node;
    if (args.size() != node->params.size()) {
        std::string plural = node->params.size() != 1 ? "s" : "";
        std::string param_list;
        for (std::size_t i = 0; i < node->params.size(); ++i) {
            if (i > 0) param_list += ", ";
            param_list += node->params[i];
        }
        throw CmcRuntimeError(
            "'" + node->name + "' wants " + std::to_string(node->params.size()) + " thing" + plural
                + ", but got " + std::to_string(args.size()) + ".",
            line,
            "look at how the clump was made: clump " + node->name + "(" + param_list + ")");
    }
    ++depth_;
    if (depth_ > max_depth_) {
        --depth_;
        throw CmcTooDeep(node->name, line);
    }
    auto local = std::make_shared<Env>(func.closure);
    for (std::size_t i = 0; i < node->params.size(); ++i) {
        local->define(node->params[i], args[i]);
    }
    try {
        exec_block(node->body, *local);
    } catch (ReturnSignal& signal) {
        --depth_;
        return signal.value;
    } catch (...) {
        --depth_;
        throw;
    }
    --depth_;
    return Value::plop();
}

}
