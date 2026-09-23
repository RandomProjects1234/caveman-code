#include "ide_blocks.hpp"

#include <cctype>
#include <set>
#include <typeinfo>

#include "ast.hpp"
#include "codegen.hpp"
#include "errors.hpp"
#include "parser.hpp"
#include "words.hpp"

namespace og {

using cmc::Ask;
using cmc::Assign;
using cmc::ExprStmt;
using cmc::FuncDef;
using cmc::If;
using cmc::InOp;
using cmc::Repeat;
using cmc::Return;
using cmc::Say;
using cmc::SetIndex;
using cmc::Skrib;
using cmc::While;
using cmc::ForEach;
using cmc::Call;

namespace {

long long next_uid() {
    static long long counter = 0;
    return ++counter;
}

BlockSpec make_skrib(const std::string& shape, const std::vector<std::string>& args,
                     const std::map<std::string, std::string>& defaults) {
    BlockSpec spec;
    spec.title = "skrib " + shape;
    spec.category = "Drawing";
    spec.defaults = defaults;
    for (const auto& arg : args) {
        FieldSpec field;
        field.key = arg;
        field.label = arg;
        field.type = "expr";
        field.width = (arg == "words" || arg == "color") ? 120 : 46;
        spec.fields.push_back(field);
    }
    return spec;
}

std::unordered_map<std::string, BlockSpec> build_specs() {
    std::unordered_map<std::string, BlockSpec> table;
    auto add = [&table](const std::string& kind, BlockSpec spec) { table[kind] = std::move(spec); };

    {
        BlockSpec spec;
        spec.title = "oga";
        spec.category = "Talking";
        spec.fields = {{"expr", "say", "expr", 190}};
        spec.defaults = {{"expr", "\"hello cave!\""}};
        add("say", spec);
    }
    {
        BlockSpec spec;
        spec.title = "ask";
        spec.category = "Talking";
        spec.fields = {{"name", "into", "name", 80}, {"prompt", "question", "expr", 170}};
        spec.defaults = {{"name", "answer"}, {"prompt", "\"What?\""}};
        add("ask", spec);
    }
    {
        BlockSpec spec;
        spec.title = "make box";
        spec.category = "Boxes";
        spec.fields = {{"name", "name", "name", 80}, {"expr", "=", "expr", 150}};
        spec.defaults = {{"name", "x"}, {"expr", "5"}};
        add("grunk", spec);
    }
    {
        BlockSpec spec;
        spec.title = "change box";
        spec.category = "Boxes";
        spec.fields = {{"name", "name", "name", 80}, {"expr", "=", "expr", 150}};
        spec.defaults = {{"name", "x"}, {"expr", "6"}};
        add("set", spec);
    }
    {
        BlockSpec spec;
        spec.title = "change a spot";
        spec.category = "Boxes";
        spec.fields = {{"target", "pile", "expr", 80}, {"index", "spot", "expr", 44},
                       {"value", "=", "expr", 110}};
        spec.defaults = {{"target", "pets"}, {"index", "0"}, {"value", "1"}};
        add("set_index", spec);
    }
    {
        BlockSpec spec;
        spec.title = "binga";
        spec.category = "Choices";
        spec.fields = {{"cond", "check", "expr", 190}};
        spec.container = "if";
        spec.defaults = {{"cond", "gronk"}};
        add("if", spec);
    }
    {
        BlockSpec spec;
        spec.title = "booga";
        spec.category = "Loops";
        spec.fields = {{"count", "times", "expr", 56}};
        spec.container = "body";
        spec.defaults = {{"count", "3"}};
        add("repeat", spec);
    }
    {
        BlockSpec spec;
        spec.title = "zug";
        spec.category = "Loops";
        spec.fields = {{"cond", "while", "expr", 180}};
        spec.container = "body";
        spec.defaults = {{"cond", "gronk"}};
        add("while", spec);
    }
    {
        BlockSpec spec;
        spec.title = "zoop each";
        spec.category = "Loops";
        spec.fields = {{"name", "box", "name", 80}, {"iterable", "in", "expr", 150}};
        spec.container = "body";
        spec.defaults = {{"name", "thing"}, {"iterable", "pets"}};
        add("foreach", spec);
    }
    {
        BlockSpec spec;
        spec.title = "clump";
        spec.category = "Clumps";
        spec.fields = {{"name", "name", "name", 90}, {"params", "boxes", "params", 130}};
        spec.container = "body";
        spec.defaults = {{"name", "myword"}, {"params", ""}};
        add("clump", spec);
    }
    {
        BlockSpec spec;
        spec.title = "give back";
        spec.category = "Clumps";
        spec.fields = {{"expr", "answer", "expr", 150}};
        spec.defaults = {{"expr", ""}};
        add("ork", spec);
    }
    {
        BlockSpec spec;
        spec.title = "call clump";
        spec.category = "Clumps";
        spec.fields = {{"name", "name", "name", 90}, {"args", "with", "args", 150}};
        spec.defaults = {{"name", "myword"}, {"args", ""}};
        add("call", spec);
    }
    {
        BlockSpec spec;
        spec.title = "make pile";
        spec.category = "Piles";
        spec.fields = {{"name", "name", "name", 80}, {"items", "things", "args", 150}};
        spec.defaults = {{"name", "pets"}, {"items", "\"dog\", \"cat\""}};
        add("pile_make", spec);
    }
    {
        BlockSpec spec;
        spec.title = "plop into pile";
        spec.category = "Piles";
        spec.fields = {{"pile", "pile", "expr", 70}, {"item", "thing", "expr", 130}};
        spec.defaults = {{"pile", "pets"}, {"item", "\"fish\""}};
        add("pile_plop", spec);
    }
    {
        BlockSpec spec;
        spec.title = "yoink from pile";
        spec.category = "Piles";
        spec.fields = {{"name", "into", "name", 80}, {"pile", "pile", "expr", 110}};
        spec.defaults = {{"name", "taken"}, {"pile", "pets"}};
        add("pile_yoink", spec);
    }
    {
        BlockSpec spec;
        spec.title = "number";
        spec.category = "Values";
        spec.value = true;
        spec.fields = {{"value", "", "number", 60}};
        spec.defaults = {{"value", "1"}};
        add("value_number", spec);
    }
    {
        BlockSpec spec;
        spec.title = "text";
        spec.category = "Values";
        spec.value = true;
        spec.fields = {{"value", "", "text", 130}};
        spec.defaults = {{"value", "hello"}};
        add("value_text", spec);
    }
    {
        BlockSpec spec;
        spec.title = "box value";
        spec.category = "Values";
        spec.value = true;
        spec.fields = {{"name", "", "name", 90}};
        spec.defaults = {{"name", "x"}};
        add("value_var", spec);
    }
    {
        BlockSpec spec;
        spec.title = "random";
        spec.category = "Values";
        spec.value = true;
        spec.fields = {{"low", "", "expr", 46}, {"high", "", "expr", 46}};
        spec.defaults = {{"low", "1"}, {"high", "6"}};
        add("value_random", spec);
    }

    add("skrib_clear", make_skrib("clear", {}, {}));
    add("skrib_size", make_skrib("size", {"w", "h"}, {{"w", "400"}, {"h", "400"}}));
    add("skrib_color", make_skrib("color", {"color"}, {{"color", "\"red\""}}));
    add("skrib_dot", make_skrib("dot", {"x", "y", "r"}, {{"x", "100"}, {"y", "100"}, {"r", "10"}}));
    add("skrib_circle",
        make_skrib("circle", {"x", "y", "r"}, {{"x", "100"}, {"y", "100"}, {"r", "30"}}));
    add("skrib_line", make_skrib("line", {"x1", "y1", "x2", "y2"},
                                 {{"x1", "0"}, {"y1", "0"}, {"x2", "200"}, {"y2", "200"}}));
    add("skrib_box", make_skrib("box", {"x", "y", "w", "h"},
                                {{"x", "10"}, {"y", "10"}, {"w", "50"}, {"h", "50"}}));
    add("skrib_blob", make_skrib("blob", {"x", "y", "w", "h"},
                                 {{"x", "10"}, {"y", "10"}, {"w", "50"}, {"h", "50"}}));
    add("skrib_write", make_skrib("write", {"words", "x", "y"},
                                  {{"words", "\"hi\""}, {"x", "10"}, {"y", "10"}}));

    return table;
}

const std::set<std::string>& forbidden_names() {
    static const std::set<std::string> table = [] {
        std::set<std::string> names;
        for (const auto& name : cmc::words::builtin_names()) names.insert(name);
        names.insert("gronk");
        names.insert("nork");
        names.insert("plop");
        names.insert("pi");
        return names;
    }();
    return table;
}

std::string lower_copy(const std::string& text) {
    std::string out = text;
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return out;
}

std::string join_strings(const std::vector<std::string>& items, const std::string& sep) {
    std::string out;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) out += sep;
        out += items[i];
    }
    return out;
}

