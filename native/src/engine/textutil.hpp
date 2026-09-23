#pragma once

#include <cstddef>
#include <string>

namespace cmc {

std::size_t utf8_length(const std::string& s);
std::string utf8_at(const std::string& s, long long index);
std::string utf8_reverse(const std::string& s);
long long utf8_find(const std::string& haystack, const std::string& needle);

std::string text_upper(const std::string& s);
std::string text_lower(const std::string& s);
std::string trim_ascii(const std::string& s);

std::string py_repr_string(const std::string& s);
std::string py_repr_double(double d);
std::string format_show_double(double d);

bool parse_python_int(const std::string& text, long long& out);
bool parse_python_float(const std::string& text, double& out);

}
