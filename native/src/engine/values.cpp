#include "values.hpp"

#include <cmath>

#include "errors.hpp"
#include "textutil.hpp"

namespace cmc {

Value Value::plop() {
    return Value();
}

Value Value::truth(bool b) {
    Value v;
    v.data = b;
    return v;
}

Value Value::integer(long long n) {
    Value v;
    v.data = n;
    return v;
}

Value Value::real(double d) {
    Value v;
    v.data = d;
    return v;
}

Value Value::text(std::string s) {
    Value v;
    v.data = std::move(s);
    return v;
}

Value Value::text(const char* s) {
    Value v;
    v.data = std::string(s);
    return v;
}

Value Value::pile() {
    Value v;
    v.data = std::make_shared<Pile>();
    return v;
}

Value Value::pile(std::shared_ptr<Pile> p) {
    Value v;
    v.data = std::move(p);
    return v;
}

bool Value::is_plop() const {
    return std::holds_alternative<std::monostate>(data);
}

bool Value::is_bool() const {
    return std::holds_alternative<bool>(data);
}

bool Value::is_int() const {
    return std::holds_alternative<long long>(data);
}

bool Value::is_float() const {
    return std::holds_alternative<double>(data);
}

bool Value::is_number() const {
    return is_int() || is_float();
}

bool Value::is_text() const {
    return std::holds_alternative<std::string>(data);
}

bool Value::is_pile() const {
    return std::holds_alternative<std::shared_ptr<Pile>>(data);
}

bool Value::as_bool() const {
    return std::get<bool>(data);
}

long long Value::as_int() const {
    return std::get<long long>(data);
}

double Value::as_float() const {
    return std::get<double>(data);
}

double Value::as_number() const {
    return is_int() ? static_cast<double>(as_int()) : as_float();
}

const std::string& Value::as_text() const {
    return std::get<std::string>(data);
}

std::shared_ptr<Pile> Value::as_pile() const {
    return std::get<std::shared_ptr<Pile>>(data);
}

std::string type_name(const Value& v) {
    if (v.is_plop()) return "plop";
    if (v.is_bool()) return "truth";
    if (v.is_number()) return "number";
    if (v.is_text()) return "text";
    if (v.is_pile()) return "pile";
    return "thing";
}

std::string type_word(const Value& v) {
    std::string name = type_name(v);
    if (name == "plop") return "nothing (plop)";
    if (name == "truth") return "a truth (gronk or nork)";
    if (name == "number") return "a number";
    if (name == "text") return "text (goop)";
    if (name == "pile") return "a pile (snorf)";
    return "a strange thing";
}

std::string show(const Value& v) {
    if (v.is_plop()) return "plop";
    if (v.is_bool()) return v.as_bool() ? "gronk" : "nork";
    if (v.is_int()) return std::to_string(v.as_int());
    if (v.is_float()) return format_show_double(v.as_float());
    if (v.is_text()) return v.as_text();
    if (v.is_pile()) {
        auto pile = v.as_pile();
        std::string out = "[";
        for (std::size_t i = 0; i < pile->items.size(); ++i) {
            if (i > 0) out += ", ";
            out += show(pile->items[i]);
        }
        out += "]";
        return out;
    }
    return "plop";
}

bool truthy(const Value& v) {
    if (v.is_plop()) return false;
    if (v.is_bool()) return v.as_bool();
    if (v.is_int()) return v.as_int() != 0;
    if (v.is_float()) return v.as_float() != 0.0;
    if (v.is_text()) return !v.as_text().empty();
    if (v.is_pile()) return !v.as_pile()->items.empty();
    return true;
}

bool equal(const Value& a, const Value& b) {
    if (a.is_bool() || b.is_bool()) {
        return a.is_bool() && b.is_bool() && a.as_bool() == b.as_bool();
    }
    if (a.is_plop() || b.is_plop()) {
        return a.is_plop() && b.is_plop();
    }
    if (a.is_number() && b.is_number()) {
        if (a.is_int() && b.is_int()) return a.as_int() == b.as_int();
        return a.as_number() == b.as_number();
    }
    if (a.is_text() && b.is_text()) return a.as_text() == b.as_text();
    if (a.is_pile() && b.is_pile()) {
        auto pa = a.as_pile();
        auto pb = b.as_pile();
        if (pa->items.size() != pb->items.size()) return false;
        for (std::size_t i = 0; i < pa->items.size(); ++i) {
            if (!equal(pa->items[i], pb->items[i])) return false;
        }
        return true;
    }
    return false;
}

bool isnum(const Value& v) {
    return v.is_number();
}

Value number(const Value& v, int line, const std::string& extra) {
    if (!v.is_number()) {
        std::string what = extra.empty() ? "This needs" : extra + " needs";
        throw CmcRuntimeError(
            what + " a number, but got " + type_word(v) + ".",
            line,
            "use numbers like 1, 2, or 3.5 -- or wrap things in 'goop' to make text.");
    }
    return v;
}

Value whole(const Value& v, int line, const std::string& extra) {
    Value n = number(v, line, extra);
    if (n.is_float()) {
        double d = n.as_float();
        if (std::floor(d) == d) return Value::integer(static_cast<long long>(d));
        std::string what = extra.empty() ? "This needs" : extra + " needs";
        throw CmcRuntimeError(
            what + " a whole number, but got " + show(n) + ".",
            line,
            "try 'round' or 'flat' to make it whole.");
    }
    return n;
}

}