std::string line_of(const BlockPtr& block) {
    const std::string& kind = block->kind;
    if (kind == "say") return "oga " + block->field("expr");
    if (kind == "ask") {
        std::string prompt = block->field("prompt");
        std::string trimmed = prompt;
        std::size_t start = trimmed.find_first_not_of(" \t\r\n");
        std::size_t end = trimmed.find_last_not_of(" \t\r\n");
        trimmed = (start == std::string::npos) ? "" : trimmed.substr(start, end - start + 1);
        if (!trimmed.empty()) return "grunk " + block->field("name") + " = blorp " + prompt;
        return "grunk " + block->field("name") + " = blorp()";
    }
    if (kind == "grunk") return "grunk " + block->field("name") + " = " + block->field("expr");
    if (kind == "set") return block->field("name") + " = " + block->field("expr");
    if (kind == "set_index") {
        return block->field("target") + "[" + block->field("index") + "] = " + block->field("value");
    }
    if (kind == "repeat") return "booga " + block->field("count");
    if (kind == "while") return "zug " + block->field("cond");
    if (kind == "foreach") {
        return "zoop " + block->field("name") + " in " + block->field("iterable");
    }
    if (kind == "clump") {
        return "clump " + block->field("name") + "(" + block->field("params") + ")";
    }
    if (kind == "ork") {
        std::string answer = block->field("expr");
        std::size_t start = answer.find_first_not_of(" \t\r\n");
        std::size_t end = answer.find_last_not_of(" \t\r\n");
        answer = (start == std::string::npos) ? "" : answer.substr(start, end - start + 1);
        return "ork" + (answer.empty() ? "" : " " + answer);
    }
    if (kind == "call") return block->field("name") + "(" + block->field("args") + ")";
    if (kind == "pile_make") {
        return "grunk " + block->field("name") + " = snorf(" + block->field("items") + ")";
    }
    if (kind == "pile_plop") {
        return "plop(" + block->field("pile") + ", " + block->field("item") + ")";
    }
    if (kind == "pile_yoink") {
        return "grunk " + block->field("name") + " = yoink(" + block->field("pile") + ")";
    }
    if (kind.compare(0, 6, "skrib_") == 0) {
        std::string shape = kind.substr(6);
        std::vector<std::string> args;
        for (const auto& field : specs().at(kind).fields) {
            std::string value = block->field(field.key);
            std::size_t start = value.find_first_not_of(" \t\r\n");
            std::size_t end = value.find_last_not_of(" \t\r\n");
            args.push_back(start == std::string::npos ? "" : value.substr(start, end - start + 1));
        }
        if (shape == "clear" || args.empty()) {
            return shape == "clear" ? "skrib clear" : "skrib " + shape;
        }
        return "skrib " + shape + " " + join_strings(args, ", ");
    }
    throw BlockError("I do not know how to write the block '" + kind + "' yet.");
}

