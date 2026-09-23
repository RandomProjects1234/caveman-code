#include "textutil.hpp"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cmc {

namespace {

std::string ascii_case(const std::string& s, bool upper) {
    std::string out = s;
    for (char& c : out) {
        if (upper) {
            if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
        } else {
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return out;
}

bool is_ascii(const std::string& s) {
    for (unsigned char c : s) {
        if (c >= 0x80) return false;
    }
    return true;
}

std::string map_case(const std::string& s, bool upper) {
    if (is_ascii(s)) return ascii_case(s, upper);
#ifdef _WIN32
    int wide_length = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()),
                                          nullptr, 0);
    if (wide_length <= 0) return ascii_case(s, upper);
    std::wstring wide(static_cast<std::size_t>(wide_length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), wide.data(),
                        wide_length);
    DWORD flags = upper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE;
    int needed = LCMapStringW(LOCALE_INVARIANT, flags, wide.c_str(), wide_length, nullptr, 0);
    if (needed <= 0) return ascii_case(s, upper);
    std::wstring mapped(static_cast<std::size_t>(needed), L'\0');
    int got = LCMapStringW(LOCALE_INVARIANT, flags, wide.c_str(), wide_length, mapped.data(),
                           needed);
    if (got <= 0) return ascii_case(s, upper);
    int utf8_length = WideCharToMultiByte(CP_UTF8, 0, mapped.c_str(), got, nullptr, 0, nullptr,
                                          nullptr);
    if (utf8_length <= 0) return ascii_case(s, upper);
    std::string out(static_cast<std::size_t>(utf8_length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, mapped.c_str(), got, out.data(), utf8_length, nullptr,
                        nullptr);
    return out;
#else
    return ascii_case(s, upper);
#endif
}

bool is_continuation(unsigned char c) {
    return (c & 0xC0) == 0x80;
}

std::size_t char_length(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool valid_underscores(const std::string& s) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '_') {
            if (i == 0 || i + 1 >= s.size()) return false;
            if (!is_digit(s[i - 1]) || !is_digit(s[i + 1])) return false;
        }
    }
    return true;
}

std::string strip_underscores(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c != '_') out.push_back(c);
    }
    return out;
}

std::string shortest_double(double d) {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    char buffer[64];
    auto result = std::to_chars(buffer, buffer + sizeof(buffer), d);
    if (result.ec == std::errc()) {
        return std::string(buffer, result.ptr);
    }
#endif
    for (int precision = 15; precision <= 17; ++precision) {
        std::ostringstream stream;
        stream.precision(precision);
        stream << d;
        std::string text = stream.str();
        double back = std::strtod(text.c_str(), nullptr);
        if (back == d) return text;
    }
    std::ostringstream stream;
    stream.precision(17);
    stream << d;
    return stream.str();
}

}

std::size_t utf8_length(const std::string& s) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < s.size();) {
        i += char_length(static_cast<unsigned char>(s[i]));
        ++count;
    }
    return count;
}

std::string utf8_at(const std::string& s, long long index) {
    long long count = 0;
    for (std::size_t i = 0; i < s.size();) {
        std::size_t len = char_length(static_cast<unsigned char>(s[i]));
        if (count == index) return s.substr(i, len);
        i += len;
        ++count;
    }
    return "";
}

std::string utf8_reverse(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    std::size_t end = s.size();
    while (end > 0) {
        std::size_t start = end - 1;
        while (start > 0 && is_continuation(static_cast<unsigned char>(s[start]))) --start;
        out.append(s, start, end - start);
        end = start;
    }
    return out;
}

long long utf8_find(const std::string& haystack, const std::string& needle) {
    std::size_t byte_pos = haystack.find(needle);
    if (byte_pos == std::string::npos) return -1;
    long long count = 0;
    for (std::size_t i = 0; i < byte_pos;) {
        i += char_length(static_cast<unsigned char>(haystack[i]));
        ++count;
    }
    return count;
}

std::string text_upper(const std::string& s) {
    return map_case(s, true);
}

std::string text_lower(const std::string& s) {
    return map_case(s, false);
}

std::string trim_ascii(const std::string& s) {
    std::size_t start = 0;
    std::size_t end = s.size();
    auto space = [](char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
    };
    while (start < end && space(s[start])) ++start;
    while (end > start && space(s[end - 1])) --end;
    return s.substr(start, end - start);
}

std::string py_repr_string(const std::string& s) {
    bool has_single = s.find('\'') != std::string::npos;
    bool has_double = s.find('"') != std::string::npos;
    char quote = (has_single && !has_double) ? '"' : '\'';
    std::string out;
    out.push_back(quote);
    for (unsigned char c : s) {
        if (c == '\\') {
            out += "\\\\";
        } else if (c == static_cast<unsigned char>(quote)) {
            out.push_back('\\');
            out.push_back(static_cast<char>(c));
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else if (c < 0x20 || c == 0x7f) {
            static const char* hex = "0123456789abcdef";
            out += "\\x";
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    out.push_back(quote);
    return out;
}

std::string py_repr_double(double d) {
    if (std::isnan(d)) return "nan";
    if (std::isinf(d)) return d < 0 ? "-inf" : "inf";
    std::string text = shortest_double(d);
    if (text.find('.') == std::string::npos && text.find('e') == std::string::npos
        && text.find('E') == std::string::npos && text.find("inf") == std::string::npos
        && text.find("nan") == std::string::npos) {
        text += ".0";
    }
    return text;
}

std::string format_show_double(double d) {
    if (std::isfinite(d) && std::floor(d) == d && std::fabs(d) < 9.0e18) {
        long long whole = static_cast<long long>(d);
        return std::to_string(whole);
    }
    return py_repr_double(d);
}

bool parse_python_int(const std::string& text, long long& out) {
    if (text.empty()) return false;
    if (!valid_underscores(text)) return false;
    std::string clean = strip_underscores(text);
    std::size_t pos = 0;
    bool negative = false;
    if (clean[0] == '+' || clean[0] == '-') {
        negative = clean[0] == '-';
        pos = 1;
    }
    if (pos >= clean.size()) return false;
    for (std::size_t i = pos; i < clean.size(); ++i) {
        if (!is_digit(clean[i])) return false;
    }
    errno = 0;
    char* end = nullptr;
    long long value = std::strtoll(clean.c_str(), &end, 10);
    if (end == nullptr || *end != '\0' || errno == ERANGE) return false;
    out = negative ? value : value;
    return true;
}

bool parse_python_float(const std::string& text, double& out) {
    if (text.empty()) return false;
    if (!valid_underscores(text)) return false;
    std::string clean = strip_underscores(text);
    std::string lowered = text_lower(clean);
    if (lowered == "inf" || lowered == "+inf" || lowered == "infinity" || lowered == "+infinity") {
        out = std::numeric_limits<double>::infinity();
        return true;
    }
    if (lowered == "-inf" || lowered == "-infinity") {
        out = -std::numeric_limits<double>::infinity();
        return true;
    }
    if (lowered == "nan" || lowered == "+nan" || lowered == "-nan") {
        out = std::numeric_limits<double>::quiet_NaN();
        return true;
    }
    errno = 0;
    char* end = nullptr;
    double value = std::strtod(clean.c_str(), &end);
    if (end == clean.c_str() || end == nullptr || *end != '\0') return false;
    out = value;
    return true;
}

}
