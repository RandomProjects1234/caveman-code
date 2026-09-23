#pragma once

#include <string>
#include <vector>

namespace cmc {

enum class TokKind {
    Kw,
    Name,
    Num,
    Str,
    Op,
    LParen,
    RParen,
    LBracket,
    RBracket,
    Comma,
    Nl,
    Eof,
};

struct Token {
    TokKind kind = TokKind::Eof;
    std::string value;
    int line = 1;
    std::string raw;
    long long inum = 0;
    double fnum = 0.0;
    bool is_float = false;

    std::string describe() const;
};

std::vector<Token> tokenize(const std::string& source, const std::string& filename = "<cmc>");

}
