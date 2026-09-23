#pragma once

#include <string>

#include "ast.hpp"

namespace cmc {

std::string to_python_source(const std::string& source, const std::string& filename = "<cmc>");
std::string expr_to_cmc(const Expr* node);

}
