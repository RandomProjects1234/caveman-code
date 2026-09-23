#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cmc {

struct Node {
    int line = 0;
    virtual ~Node() = default;
};

struct Expr : Node {};
struct Stmt : Node {};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

struct Program : Node {
    std::vector<StmtPtr> statements;
};

struct Num : Expr {
    bool is_float = false;
    long long i = 0;
    double f = 0.0;
};

struct Str : Expr {
    std::string value;
};

struct Var : Expr {
    std::string name;
};

struct BinOp : Expr {
    std::string op;
    ExprPtr left;
    ExprPtr right;
};

struct Neg : Expr {
    ExprPtr operand;
};

struct LogicOp : Expr {
    std::string op;
    ExprPtr left;
    ExprPtr right;
};

struct Not : Expr {
    ExprPtr operand;
};

struct Compare : Expr {
    std::string op;
    ExprPtr left;
    ExprPtr right;
};

struct InOp : Expr {
    ExprPtr item;
    ExprPtr container;
    bool negated = false;
};

struct Call : Expr {
    std::string name;
    std::vector<ExprPtr> args;
};

struct Index : Expr {
    ExprPtr target;
    ExprPtr index;
};

struct Ask : Expr {
    ExprPtr prompt;
};

struct Snorf : Expr {
    std::vector<ExprPtr> items;
};

struct Say : Stmt {
    std::vector<ExprPtr> exprs;
};

struct Assign : Stmt {
    std::string name;
    ExprPtr expr;
    bool declare = false;
};

struct SetIndex : Stmt {
    ExprPtr target;
    ExprPtr index;
    ExprPtr value;
};

struct If : Stmt {
    ExprPtr cond;
    std::vector<StmtPtr> body;
    std::vector<StmtPtr> else_body;
};

struct Repeat : Stmt {
    ExprPtr count;
    std::vector<StmtPtr> body;
};

struct While : Stmt {
    ExprPtr cond;
    std::vector<StmtPtr> body;
};

struct ForEach : Stmt {
    std::string name;
    ExprPtr iterable;
    std::vector<StmtPtr> body;
};

struct FuncDef : Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
};

struct Return : Stmt {
    ExprPtr expr;
};

struct ExprStmt : Stmt {
    ExprPtr expr;
};

struct Skrib : Stmt {
    std::string shape;
    std::vector<ExprPtr> args;
};

}
