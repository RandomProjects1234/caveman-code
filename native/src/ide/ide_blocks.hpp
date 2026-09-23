#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace og {

struct FieldSpec {
    std::string key;
    std::string label;
    std::string type;
    int width = 100;
};

struct BlockSpec {
    std::string title;
    std::string category;
    std::string container;
    bool value = false;
    std::vector<FieldSpec> fields;
    std::map<std::string, std::string> defaults;
};

const std::unordered_map<std::string, BlockSpec>& specs();
const std::vector<std::string>& spec_order();
const std::vector<std::string>& category_order();

struct Block {
    std::string kind;
    std::map<std::string, std::string> fields;
    std::vector<std::shared_ptr<Block>> body;
    std::vector<std::shared_ptr<Block>> else_body;
    long long uid = 0;

    std::string field(const std::string& key) const;
    bool is_value() const;
    std::string title() const;
};

using BlockPtr = std::shared_ptr<Block>;

class BlockError : public std::exception {
public:
    explicit BlockError(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }
    const std::string& message() const { return message_; }

private:
    std::string message_;
};

BlockPtr new_block(const std::string& kind);
BlockPtr clone_block(const BlockPtr& block);
std::vector<BlockPtr> clone_all(const std::vector<BlockPtr>& blocks);
void all_blocks(const std::vector<BlockPtr>& blocks,
                const std::function<void(const BlockPtr&)>& visit);
std::string value_expression(const BlockPtr& block);
std::string blocks_to_source(const std::vector<BlockPtr>& blocks);
std::vector<std::string> check_blocks(const std::vector<BlockPtr>& blocks);
std::string describe_problems(const std::vector<std::string>& problems);
std::string validate_source(const std::string& source);
std::vector<BlockPtr> source_to_blocks(const std::string& source);
bool is_good_name(const std::string& name);
bool is_forbidden_block_name(const std::string& name);

}