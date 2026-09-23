#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cmc {
namespace words {

extern const char* VERSION;

const std::unordered_map<std::string, std::string>& aliases();
const std::unordered_set<std::string>& keywords();

bool keyword_alias(const std::string& lower, std::string& canonical);
bool constant_alias(const std::string& lower, std::string& canonical);
bool is_forbidden_name(const std::string& name);

const std::unordered_map<std::string, std::pair<int, int>>& arity();
const std::vector<std::string>& builtin_names();

}
}
