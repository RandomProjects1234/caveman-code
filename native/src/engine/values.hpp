#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace cmc {

struct Pile;

class Value {
public:
    using Storage =
        std::variant<std::monostate, bool, long long, double, std::string, std::shared_ptr<Pile>>;

    Storage data;

    Value() : data(std::monostate{}) {}

    static Value plop();
    static Value truth(bool b);
    static Value integer(long long n);
    static Value real(double d);
    static Value text(std::string s);
    static Value text(const char* s);
    static Value pile();
    static Value pile(std::shared_ptr<Pile> p);

    bool is_plop() const;
    bool is_bool() const;
    bool is_int() const;
    bool is_float() const;
    bool is_number() const;
    bool is_text() const;
    bool is_pile() const;

    bool as_bool() const;
    long long as_int() const;
    double as_float() const;
    double as_number() const;
    const std::string& as_text() const;
    std::shared_ptr<Pile> as_pile() const;
};

struct Pile {
    std::vector<Value> items;
};

std::string type_name(const Value& v);
std::string type_word(const Value& v);
std::string show(const Value& v);
bool truthy(const Value& v);
bool equal(const Value& a, const Value& b);
bool isnum(const Value& v);
Value number(const Value& v, int line, const std::string& extra = "");
Value whole(const Value& v, int line, const std::string& extra = "");

}
