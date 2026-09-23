#include "lexer.hpp"

#include <cctype>
#include <cerrno>
#include <cstdlib>

#include "errors.hpp"
#include "textutil.hpp"
#include "words.hpp"

namespace cmc {

namespace {

const char* COMMENT_WORDS[] = {"ugg", "comment"};

bool is_word_start(unsigned char c) {
    return std::isalpha(c) != 0 || c == '_' || c >= 0x80;
}

bool is_word_part(unsigned char c) {
    return std::isalnum(c) != 0 || c == '_' || c >= 0x80;
}

TokKind punct_kind(char c) {
    switch (c) {
        case '(':
            return TokKind::LParen;
        case ')':
            return TokKind::RParen;
        case '[':
            return TokKind::LBracket;
        case ']':
            return TokKind::RBracket;
        case ',':
            return TokKind::Comma;
        default:
            return TokKind::Eof;
    }
}

bool is_punct(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']' || c == ',';
}

bool is_two_char_op(const std::string& s) {
    return s == "==" || s == "!=" || s == "<=" || s == ">=";
}

bool is_one_char_op(char c) {
    return c == '=' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == '/'
           || c == '%';
}

std::string simple_escape(char c, bool& found) {
    found = true;
    switch (c) {
        case 'n':
            return "\n";
        case 't':
            return "\t";
        case 'r':
            return "\r";
        case '\\':
            return "\\";
        case '"':
            return "\"";
        case '\'':
            return "'";
        case '0':
            return std::string(1, '\0');
        default:
            found = false;
            return "";
    }
}

void append_utf8(std::string& out, unsigned int code) {
    if (code < 0x80) {
        out.push_back(static_cast<char>(code));
    } else if (code < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (code >> 6)));
        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else if (code < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (code >> 12)));
        out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (code >> 18)));
        out.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
}

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename)
        : source_(source), filename_(filename) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (!at_end()) {
            char ch = source_[pos_];
            if (ch == '\r') {
                ++pos_;
                continue;
            }
            if (ch == '\n') {
                Token tok;
                tok.kind = TokKind::Nl;
                tok.value = "\n";
                tok.raw = "\n";
                tok.line = line_;
                tokens.push_back(tok);
                ++line_;
                ++pos_;
                continue;
            }
            if (ch == ' ' || ch == '\t') {
                ++pos_;
                continue;
            }
            if (is_word_start(static_cast<unsigned char>(ch))) {
                read_word(tokens);
                continue;
            }
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                read_number(tokens);
                continue;
            }
            if (ch == '"' || ch == '\'') {
                read_string(tokens);
                continue;
            }
            if (is_punct(ch)) {
                Token tok;
                tok.kind = punct_kind(ch);
                tok.value = std::string(1, ch);
                tok.raw = tok.value;
                tok.line = line_;
                tokens.push_back(tok);
                ++pos_;
                continue;
            }
            std::string two = std::string(1, ch) + peek();
            if (is_two_char_op(two)) {
                Token tok;
                tok.kind = TokKind::Op;
                tok.value = two;
                tok.raw = two;
                tok.line = line_;
                tokens.push_back(tok);
                pos_ += 2;
                continue;
            }
            if (is_one_char_op(ch)) {
                Token tok;
                tok.kind = TokKind::Op;
                tok.value = std::string(1, ch);
                tok.raw = tok.value;
                tok.line = line_;
                tokens.push_back(tok);
                ++pos_;
                continue;
            }
            if (ch == '!') {
                throw CmcLexError(
                    "I found a single '!' but I do not know what it means.",
                    line_,
                    "to say 'not equal' write '!='. To flip a check use the word 'not'.");
            }
            if (ch == '&' || ch == '|') {
                throw CmcLexError(
                    "I found '" + std::string(1, ch) + "' but CMC uses words for that.",
                    line_,
                    "use 'and' or 'or' between checks.");
            }
            if (ch == ';') {
                throw CmcLexError(
                    "I found a ';'. CMC does not use those.",
                    line_,
                    "one thing per line is plenty. Just press Enter.");
            }
            throw CmcLexError(
                "I do not know the squiggle '" + std::string(1, ch) + "'.",
                line_,
                "CMC words are made of letters, numbers, and _");
        }

        if (tokens.empty() || tokens.back().kind != TokKind::Nl) {
            Token tok;
            tok.kind = TokKind::Nl;
            tok.value = "\n";
            tok.raw = "\n";
            tok.line = line_;
            tokens.push_back(tok);
        }
        Token eof;
        eof.kind = TokKind::Eof;
        eof.line = line_;
        tokens.push_back(eof);
        return tokens;
    }