void block_lines(const std::vector<BlockPtr>& blocks, int indent, std::vector<std::string>& lines);

void if_lines(const BlockPtr& block, int indent, bool chained, std::vector<std::string>& lines) {
    std::string pad(static_cast<std::size_t>(indent) * 4, ' ');
    lines.push_back(pad + (chained ? "wonga binga " : "binga ") + block->field("cond"));
    block_lines(block->body, indent + 1, lines);
    const auto& tail = block->else_body;
    if (tail.size() == 1 && tail[0]->kind == "if") {
        if_lines(tail[0], indent, true, lines);
    } else if (!tail.empty()) {
        lines.push_back(pad + "wonga");
        block_lines(tail, indent + 1, lines);
    }
    if (!chained) lines.push_back(pad + "unga");
}

void block_lines(const std::vector<BlockPtr>& blocks, int indent, std::vector<std::string>& lines) {
    for (const auto& block : blocks) {
        std::string pad(static_cast<std::size_t>(indent) * 4, ' ');
        if (block->kind == "if") {
            if_lines(block, indent, false, lines);
            continue;
        }
        if (block->kind == "repeat" || block->kind == "while" || block->kind == "foreach"
            || block->kind == "clump") {
            lines.push_back(pad + line_of(block));
            block_lines(block->body, indent + 1, lines);
            lines.push_back(pad + "unga");
            continue;
        }
        if (block->is_value()) {
            throw BlockError("A value block like 'number' cannot sit on its own. Put it inside a slot.");
        }
        lines.push_back(pad + line_of(block));
    }
}

