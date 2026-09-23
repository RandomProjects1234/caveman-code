#include "words.hpp"

#include "textutil.hpp"

namespace cmc {
namespace words {

const char* VERSION = "0.1.0";

const std::unordered_map<std::string, std::string>& aliases() {
    static const std::unordered_map<std::string, std::string> table = {
        {"true", "gronk"},
        {"false", "nork"},
        {"nothing", "plop"},
        {"oga", "oga"},
        {"print", "oga"},
        {"say", "oga"},
        {"blorp", "blorp"},
        {"ask", "blorp"},
        {"grunk", "grunk"},
        {"let", "grunk"},
        {"make", "grunk"},
        {"binga", "binga"},
        {"if", "binga"},
        {"wonga", "wonga"},
        {"else", "wonga"},
        {"unga", "unga"},
        {"end", "unga"},
        {"done", "unga"},
        {"finish", "unga"},
        {"booga", "booga"},
        {"repeat", "booga"},
        {"zug", "zug"},
        {"while", "zug"},
        {"zoop", "zoop"},
        {"for", "zoop"},
        {"clump", "clump"},
        {"fn", "clump"},
        {"function", "clump"},
        {"def", "clump"},
        {"ork", "ork"},
        {"return", "ork"},
        {"give", "ork"},
        {"skrib", "skrib"},
        {"draw", "skrib"},
        {"ugg", "ugg"},
        {"comment", "ugg"},
        {"and", "and"},
        {"or", "or"},
        {"not", "not"},
        {"in", "in"},
        {"gronk", "gronk"},
        {"nork", "nork"},
        {"plop", "plop"},
        {"none", "plop"},
        {"pi", "pi"},
        {"snorf", "snorf"},
        {"plop_call", "plop_call"},
        {"nom", "nom"},
        {"skoop", "skoop"},
        {"yoink", "yoink"},
        {"goop", "goop"},
        {"shout", "shout"},
        {"whisper", "whisper"},
        {"flip", "flip"},
        {"find", "find"},
        {"split", "split"},
        {"join", "join"},
        {"what", "what"},
        {"munga", "munga"},
        {"numba", "numba"},
        {"round", "round"},
        {"flat", "flat"},
        {"roof", "roof"},
        {"abs", "abs"},
        {"small", "small"},
        {"big", "big"},
        {"root", "root"},
        {"pow", "pow"},
        {"wait", "wait"},
    };
    return table;
}

const std::unordered_set<std::string>& keywords() {
    static const std::unordered_set<std::string> table = {
        "oga", "grunk", "binga", "wonga", "unga", "booga", "zug",
        "zoop", "clump", "ork", "skrib", "blorp", "and", "or",
        "not", "in", "ugg",
    };
    return table;
}

bool keyword_alias(const std::string& lower, std::string& canonical) {
    auto it = aliases().find(lower);
    if (it == aliases().end()) return false;
    if (!keywords().count(it->second)) return false;
    canonical = it->second;
    return true;
}

bool constant_alias(const std::string& lower, std::string& canonical) {
    auto it = aliases().find(lower);
    if (it == aliases().end()) return false;
    const std::string& canon = it->second;
    if ((canon == "gronk" || canon == "nork" || canon == "plop") && lower != canon) {
        canonical = canon;
        return true;
    }
    return false;
}

const std::vector<std::string>& builtin_names() {
    static const std::vector<std::string> table = {
        "snorf", "plop_call", "nom", "skoop", "yoink", "goop", "shout",
        "whisper", "flip", "find", "split", "join", "what", "munga",
        "numba", "round", "flat", "roof", "abs", "small", "big", "root",
        "pow", "wait",
    };
    return table;
}

bool is_forbidden_name(const std::string& name) {
    static const std::unordered_set<std::string> table = [] {
        std::unordered_set<std::string> set;
        for (const auto& n : builtin_names()) set.insert(n);
        set.insert("gronk");
        set.insert("nork");
        set.insert("plop");
        set.insert("pi");
        return set;
    }();
    return table.count(text_lower(name)) > 0;
}

const std::unordered_map<std::string, std::pair<int, int>>& arity() {
    static const std::unordered_map<std::string, std::pair<int, int>> table = {
        {"snorf", {0, 99}},
        {"plop", {2, 2}},
        {"nom", {1, 1}},
        {"skoop", {2, 2}},
        {"yoink", {1, 1}},
        {"goop", {1, 1}},
        {"shout", {1, 1}},
        {"whisper", {1, 1}},
        {"flip", {1, 1}},
        {"find", {2, 2}},
        {"split", {2, 2}},
        {"join", {2, 2}},
        {"what", {1, 1}},
        {"munga", {2, 2}},
        {"numba", {1, 1}},
        {"round", {1, 1}},
        {"flat", {1, 1}},
        {"roof", {1, 1}},
        {"abs", {1, 1}},
        {"small", {2, 2}},
        {"big", {2, 2}},
        {"root", {1, 1}},
        {"pow", {2, 2}},
        {"wait", {1, 1}},
    };
    return table;
}

}
}
