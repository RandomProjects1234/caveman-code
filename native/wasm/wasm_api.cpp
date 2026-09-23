#include <emscripten.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "codegen.hpp"
#include "errors.hpp"
#include "ide_blocks.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "words.hpp"
#include "words_data.hpp"

using nlohmann::json;

extern "C" {
void cmc_js_emit(const char* line);
void cmc_js_draw(const char* op, double a, double b, double c, double d, const char* text);
int cmc_js_should_stop(void);
void cmc_js_ask_async(const char* prompt, char* buffer, int capacity);
void cmc_js_done(void);
}

namespace {

std::string& last_result() {
    static std::string text;
    return text;
}

char* to_c_string(const std::string& text) {
    char* out = static_cast<char*>(std::malloc(text.size() + 1));
    if (out == nullptr) return nullptr;
    std::memcpy(out, text.c_str(), text.size() + 1);
    return out;
}

class WebDraw : public cmc::DrawSurface {
public:
    void clear() override { cmc_js_draw("clear", 0, 0, 0, 0, ""); }
    void size(int w, int h) override { cmc_js_draw("size", w, h, 0, 0, ""); }
    void color(const std::string& c) override { cmc_js_draw("color", 0, 0, 0, 0, c.c_str()); }
    void dot(double x, double y, double r) override { cmc_js_draw("dot", x, y, r, 0, ""); }
    void circle(double x, double y, double r) override { cmc_js_draw("circle", x, y, r, 0, ""); }
    void line(double x1, double y1, double x2, double y2) override {
        cmc_js_draw("line", x1, y1, x2, y2, "");
    }
    void box(double x, double y, double w, double h) override {
        cmc_js_draw("box", x, y, w, h, "");
    }
    void blob(double x, double y, double w, double h) override {
        cmc_js_draw("blob", x, y, w, h, "");
    }
    void write(const std::string& text, double x, double y) override {
        cmc_js_draw("write", x, y, 0, 0, text.c_str());
    }
};

json block_to_json(const og::BlockPtr& block) {
    json out;
    out["kind"] = block->kind;
    out["fields"] = block->fields;
    out["body"] = json::array();
    for (const auto& child : block->body) out["body"].push_back(block_to_json(child));
    out["elseBody"] = json::array();
    for (const auto& child : block->else_body) out["elseBody"].push_back(block_to_json(child));
    return out;
}

og::BlockPtr block_from_json(const json& data) {
    std::string kind = data.value("kind", "say");
    og::BlockPtr block = og::new_block(kind);
    if (data.contains("fields") && data["fields"].is_object()) {
        for (auto it = data["fields"].begin(); it != data["fields"].end(); ++it) {
            if (it.value().is_string()) block->fields[it.key()] = it.value().get<std::string>();
        }
    }
    if (data.contains("body") && data["body"].is_array()) {
        for (const auto& child : data["body"]) block->body.push_back(block_from_json(child));
    }
    if (data.contains("elseBody") && data["elseBody"].is_array()) {
        for (const auto& child : data["elseBody"]) {
            block->else_body.push_back(block_from_json(child));
        }
    }
    return block;
}

std::vector<og::BlockPtr> blocks_from_json(const json& data) {
    std::vector<og::BlockPtr> blocks;
    if (!data.is_array()) return blocks;
    for (const auto& item : data) blocks.push_back(block_from_json(item));
    return blocks;
}

json specs_to_json() {
    json out;
    out["order"] = og::spec_order();
    out["categories"] = og::category_order();
    out["specs"] = json::object();
    for (const auto& entry : og::specs()) {
        const og::BlockSpec& spec = entry.second;
        json item;
        item["title"] = spec.title;
        item["category"] = spec.category;
        item["container"] = spec.container;
        item["value"] = spec.value;
        item["defaults"] = spec.defaults;
        item["fields"] = json::array();
        for (const auto& field : spec.fields) {
            json field_json;
            field_json["key"] = field.key;
            field_json["label"] = field.label;
            field_json["type"] = field.type;
            field_json["width"] = field.width;
            item["fields"].push_back(field_json);
        }
        out["specs"][entry.first] = item;
    }
    return out;
}

json word_to_json(const cmc::words::WordInfo& word) {
    json item;
    item["name"] = word.name;
    item["display"] = word.display;
    item["category"] = word.category;
    item["summary"] = word.summary;
    item["syntax"] = word.syntax;
    item["doc"] = word.doc;
    item["example"] = word.example;
    item["aliases"] = word.aliases;
    item["extraSyntax"] = word.extra_syntax;
    item["tips"] = word.tips;
    item["related"] = word.related;
    return item;
}

}

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* cmc_version() {
    return to_c_string(cmc::words::VERSION);
}