void check_name(const BlockPtr& block, const std::string& key, const std::string& label,
                std::vector<std::string>& problems) {
    std::string name = block->field(key);
    std::size_t start = name.find_first_not_of(" \t\r\n");
    std::size_t end = name.find_last_not_of(" \t\r\n");
    name = (start == std::string::npos) ? "" : name.substr(start, end - start + 1);
    if (name.empty()) {
        problems.push_back("A " + block->title() + " block is missing its " + label + ".");
    } else if (!is_good_name(name)) {
        problems.push_back("'" + name + "' cannot be a " + label + " (use letters, numbers, _).");
    } else if (forbidden_names().count(lower_copy(name))) {
        problems.push_back("'" + name + "' already means something in CMC.");
    } else if (name == "lap") {
        problems.push_back("'lap' is the magic repeat box. Pick another name.");
    }
}

BlockPtr statement_to_block(const cmc::Stmt* node);

BlockPtr skrib_to_block(const Skrib* node) {
    std::string kind = "skrib_" + node->shape;
    if (!specs().count(kind)) {
        throw BlockError("skrib " + node->shape + " has no block yet.");
    }
    BlockPtr block = new_block(kind);
    const auto& fields = specs().at(kind).fields;
    for (std::size_t i = 0; i < fields.size() && i < node->args.size(); ++i) {
        block->fields[fields[i].key] = cmc::expr_to_cmc(node->args[i].get());
    }
    return block;
}

