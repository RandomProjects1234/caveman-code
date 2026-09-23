#pragma once

#include <string>
#include <vector>

#include "values.hpp"

namespace cmc {

class Interpreter;

bool is_builtin(const std::string& name);
Value call_builtin(Interpreter& interp, const std::string& name, const std::vector<Value>& args,
                   int line);

}