EMSCRIPTEN_KEEPALIVE
void cmc_run(const char* source, const char* filename) {
    json result;
    result["ok"] = true;
    result["output"] = json::array();
    result["stopped"] = false;
    result["error"] = "";
    std::vector<std::string> lines;
    WebDraw draw;
    try {
        cmc::Interpreter interpreter;
        interpreter.set_output([&lines](const std::string& line) {
            lines.push_back(line);
            cmc_js_emit(line.c_str());
        });
        interpreter.set_draw(&draw);
        interpreter.set_should_stop([] { return cmc_js_should_stop() != 0; });
        interpreter.run(source != nullptr ? source : "", filename != nullptr ? filename : "<web>");
    } catch (cmc::CmcStopped&) {
        result["ok"] = false;
        result["stopped"] = true;
    } catch (cmc::CmcError& error) {
        result["ok"] = false;
        result["error"] = error.format();
    } catch (std::exception& error) {
        result["ok"] = false;
        result["error"] = std::string("CMC had a bug: ") + error.what();
    }
    result["output"] = lines;
    last_result() = result.dump();
    cmc_js_done();
}

EMSCRIPTEN_KEEPALIVE
void cmc_compile(const char* source, const char* filename) {
    json result;
    result["ok"] = true;
    result["python"] = "";
    result["error"] = "";
    try {
        result["python"] = cmc::to_python_source(source != nullptr ? source : "",
                                                 filename != nullptr ? filename : "<web>");
    } catch (cmc::CmcError& error) {
        result["ok"] = false;
        result["error"] = error.format();
    }
    last_result() = result.dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_check(const char* source) {
    json result;
    result["ok"] = true;
    result["error"] = "";
    try {
        cmc::parse_source(source != nullptr ? source : "");
    } catch (cmc::CmcError& error) {
        result["ok"] = false;
        result["error"] = error.format();
    }
    last_result() = result.dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_blocks_specs() {
    last_result() = specs_to_json().dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_blocks_from_source(const char* source) {
    json result;
    result["ok"] = true;
    result["error"] = "";
    result["blocks"] = json::array();
    try {
        std::vector<og::BlockPtr> blocks = og::source_to_blocks(source != nullptr ? source : "");
        for (const auto& block : blocks) result["blocks"].push_back(block_to_json(block));
    } catch (og::BlockError& error) {
        result["ok"] = false;
        result["error"] = error.message();
    }
    last_result() = result.dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_blocks_to_source(const char* blocks_json) {
    json result;
    result["ok"] = true;
    result["error"] = "";
    result["source"] = "";
    try {
        json data = json::parse(blocks_json != nullptr ? blocks_json : "[]");
        result["source"] = og::blocks_to_source(blocks_from_json(data));
    } catch (og::BlockError& error) {
        result["ok"] = false;
        result["error"] = error.message();
    } catch (std::exception& error) {
        result["ok"] = false;
        result["error"] = error.what();
    }
    last_result() = result.dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_blocks_check(const char* blocks_json) {
    json result;
    result["ok"] = true;
    result["problems"] = json::array();
    try {
        json data = json::parse(blocks_json != nullptr ? blocks_json : "[]");
        for (const auto& problem : og::check_blocks(blocks_from_json(data))) {
            result["problems"].push_back(problem);
        }
    } catch (std::exception& error) {
        result["ok"] = false;
        result["problems"].push_back(error.what());
    }
    last_result() = result.dump();
}

EMSCRIPTEN_KEEPALIVE
void cmc_dictionary() {
    json out = json::array();
    for (const auto& word : cmc::words::word_infos()) out.push_back(word_to_json(word));
    last_result() = out.dump();
}

EMSCRIPTEN_KEEPALIVE
const char* cmc_last() {
    return last_result().c_str();
}

EMSCRIPTEN_KEEPALIVE
void cmc_free(const char* text) {
    std::free(const_cast<char*>(text));
}

}