BlockPtr statement_to_block(const cmc::Stmt* node) {
    if (auto* say = dynamic_cast<const Say*>(node)) {
        BlockPtr block = new_block("say");
        std::vector<std::string> parts;
        for (const auto& expr : say->exprs) parts.push_back(cmc::expr_to_cmc(expr.get()));
        block->fields["expr"] = join_strings(parts, ", ");
        return block;
    }
    if (auto* assign = dynamic_cast<const Assign*>(node)) {
        if (dynamic_cast<const Ask*>(assign->expr.get()) != nullptr) {
            if (!assign->declare) {
                throw BlockError("A blorp question must be put into a new box with 'grunk'.");
            }
            BlockPtr block = new_block("ask");
            block->fields["name"] = assign->name;
            const auto* ask = dynamic_cast<const Ask*>(assign->expr.get());
            block->fields["prompt"] = ask->prompt ? cmc::expr_to_cmc(ask->prompt.get()) : "";
            return block;
        }
        BlockPtr block = new_block(assign->declare ? "grunk" : "set");
        block->fields["name"] = assign->name;
        block->fields["expr"] = cmc::expr_to_cmc(assign->expr.get());
        return block;
    }
    if (auto* set_index = dynamic_cast<const SetIndex*>(node)) {
        BlockPtr block = new_block("set_index");
        block->fields["target"] = cmc::expr_to_cmc(set_index->target.get());
        block->fields["index"] = cmc::expr_to_cmc(set_index->index.get());
        block->fields["value"] = cmc::expr_to_cmc(set_index->value.get());
        return block;
    }
    if (auto* if_node = dynamic_cast<const If*>(node)) {
        BlockPtr block = new_block("if");
        block->fields["cond"] = cmc::expr_to_cmc(if_node->cond.get());
        for (const auto& stmt : if_node->body) block->body.push_back(statement_to_block(stmt.get()));
        for (const auto& stmt : if_node->else_body) {
            block->else_body.push_back(statement_to_block(stmt.get()));
        }
        return block;
    }
    if (auto* repeat = dynamic_cast<const Repeat*>(node)) {
        BlockPtr block = new_block("repeat");
        block->fields["count"] = cmc::expr_to_cmc(repeat->count.get());
        for (const auto& stmt : repeat->body) block->body.push_back(statement_to_block(stmt.get()));
        return block;
    }
    if (auto* while_node = dynamic_cast<const While*>(node)) {
        BlockPtr block = new_block("while");
        block->fields["cond"] = cmc::expr_to_cmc(while_node->cond.get());
        for (const auto& stmt : while_node->body) block->body.push_back(statement_to_block(stmt.get()));
        return block;
    }
    if (auto* foreach = dynamic_cast<const ForEach*>(node)) {
        BlockPtr block = new_block("foreach");
        block->fields["name"] = foreach->name;
        block->fields["iterable"] = cmc::expr_to_cmc(foreach->iterable.get());
        for (const auto& stmt : foreach->body) block->body.push_back(statement_to_block(stmt.get()));
        return block;
    }
    if (auto* func = dynamic_cast<const FuncDef*>(node)) {
        BlockPtr block = new_block("clump");
        block->fields["name"] = func->name;
        block->fields["params"] = join_strings(func->params, ", ");
        for (const auto& stmt : func->body) block->body.push_back(statement_to_block(stmt.get()));
        return block;
    }
    if (auto* ret = dynamic_cast<const Return*>(node)) {
        BlockPtr block = new_block("ork");
        block->fields["expr"] = ret->expr ? cmc::expr_to_cmc(ret->expr.get()) : "";
        return block;
    }
    if (auto* stmt = dynamic_cast<const ExprStmt*>(node)) {
        const auto* call = dynamic_cast<const Call*>(stmt->expr.get());
        if (call == nullptr) {
            throw BlockError("That line cannot become a block yet.");
        }
        if (call->name == "plop" && call->args.size() == 2) {
            BlockPtr block = new_block("pile_plop");
            block->fields["pile"] = cmc::expr_to_cmc(call->args[0].get());
            block->fields["item"] = cmc::expr_to_cmc(call->args[1].get());
            return block;
        }
        BlockPtr block = new_block("call");
        block->fields["name"] = call->name;
        std::vector<std::string> parts;
        for (const auto& arg : call->args) parts.push_back(cmc::expr_to_cmc(arg.get()));
        block->fields["args"] = join_strings(parts, ", ");
        return block;
    }
    if (auto* skrib = dynamic_cast<const Skrib*>(node)) {
        return skrib_to_block(skrib);
    }
    throw BlockError("That line cannot become a block yet: " + std::string(typeid(*node).name()));
}

}

const std::unordered_map<std::string, BlockSpec>& specs() {
    static const std::unordered_map<std::string, BlockSpec> table = build_specs();
    return table;
}

const std::vector<std::string>& spec_order() {
    static const std::vector<std::string> table = {
        "say",         "ask",          "grunk",       "set",          "set_index",
        "if",          "repeat",       "while",       "foreach",      "clump",
        "ork",         "call",         "pile_make",   "pile_plop",    "pile_yoink",
        "value_number", "value_text",  "value_var",   "value_random", "skrib_clear",
        "skrib_size",  "skrib_color",  "skrib_dot",   "skrib_circle", "skrib_line",
        "skrib_box",   "skrib_blob",   "skrib_write",
    };
    return table;
}