private:
    bool at_end() const {
        return pos_ >= source_.size();
    }

    std::string peek(std::size_t ahead = 1) const {
        std::size_t spot = pos_ + ahead;
        if (spot < source_.size()) return std::string(1, source_[spot]);
        return "";
    }

    void read_word(std::vector<Token>& tokens) {
        std::size_t start = pos_;
        while (!at_end() && is_word_part(static_cast<unsigned char>(source_[pos_]))) {
            ++pos_;
        }
        std::string raw = source_.substr(start, pos_ - start);
        std::string lower = text_lower(raw);

        for (const char* comment : COMMENT_WORDS) {
            if (lower == comment) {
                skip_comment();
                return;
            }
        }

        std::string canonical;
        if (words::keyword_alias(lower, canonical)) {
            Token tok;
            tok.kind = TokKind::Kw;
            tok.value = canonical;
            tok.raw = lower;
            tok.line = line_;
            tokens.push_back(tok);
            return;
        }
        if (words::constant_alias(lower, canonical)) {
            Token tok;
            tok.kind = TokKind::Name;
            tok.value = canonical;
            tok.raw = lower;
            tok.line = line_;
            tokens.push_back(tok);
            return;
        }

        Token tok;
        tok.kind = TokKind::Name;
        tok.value = raw;
        tok.raw = raw;
        tok.line = line_;
        tokens.push_back(tok);
    }

    void skip_comment() {
        while (!at_end() && source_[pos_] != '\n') ++pos_;
    }

    void read_number(std::vector<Token>& tokens) {
        std::size_t start = pos_;
        bool seen_dot = false;
        while (!at_end()) {
            char ch = source_[pos_];
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                ++pos_;
            } else if (ch == '.' && !seen_dot) {
                seen_dot = true;
                ++pos_;
            } else {
                break;
            }
        }
        std::string raw = source_.substr(start, pos_ - start);
        if (!raw.empty() && raw.back() == '.') {
            throw CmcLexError(
                "The number " + raw + " is missing the numbers after the dot.",
                line_,
                "write something like 3.5 instead.");
        }
        if (peek() == ".") {
            throw CmcLexError(
                "There are too many dots in that number.",
                line_,
                "a number can only have one dot, like 3.5");
        }
        Token tok;
        tok.kind = TokKind::Num;
        tok.raw = raw;
        tok.line = line_;
        if (seen_dot) {
            tok.is_float = true;
            tok.fnum = std::strtod(raw.c_str(), nullptr);
        } else {
            tok.is_float = false;
            errno = 0;
            char* end = nullptr;
            long long value = std::strtoll(raw.c_str(), &end, 10);
            if (errno == ERANGE) {
                tok.is_float = true;
                tok.fnum = std::strtod(raw.c_str(), nullptr);
            } else {
                tok.inum = value;
            }
        }
        tokens.push_back(tok);
    }

    void read_string(std::vector<Token>& tokens) {
        char quote = source_[pos_];
        int start_line = line_;
        ++pos_;
        std::string pieces;
        while (true) {
            if (at_end()) {
                throw CmcLexError(
                    "Some text started with a " + std::string(1, quote) + " but never finished.",
                    start_line,
                    "add a matching " + std::string(1, quote) + " at the end of the text.");
            }
            char ch = source_[pos_];
            if (ch == '\\') {
                std::string nxt = peek();
                if (nxt.empty()) {
                    throw CmcLexError(
                        "The text ends with a lonely backslash.",
                        line_,
                        "write \\\\ if you really want a backslash.");
                }
                bool found = false;
                std::string escaped = simple_escape(nxt[0], found);
                if (found) {
                    pieces += escaped;
                    pos_ += 2;
                    continue;
                }
                if (nxt[0] == 'u') {
                    if (pos_ + 6 <= source_.size()) {
                        std::string hexdigits = source_.substr(pos_ + 2, 4);
                        int value = 0;
                        bool ok = true;
                        for (char h : hexdigits) {
                            int digit = hex_value(h);
                            if (digit < 0) {
                                ok = false;
                                break;
                            }
                            value = value * 16 + digit;
                        }
                        if (ok) {
                            append_utf8(pieces, static_cast<unsigned int>(value));
                            pos_ += 6;
                            continue;
                        }
                    }
                    throw CmcLexError(
                        "That \\u escape needs four numbers after it, like \\u2764.",
                        line_);
                }
                pieces += nxt;
                pos_ += 2;
                continue;
            }
            if (ch == quote) {
                ++pos_;
                break;
            }
            if (ch == '\n') {
                throw CmcLexError(
                    "Text cannot go over more than one line.",
                    start_line,
                    "close the text with " + std::string(1, quote) + " before pressing Enter.");
            }
            pieces.push_back(ch);
            ++pos_;
        }
        Token tok;
        tok.kind = TokKind::Str;
        tok.value = pieces;
        tok.raw = pieces;
        tok.line = line_;
        tokens.push_back(tok);
    }

    std::string source_;
    std::string filename_;
    std::size_t pos_ = 0;
    int line_ = 1;
};

}

std::string Token::describe() const {
    if (kind == TokKind::Nl) return "the end of the line";
    if (kind == TokKind::Eof) return "the end of the program";
    if (kind == TokKind::Str) return "text " + py_repr_string(raw);
    return "'" + raw + "'";
}

std::vector<Token> tokenize(const std::string& source, const std::string& filename) {
    return Lexer(source, filename).tokenize();
}

}
