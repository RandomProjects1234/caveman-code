#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "draw.hpp"
#include "values.hpp"

namespace cmc {

struct ReturnSignal {
    Value value;
};

struct Env;

struct UserFunction {
    const FuncDef* node = nullptr;
    std::shared_ptr<Env> closure;
};

using Binding = std::variant<Value, UserFunction>;

struct Env : public std::enable_shared_from_this<Env> {
    std::unordered_map<std::string, Binding> vars;
    std::shared_ptr<Env> parent;

    explicit Env(std::shared_ptr<Env> parent = nullptr) : parent(std::move(parent)) {}

    Env* find(const std::string& name);
    Value get(const std::string& name, int line);
    void define(const std::string& name, Value value);
    void define(const std::string& name, UserFunction func);
    void assign(const std::string& name, Value value, int line);
};

class Interpreter {
public:
    using OutputFn = std::function<void(const std::string&)>;
    using InputFn = std::function<std::string(const std::string&, int)>;
    using StopFn = std::function<bool()>;
    using SleepFn = std::function<void(double)>;

    Interpreter();

    void set_output(OutputFn fn);
    void set_input(InputFn fn);
    void set_draw(DrawSurface* draw);
    void set_should_stop(StopFn fn);
    void set_sleep(SleepFn fn);
    void set_step_limit(long long limit);
    void set_max_depth(int depth);

    void run(const std::string& source, const std::string& filename = "<cmc>");
    void run_tree(const Program& program);
    void wait_ms(double ms);
    void tick(int line = -1);

private:
    void install_constants();
    void exec_statement(const Stmt* node, Env& env);
    void exec_block(const std::vector<StmtPtr>& statements, Env& env);
    void exec_skrib(const Skrib* node, Env& env);
    Value eval(const Expr* node, Env& env);
    Value eval_binop(const BinOp* node, Env& env);
    Value eval_compare(const Compare* node, Env& env);
    Value eval_in(const InOp* node, Env& env);
    Value eval_index(const Index* node, Env& env);
    Value eval_ask(const Ask* node, Env& env);
    Value eval_call(const Call* node, Env& env);
    Value call_user(UserFunction& func, const std::vector<Value>& args, int line);

    OutputFn output_;
    InputFn input_;
    DrawSurface* draw_ = nullptr;
    StopFn should_stop_;
    SleepFn sleep_;
    long long step_limit_ = 5000000;
    int max_depth_ = 200;
    long long steps_ = 0;
    int depth_ = 0;
    std::shared_ptr<Env> globals_;
    bool warned_no_draw_ = false;
};

}