const std::vector<std::string>& category_order() {
    static const std::vector<std::string> table = {"Talking", "Boxes",  "Choices", "Loops",
                                                   "Clumps",  "Values", "Piles",   "Drawing"};
    return table;
}

std::string Block::field(const std::string& key) const {
    auto it = fields.find(key);
    return it == fields.end() ? "" : it->second;
}

bool Block::is_value() const {
    auto it = specs().find(kind);
    return it != specs().end() && it->second.value;
}

std::string Block::title() const {
    auto it = specs().find(kind);
    return it == specs().end() ? kind : it->second.title;
}

BlockPtr new_block(const std::string& kind) {
    auto spec = specs().find(kind);
    if (spec == specs().end()) throw BlockError("Unknown block kind: " + kind);
    BlockPtr block = std::make_shared<Block>();
    block->kind = kind;
    block->uid = next_uid();
    for (const auto& field : spec->second.fields) {
        auto def = spec->second.defaults.find(field.key);
        block->fields[field.key] = def == spec->second.defaults.end() ? "" : def->second;
    }
    return block;
}

BlockPtr clone_block(const BlockPtr& block) {
    BlockPtr copied = std::make_shared<Block>();
    copied->kind = block->kind;
    copied->fields = block->fields;
    copied->uid = next_uid();
    for (const auto& child : block->body) copied->body.push_back(clone_block(child));
    for (const auto& child : block->else_body) copied->else_body.push_back(clone_block(child));
    return copied;
}

std::vector<BlockPtr> clone_all(const std::vector<BlockPtr>& blocks) {
    std::vector<BlockPtr> out;
    out.reserve(blocks.size());
    for (const auto& block : blocks) out.push_back(clone_block(block));
    return out;
}

void all_blocks(const std::vector<BlockPtr>& blocks,
                const std::function<void(const BlockPtr&)>& visit) {
    for (const auto& block : blocks) {
        visit(block);
        all_blocks(block->body, visit);
        all_blocks(block->else_body, visit);
    }
}

std::string value_expression(const BlockPtr& block) {
    if (block->kind == "value_number") {
        std::string value = block->field("value");
        std::size_t start = value.find_first_not_of(" \t\r\n");
        std::size_t end = value.find_last_not_of(" \t\r\n");
        value = (start == std::string::npos) ? "" : value.substr(start, end - start + 1);
        return value.empty() ? "0" : value;
    }
    if (block->kind == "value_text") {
        std::string text = block->field("value");
        std::size_t start = text.find_first_not_of(" \t\r\n");
        std::size_t end = text.find_last_not_of(" \t\r\n");
        std::string stripped = (start == std::string::npos) ? "" : text.substr(start, end - start + 1);
        if (stripped.size() >= 2 && (stripped[0] == '"' || stripped[0] == '\'')
            && stripped.back() == stripped[0]) {
            return stripped;
        }
        std::string escaped;
        for (char c : text) {
            if (c == '\\') escaped += "\\\\";
            else if (c == '"') escaped += "\\\"";
            else escaped += c;
        }
        return "\"" + escaped + "\"";
    }
    if (block->kind == "value_var") {
        std::string name = block->field("name");
        std::size_t start = name.find_first_not_of(" \t\r\n");
        std::size_t end = name.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : name.substr(start, end - start + 1);
    }
    if (block->kind == "value_random") {
        auto trim = [](std::string text) {
            std::size_t start = text.find_first_not_of(" \t\r\n");
            std::size_t end = text.find_last_not_of(" \t\r\n");
            return (start == std::string::npos) ? std::string() : text.substr(start, end - start + 1);
        };
        std::string low = trim(block->field("low"));
        std::string high = trim(block->field("high"));
        return "munga(" + (low.empty() ? "1" : low) + ", " + (high.empty() ? "6" : high) + ")";
    }
    return "";
}

