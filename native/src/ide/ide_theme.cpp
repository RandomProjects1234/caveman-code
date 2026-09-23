#include "ide_theme.hpp"

namespace og {
namespace theme {

namespace {

struct BlockColorEntry {
    const char* kind;
    COLORREF color;
};

const BlockColorEntry BLOCK_COLORS[] = {
    {"say", RGB(0xff, 0xb3, 0x40)},
    {"ask", RGB(0xff, 0xd1, 0x66)},
    {"grunk", RGB(0x64, 0xb5, 0xf6)},
    {"set", RGB(0x4a, 0xa3, 0xdf)},
    {"if", RGB(0xc7, 0x92, 0xea)},
    {"repeat", RGB(0x4c, 0xc3, 0x8a)},
    {"while", RGB(0x35, 0xb0, 0x7a)},
    {"foreach", RGB(0x2f, 0x9e, 0x6e)},
    {"clump", RGB(0xf3, 0x8b, 0xa8)},
    {"ork", RGB(0xe0, 0x6c, 0x8c)},
    {"call", RGB(0xf2, 0xa2, 0xb8)},
    {"set_index", RGB(0x5a, 0xa9, 0xd6)},
    {"pile_make", RGB(0x8f, 0xd6, 0x94)},
    {"pile_plop", RGB(0x6f, 0xc4, 0x7a)},
    {"pile_yoink", RGB(0x54, 0xae, 0x66)},
    {"value_number", RGB(0xd0, 0xc4, 0xe8)},
    {"value_text", RGB(0xd0, 0xc4, 0xe8)},
    {"value_var", RGB(0xd0, 0xc4, 0xe8)},
    {"value_random", RGB(0xd0, 0xc4, 0xe8)},
    {"skrib", RGB(0xf9, 0xc7, 0x4f)},
};

const BlockColorEntry CATEGORY_COLORS[] = {
    {"Talking", RGB(0xff, 0xb3, 0x40)},
    {"Boxes", RGB(0x64, 0xb5, 0xf6)},
    {"Choices", RGB(0xc7, 0x92, 0xea)},
    {"Loops", RGB(0x4c, 0xc3, 0x8a)},
    {"Clumps", RGB(0xf3, 0x8b, 0xa8)},
    {"Values", RGB(0xd0, 0xc4, 0xe8)},
    {"Piles", RGB(0x8f, 0xd6, 0x94)},
    {"Drawing", RGB(0xf9, 0xc7, 0x4f)},
};

}

COLORREF block_color(const std::string& kind) {
    if (kind.compare(0, 6, "skrib_") == 0) {
        return RGB(0xf9, 0xc7, 0x4f);
    }
    for (const auto& entry : BLOCK_COLORS) {
        if (kind == entry.kind) return entry.color;
    }
    return DEFAULT_BLOCK;
}

COLORREF category_color(const std::string& category) {
    for (const auto& entry : CATEGORY_COLORS) {
        if (category == entry.kind) return entry.color;
    }
    return DIM;
}

}
}