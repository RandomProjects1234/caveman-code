#include "parser.hpp"

#include <unordered_map>

#include "errors.hpp"
#include "lexer.hpp"
#include "textutil.hpp"
#include "words.hpp"

namespace cmc {

namespace {

const std::unordered_map<std::string, int>& skrib_shapes() {
    static const std::unordered_map<std::string, int> table = {
        {"clear", 0}, {"size", 2},  {"color", 1}, {"dot", 3},  {"circle", 3},
        {"line", 4},  {"box", 4},   {"blob", 4},  {"write", 3},
    };
    return table;
}

bool is_compare_op(const std::string& op) {
    return op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=";
}

}

Parser::Parser(std::vector<Token> tokens, std::string filename)
    : tokens_(std::move(tokens)), filename_(std::move(filename)) {}

Token& Parser::peek(std::size_t ahead) {
    std::size_t spot = pos_ + ahead;
    if (spot >= tokens_.size()) return tokens_.back();
    return tokens_[spot];
}

const Token& Parser::peek(std::size_t ahead) const {
    std::size_t spot = pos_ + ahead;
    if (spot >= tokens_.size()) return tokens_.back();
    return tokens_[spot];
}

Token Parser::advance() {
    Token tok = peek();
    if (tok.kind != TokKind::Eof) ++pos_;
    return tok;
}

bool Parser::at_kw(const std::vector<std::string>& names) const {
    const Token& tok = peek();
    if (tok.kind != TokKind::Kw) return false;
    for (const auto& name : names) {
        if (tok.value == name) return true;
    }
    return false;
}

bool Parser::at_kw(const std::string& name) const {
    const Token& tok = peek();
    return tok.kind == TokKind::Kw && tok.value == name;
}

bool Parser::match_kw(const std::string& name) {
    if (at_kw(name)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect_kw(const std::string& name, const std::string& message,
                        const std::string& hint) {
    const Token& tok = peek();
    if (tok.kind == TokKind::Kw && tok.value == name) {
        return advance();
    }
    std::string text = message;
    if (text.empty()) {
        text = "I expected '" + name + "' here, but found " + tok.describe() + ".";
    }
    throw CmcParseError(text, tok.line, hint);
}

void Parser::expect_eol(const std::string& what) {
    const Token& tok = peek();
    if (tok.kind == TokKind::Nl || tok.kind == TokKind::Eof) return;
    throw CmcParseError(
        "There are extra things on " + what + " that I do not understand: " + tok.describe() + ".",
        tok.line,
        "one thing per line. Press Enter and try the next part there.");
}

Token Parser::expect_name(const std::string& message, const std::string& hint) {
    const Token& tok = peek();
    if (tok.kind == TokKind::Name) return advance();
    throw CmcParseError(message, tok.line, hint);
}

Token Parser::expect_kind(TokKind kind, const std::string& message, const std::string& hint) {
    const Token& tok = peek();
    if (tok.kind == kind) return advance();
    throw CmcParseError(message, tok.line, hint);
}

void Parser::check_name_ok(const std::string& name, int line) {
    if (words::is_forbidden_name(name)) {
        throw CmcParseError(
            "'" + name + "' already means something in CMC, so it cannot be a name you make up.",
            line,
            "try adding a letter or number: " + name + "1");
    }
}

void Parser::error(const Token& tok, const std::string& message, const std::string& hint) {
    throw CmcParseError(message, tok.line, hint);
}

void Parser::skip_newlines() {
    while (peek().kind == TokKind::Nl) advance();
}

std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    skip_newlines();
    while (peek().kind != TokKind::Eof) {
        program->statements.push_back(parse_statement());
        expect_eol();
        skip_newlines();
    }
    return program;
}

StmtPtr Parser::parse_statement() {
    const Token tok = peek();

    if (tok.kind == TokKind::Kw) {
        const std::string& value = tok.value;
        if (value == "oga") return parse_say();
        if (value == "grunk") return parse_grunk();
        if (value == "binga") return parse_if();
        if (value == "booga") return parse_repeat();
        if (value == "zug") return parse_while();
        if (value == "zoop") return parse_foreach();
        if (value == "clump") return parse_clump();
        if (value == "ork") return parse_return();
        if (value == "skrib") return parse_skrib();
        if (value == "unga" || value == "wonga") {
            error(tok,
                  "You said '" + value + "', but nothing was open for it to close.",
                  "every 'unga' closes one 'binga', 'booga', 'zug', 'zoop', or 'clump'.");
        }
        if (value == "and" || value == "or" || value == "in") {
            error(tok,
                  "'" + value + "' is a joining word, so a line cannot start with it.",
                  "put something in front of it, like: apples > 2 " + value + " pears > 1");
        }
    }

    if (tok.kind == TokKind::Name && peek(1).kind == TokKind::Op && peek(1).value == "=") {
        return parse_assignment();
    }

    ExprPtr expr = parse_expression();
    if (peek().kind == TokKind::Op && peek().value == "=") {
        if (auto* index = dynamic_cast<Index*>(expr.get())) {
            advance();
            ExprPtr value = parse_expression();
            auto node = std::make_unique<SetIndex>();
            node->line = index->line;
            node->target = std::move(index->target);
            node->index = std::move(index->index);
            node->value = std::move(value);
            return node;
        }
    }
    if (dynamic_cast<Call*>(expr.get()) != nullptr || dynamic_cast<Ask*>(expr.get()) != nullptr) {
        auto node = std::make_unique<ExprStmt>();
        node->line = expr->line;
        node->expr = std::move(expr);
        return node;
    }
    error(tok,
          "This line makes an answer (" + tok.describe() + ") but does nothing with it.",
          "use 'oga' to show it, or 'grunk x = ...' to keep it in a box.");
}

StmtPtr Parser::parse_say() {
    int line = advance().line;
    auto node = std::make_unique<Say>();
    node->line = line;
    node->exprs.push_back(parse_expression());
    while (peek().kind == TokKind::Comma) {
        advance();
        node->exprs.push_back(parse_expression());
    }
    return node;
}

StmtPtr Parser::parse_grunk() {
    int line = advance().line;
    Token name_tok = expect_name("After 'grunk' comes the name of the new box.",
                                 "like this: grunk x = 5");
    check_name_ok(name_tok.value, name_tok.line);
    const Token& op = peek();
    if (!(op.kind == TokKind::Op && op.value == "=")) {
        error(op,
              "The box '" + name_tok.value + "' needs a '=' and something to put in it.",
              "like this: grunk " + name_tok.value + " = 5");
    }
    advance();
    auto node = std::make_unique<Assign>();
    node->line = line;
    node->name = name_tok.value;
    node->expr = parse_expression();
    node->declare = true;
    return node;
}

StmtPtr Parser::parse_assignment() {
    Token name_tok = advance();
    advance();
    auto node = std::make_unique<Assign>();
    node->line = name_tok.line;
    node->name = name_tok.value;
    node->expr = parse_expression();
    node->declare = false;
    return node;
}

StmtPtr Parser::parse_if() {
    int line = advance().line;
    ExprPtr cond = parse_expression();
    expect_eol("the 'binga' line");
    std::vector<StmtPtr> body = parse_block({"wonga", "unga"});
    std::vector<StmtPtr> else_body;
    if (match_kw("wonga")) {
        if (at_kw("binga")) {
            StmtPtr nested = parse_if();
            else_body.push_back(std::move(nested));
            auto node = std::make_unique<If>();
            node->line = line;
            node->cond = std::move(cond);
            node->body = std::move(body);
            node->else_body = std::move(else_body);
            return node;
        }
        expect_eol("the 'wonga' line");
        else_body = parse_block({"unga"});
    }
    expect_kw("unga");
    auto node = std::make_unique<If>();
    node->line = line;
    node->cond = std::move(cond);
    node->body = std::move(body);
    node->else_body = std::move(else_body);
    return node;
}

StmtPtr Parser::parse_repeat() {
    int line = advance().line;
    ExprPtr count = parse_expression();
    expect_eol("the 'booga' line");
    std::vector<StmtPtr> body = parse_block({"unga"});
    expect_kw("unga");
    auto node = std::make_unique<Repeat>();
    node->line = line;
    node->count = std::move(count);
    node->body = std::move(body);
    return node;
}

StmtPtr Parser::parse_while() {
    int line = advance().line;
    ExprPtr cond = parse_expression();
    expect_eol("the 'zug' line");
    std::vector<StmtPtr> body = parse_block({"unga"});
    expect_kw("unga");
    auto node = std::make_unique<While>();
    node->line = line;
    node->cond = std::move(cond);
    node->body = std::move(body);
    return node;
}

StmtPtr Parser::parse_foreach() {
    int line = advance().line;
    Token name_tok = expect_name("A 'zoop' needs a name for the loop box, like: zoop pet in pets",
                                 "the loop box holds one thing each lap.");
    check_name_ok(name_tok.value, name_tok.line);
    expect_kw("in", "A 'zoop' needs the little word 'in', like: zoop pet in pets");
    ExprPtr iterable = parse_expression();
    expect_eol("the 'zoop' line");
    std::vector<StmtPtr> body = parse_block({"unga"});
    expect_kw("unga");
    auto node = std::make_unique<ForEach>();
    node->line = line;
    node->name = name_tok.value;
    node->iterable = std::move(iterable);
    node->body = std::move(body);
    return node;
}

StmtPtr Parser::parse_clump() {
    int line = advance().line;
    Token name_tok = expect_name("After 'clump' comes the name of your new word.",
                                 "like this: clump double(n)");
    check_name_ok(name_tok.value, name_tok.line);
    expect_kind(TokKind::LParen, "A clump name needs '(' after it, like: clump double(n)");
    std::vector<std::string> params;
    if (peek().kind != TokKind::RParen) {
        while (true) {
            Token p = expect_name("Inside the parentheses come box names, separated by commas.",
                                  "like this: clump add(a, b)");
            check_name_ok(p.value, p.line);
            params.push_back(p.value);
            if (peek().kind == TokKind::Comma) {
                advance();
                continue;
            }
            break;
        }
    }
    expect_kind(TokKind::RParen, "This clump's box list needs a ')' to close it.");
    expect_eol("the 'clump' line");
    std::vector<StmtPtr> body = parse_block({"unga"});
    expect_kw("unga");
    auto node = std::make_unique<FuncDef>();
    node->line = line;
    node->name = name_tok.value;
    node->params = std::move(params);
    node->body = std::move(body);
    return node;
}

StmtPtr Parser::parse_return() {
    Token tok = advance();
    auto node = std::make_unique<Return>();
    node->line = tok.line;
    if (peek().kind == TokKind::Nl || peek().kind == TokKind::Eof) {
        return node;
    }
    node->expr = parse_expression();
    return node;
}

StmtPtr Parser::parse_skrib() {
    int line = advance().line;
    const Token& shape_tok = peek();
    if (shape_tok.kind != TokKind::Name) {
        error(shape_tok,
              "After 'skrib' comes what to draw, but I found " + shape_tok.describe() + ".",
              "try: skrib circle 100, 100, 30");
    }
    std::string shape_raw = shape_tok.value;
    advance();
    std::string shape = text_lower(shape_raw);
    if (!skrib_shapes().count(shape)) {
        error(shape_tok,
              "'skrib' does not know how to draw '" + shape_raw + "'.",
              "it knows: circle, dot, line, box, blob, write, color, size, clear.");
    }
    std::vector<ExprPtr> args;
    while (peek().kind != TokKind::Nl && peek().kind != TokKind::Eof) {
        if (peek().kind == TokKind::Comma) {
            advance();
            continue;
        }
        args.push_back(parse_expression());
    }
    int need = skrib_shapes().at(shape);
    if (static_cast<int>(args.size()) != need) {
        error(shape_tok,
              "'skrib " + shape + "' needs " + std::to_string(need) + " number"
                  + (need != 1 ? "s" : "") + " after it, but I counted "
                  + std::to_string(args.size()) + ".",
              "put commas between them: " + skrib_hint(shape));
    }
    auto node = std::make_unique<Skrib>();
    node->line = line;
    node->shape = shape;
    node->args = std::move(args);
    return node;
}

std::string Parser::skrib_hint(const std::string& shape) {
    static const std::unordered_map<std::string, std::string> examples = {
        {"clear", "skrib clear"},
        {"size", "skrib size 400, 400"},
        {"color", "skrib color \"red\""},
        {"dot", "skrib dot 100, 100, 10"},
        {"circle", "skrib circle 100, 100, 30"},
        {"line", "skrib line 0, 0, 200, 200"},
        {"box", "skrib box 10, 10, 50, 50"},
        {"blob", "skrib blob 10, 10, 50, 50"},
        {"write", "skrib write \"hi\", 10, 10"},
    };
    auto it = examples.find(shape);
    return it == examples.end() ? "" : it->second;
}

std::vector<StmtPtr> Parser::parse_block(const std::vector<std::string>& terminators) {
    std::vector<StmtPtr> body;
    skip_newlines();
    Token open_tok = peek();
    while (true) {
        const Token& tok = peek();
        if (tok.kind == TokKind::Kw) {
            for (const auto& term : terminators) {
                if (tok.value == term) return body;
            }
        }
        if (tok.kind == TokKind::Eof) {
            throw CmcParseError(
                "A block was opened but never closed.",
                open_tok.line,
                "every 'binga', 'booga', 'zug', 'zoop', and 'clump' block ends with 'unga'.");
        }
        body.push_back(parse_statement());
        expect_eol();
        skip_newlines();
    }
}

ExprPtr Parser::parse_expression() {
    return parse_or();
}

ExprPtr Parser::parse_or() {
    ExprPtr left = parse_and();
    while (at_kw("or")) {
        advance();
        ExprPtr right = parse_and();
        auto node = std::make_unique<LogicOp>();
        node->line = left->line;
        node->op = "or";
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::parse_and() {
    ExprPtr left = parse_not();
    while (at_kw("and")) {
        advance();
        ExprPtr right = parse_not();
        auto node = std::make_unique<LogicOp>();
        node->line = left->line;
        node->op = "and";
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::parse_not() {
    const Token& tok = peek();
    if (tok.kind == TokKind::Kw && tok.value == "not") {
        Token op_tok = advance();
        ExprPtr operand = parse_not();
        auto node = std::make_unique<Not>();
        node->line = op_tok.line;
        node->operand = std::move(operand);
        return node;
    }
    return parse_comparison();
}

ExprPtr Parser::parse_comparison() {
    ExprPtr left = parse_additive();

    if (at_kw("not") && peek(1).kind == TokKind::Kw && peek(1).value == "in") {
        advance();
        advance();
        ExprPtr container = parse_additive();
        auto node = std::make_unique<InOp>();
        node->line = left->line;
        node->item = std::move(left);
        node->container = std::move(container);
        node->negated = true;
        return node;
    }

    const Token& tok = peek();
    if (tok.kind == TokKind::Op && is_compare_op(tok.value)) {
        Token op_tok = advance();
        ExprPtr right = parse_additive();
        auto node = std::make_unique<Compare>();
        node->line = op_tok.line;
        node->op = op_tok.value;
        node->left = std::move(left);
        node->right = std::move(right);
        const Token& nxt = peek();
        if (nxt.kind == TokKind::Op && is_compare_op(nxt.value)) {
            throw CmcParseError(
                "One compare at a time, please!",
                nxt.line,
                "compare two things, then join checks with 'and' or 'or'.");
        }
        return node;
    }

    if (at_kw("in")) {
        advance();
        ExprPtr container = parse_additive();
        auto node = std::make_unique<InOp>();
        node->line = left->line;
        node->item = std::move(left);
        node->container = std::move(container);
        node->negated = false;
        return node;
    }

    return left;
}

ExprPtr Parser::parse_additive() {
    ExprPtr left = parse_multiplicative();
    while (true) {
        const Token& tok = peek();
        if (tok.kind == TokKind::Op && (tok.value == "+" || tok.value == "-")) {
            Token op_tok = advance();
            ExprPtr right = parse_multiplicative();
            auto node = std::make_unique<BinOp>();
            node->line = op_tok.line;
            node->op = op_tok.value;
            node->left = std::move(left);
            node->right = std::move(right);
            left = std::move(node);
        } else {
            return left;
        }
    }
}

ExprPtr Parser::parse_multiplicative() {
    ExprPtr left = parse_unary();
    while (true) {
        const Token& tok = peek();
        if (tok.kind == TokKind::Op && (tok.value == "*" || tok.value == "/" || tok.value == "%")) {
            Token op_tok = advance();
            ExprPtr right = parse_unary();
            auto node = std::make_unique<BinOp>();
            node->line = op_tok.line;
            node->op = op_tok.value;
            node->left = std::move(left);
            node->right = std::move(right);
            left = std::move(node);
        } else {
            return left;
        }
    }
}

ExprPtr Parser::parse_unary() {
    const Token& tok = peek();
    if (tok.kind == TokKind::Op && tok.value == "-") {
        Token op_tok = advance();
        ExprPtr operand = parse_unary();
        auto node = std::make_unique<Neg>();
        node->line = op_tok.line;
        node->operand = std::move(operand);
        return node;
    }
    return parse_postfix();
}

ExprPtr Parser::parse_postfix() {
    ExprPtr expr = parse_primary();
    while (peek().kind == TokKind::LBracket) {
        advance();
        ExprPtr index = parse_expression();
        const Token& close = peek();
        if (close.kind != TokKind::RBracket) {
            error(close, "A spot number needs a ']' after it.", "like this: pets[0]");
        }
        advance();
        auto node = std::make_unique<Index>();
        node->line = expr->line;
        node->target = std::move(expr);
        node->index = std::move(index);
        expr = std::move(node);
    }
    return expr;
}

std::vector<ExprPtr> Parser::parse_args() {
    advance();
    std::vector<ExprPtr> args;
    if (peek().kind == TokKind::RParen) {
        advance();
        return args;
    }
    while (true) {
        args.push_back(parse_expression());
        if (peek().kind == TokKind::Comma) {
            advance();
            continue;
        }
        break;
    }
    const Token& close = peek();
    if (close.kind != TokKind::RParen) {
        error(close, "This list of things needs a ')' to close it.",
              "count the parentheses -- every '(' needs a ')'.");
    }
    advance();
    return args;
}

ExprPtr Parser::parse_primary() {
    Token tok = advance();

    if (tok.kind == TokKind::Num) {
        auto node = std::make_unique<Num>();
        node->line = tok.line;
        node->is_float = tok.is_float;
        node->i = tok.inum;
        node->f = tok.fnum;
        return node;
    }
    if (tok.kind == TokKind::Str) {
        auto node = std::make_unique<Str>();
        node->line = tok.line;
        node->value = tok.value;
        return node;
    }
    if (tok.kind == TokKind::LParen) {
        ExprPtr expr = parse_expression();
        const Token& close = peek();
        if (close.kind != TokKind::RParen) {
            error(close, "This '(' needs a ')' to close it.", "count the parentheses!");
        }
        advance();
        return expr;
    }
    if (tok.kind == TokKind::Name) {
        if (peek().kind == TokKind::LParen) {
            std::vector<ExprPtr> args = parse_args();
            auto node = std::make_unique<Call>();
            node->line = tok.line;
            node->name = tok.value;
            node->args = std::move(args);
            return node;
        }
        auto node = std::make_unique<Var>();
        node->line = tok.line;
        node->name = tok.value;
        return node;
    }
    if (tok.kind == TokKind::Kw && tok.value == "blorp") {
        if (peek().kind == TokKind::LParen) {
            std::vector<ExprPtr> args = parse_args();
            if (args.size() > 1) {
                error(tok,
                      "'blorp' wants one question, not " + std::to_string(args.size()) + ".",
                      "like this: blorp \"What is your name?\"");
            }
            auto node = std::make_unique<Ask>();
            node->line = tok.line;
            if (!args.empty()) node->prompt = std::move(args[0]);
            return node;
        }
        if (peek().kind == TokKind::Nl || peek().kind == TokKind::Eof
            || peek().kind == TokKind::Comma || peek().kind == TokKind::RParen
            || peek().kind == TokKind::RBracket) {
            auto node = std::make_unique<Ask>();
            node->line = tok.line;
            return node;
        }
        ExprPtr prompt = parse_expression();
        auto node = std::make_unique<Ask>();
        node->line = tok.line;
        node->prompt = std::move(prompt);
        return node;
    }

    if (tok.kind == TokKind::Kw) {
        const std::string& value = tok.value;
        if (value == "unga" || value == "wonga" || value == "booga" || value == "binga"
            || value == "zug" || value == "zoop" || value == "clump" || value == "grunk"
            || value == "oga" || value == "skrib") {
            error(tok,
                  "'" + value + "' is a doing word for starting a line, not a thing to use here.",
                  "write a number, some text, or a box name instead.");
        }
        error(tok, "I do not know how to use '" + value + "' here.");
    }

    error(tok,
          "I expected something to use here, but found " + tok.describe() + ".",
          "try a number like 5, text like \"hi\", or a box name.");
}

std::unique_ptr<Program> parse_source(const std::string& source, const std::string& filename) {
    std::vector<Token> tokens = tokenize(source, filename);
    Parser parser(std::move(tokens), filename);
    return parser.parse_program();
}

ExprPtr parse_expression_source(const std::string& source, const std::string& filename) {
    std::vector<Token> tokens = tokenize(source, filename);
    Parser parser(std::move(tokens), filename);
    ExprPtr expr = parser.parse_expression();
    const Token& tok = parser.peek();
    if (tok.kind != TokKind::Nl && tok.kind != TokKind::Eof) {
        throw CmcParseError(
            "There are extra things after the expression: " + tok.describe() + ".",
            tok.line);
    }
    return expr;
}

}