std::string blocks_to_source(const std::vector<BlockPtr>& blocks) {
    std::vector<std::string> lines;
    block_lines(blocks, 0, lines);
    return lines.empty() ? "" : join_strings(lines, "\n") + "\n";
}

std::vector<std::string> check_blocks(const std::vector<BlockPtr>& blocks) {
    std::vector<std::string> problems;
    all_blocks(blocks, [&problems](const BlockPtr& block) {
        const std::string& kind = block->kind;
        if (kind == "grunk" || kind == "set" || kind == "ask" || kind == "foreach"
            || kind == "pile_make" || kind == "pile_yoink") {
            check_name(block, "name", "box name", problems);
        }
        if (kind == "clump") {
            check_name(block, "name", "clump name", problems);
            std::string params = block->field("params");
            std::size_t start = params.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) {
                std::size_t pos = 0;
                while (pos <= params.size()) {
                    std::size_t comma = params.find(',', pos);
                    std::string piece = params.substr(pos, comma == std::string::npos
                                                               ? std::string::npos
                                                               : comma - pos);
                    std::size_t ps = piece.find_first_not_of(" \t\r\n");
                    std::size_t pe = piece.find_last_not_of(" \t\r\n");
                    piece = (ps == std::string::npos) ? "" : piece.substr(ps, pe - ps + 1);
                    if (!piece.empty()
                        && (!is_good_name(piece) || forbidden_names().count(lower_copy(piece)))) {
                        problems.push_back("'" + piece
                                           + "' is not a good box name in clump boxes.");
                    }
                    if (comma == std::string::npos) break;
                    pos = comma + 1;
                }
            }
        }
        if (kind == "call") {
            check_name(block, "name", "clump name", problems);
        }
        if (kind == "set_index") {
            std::string target = block->field("target");
            if (target.find_first_not_of(" \t\r\n") == std::string::npos) {
                problems.push_back("A 'change a spot' block needs a pile name.");
            }
        }
    });
    return problems;
}

std::string describe_problems(const std::vector<std::string>& problems) {
    std::vector<std::string> lines;
    for (const auto& problem : problems) lines.push_back("  - " + problem);
    return join_strings(lines, "\n");
}

bool is_good_name(const std::string& name) {
    if (name.empty()) return false;
    char first = name[0];
    if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z') || first == '_')) {
        return false;
    }
    for (char c : name) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
              || c == '_')) {
            return false;
        }
    }
    return true;
}

bool is_forbidden_block_name(const std::string& name) {
    return forbidden_names().count(lower_copy(name)) > 0;
}

std::string validate_source(const std::string& source) {
    try {
        cmc::parse_source(source);
    } catch (cmc::CmcError& error) {
        return error.format();
    }
    return "";
}

std::vector<BlockPtr> source_to_blocks(const std::string& source) {
    std::unique_ptr<cmc::Program> program;
    try {
        program = cmc::parse_source(source);
    } catch (cmc::CmcError& error) {
        throw BlockError("The text has a mistake first:\n" + error.format());
    }
    std::vector<BlockPtr> blocks;
    for (const auto& statement : program->statements) {
        if (auto* assign = dynamic_cast<const Assign*>(statement.get())) {
            if (auto* call = dynamic_cast<const Call*>(assign->expr.get())) {
                if (call->name == "snorf" && assign->declare) {
                    BlockPtr block = new_block("pile_make");
                    block->fields["name"] = assign->name;
                    std::vector<std::string> parts;
                    for (const auto& arg : call->args) parts.push_back(cmc::expr_to_cmc(arg.get()));
                    block->fields["items"] = join_strings(parts, ", ");
                    blocks.push_back(block);
                    continue;
                }
                if (call->name == "yoink" && assign->declare && call->args.size() == 1) {
                    BlockPtr block = new_block("pile_yoink");
                    block->fields["name"] = assign->name;
                    block->fields["pile"] = cmc::expr_to_cmc(call->args[0].get());
                    blocks.push_back(block);
                    continue;
                }
            }
        }
        blocks.push_back(statement_to_block(statement.get()));
    }
    return blocks;
}

}