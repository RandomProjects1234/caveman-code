#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

namespace cmc {

class Parser {
public:
    Parser(std::vector<Token> tokens, std::string filename);

    std::unique_ptr<Program> parse_program();
    ExprPtr parse_expression();

    Token& peek(std::size_t ahead = 0);
    const Token& peek(std::size_t ahead = 0) const;

private:
    Token advance();
    bool at_kw(const std::vector<std::string>& names) const;
    bool at_kw(const std::string& name) const;
    bool match_kw(const std::string& name);
    Token expect_kw(const std::string& name, const std::string& message = "",
                    const std::string& hint = "");
    void expect_eol(const std::string& what = "this line");
    Token expect_name(const std::string& message, const std::string& hint = "");
    Token expect_kind(TokKind kind, const std::string& message, const std::string& hint = "");
    void check_name_ok(const std::string& name, int line);
    [[noreturn]] void error(const Token& tok, const std::string& message,
                            const std::string& hint = "");
    void skip_newlines();

    StmtPtr parse_statement();
    StmtPtr parse_say();
    StmtPtr parse_grunk();
    StmtPtr parse_assignment();
    StmtPtr parse_if();
    StmtPtr parse_repeat();
    StmtPtr parse_while();
    StmtPtr parse_foreach();
    StmtPtr parse_clump();
    StmtPtr parse_return();
    StmtPtr parse_skrib();
    std::vector<StmtPtr> parse_block(const std::vector<std::string>& terminators);

    ExprPtr parse_or();
    ExprPtr parse_and();
    ExprPtr parse_not();
    ExprPtr parse_comparison();
    ExprPtr parse_additive();
    ExprPtr parse_multiplicative();
    ExprPtr parse_unary();
    ExprPtr parse_postfix();
    std::vector<ExprPtr> parse_args();
    ExprPtr parse_primary();

    static std::string skrib_hint(const std::string& shape);

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
    std::string filename_;
};

std::unique_ptr<Program> parse_source(const std::string& source,
                                      const std::string& filename = "<cmc>");
ExprPtr parse_expression_source(const std::string& source,
                                const std::string& filename = "<cmc>");

}
