#pragma once

#include <string>
#include <vector>

namespace cmc {
namespace words {

struct WordInfo {
    std::string name;
    std::string display;
    std::string category;
    std::string summary;
    std::string syntax;
    std::string doc;
    std::string example;
    std::vector<std::string> aliases;
    std::vector<std::string> extra_syntax;
    std::vector<std::string> tips;
    std::vector<std::string> related;
    int arity_low = 0;
    int arity_high = 0;
    bool is_builtin = false;
};

const std::vector<WordInfo>& word_infos();

const std::vector<std::string>& categories();

}
}