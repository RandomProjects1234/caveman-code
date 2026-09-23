#pragma once

#include <windows.h>

#include <string>
#include <unordered_map>

namespace og {
namespace theme {

const COLORREF BG = RGB(0x24, 0x1a, 0x12);
const COLORREF PANEL = RGB(0x31, 0x24, 0x1a);
const COLORREF PANEL2 = RGB(0x3d, 0x2d, 0x1f);
const COLORREF PANEL3 = RGB(0x4a, 0x37, 0x28);
const COLORREF TEXT = RGB(0xf7, 0xea, 0xd9);
const COLORREF DIM = RGB(0xb3, 0x9b, 0x80);
const COLORREF ACCENT = RGB(0xff, 0xb3, 0x40);
const COLORREF GREEN = RGB(0x4c, 0xc3, 0x8a);
const COLORREF RED = RGB(0xff, 0x6b, 0x6b);
const COLORREF BLUE = RGB(0x64, 0xb5, 0xf6);
const COLORREF PURPLE = RGB(0xc7, 0x92, 0xea);
const COLORREF YELLOW = RGB(0xff, 0xd1, 0x66);
const COLORREF WHITE = RGB(0xff, 0xff, 0xff);
const COLORREF COMMENT = RGB(0x8a, 0x7a, 0x68);
const COLORREF FIELD_BG = RGB(0x24, 0x1a, 0x12);
const COLORREF FIELD_EDIT_BG = RGB(0x12, 0x0d, 0x08);
const COLORREF CONTAINER_BG = RGB(0x2b, 0x20, 0x16);
const COLORREF HINT_TEXT = RGB(0x6b, 0x59, 0x47);
const COLORREF BLOCK_TEXT = RGB(0x24, 0x1a, 0x12);
const COLORREF DEFAULT_BLOCK = RGB(0xb3, 0x9b, 0x80);

COLORREF block_color(const std::string& kind);
COLORREF category_color(const std::string& category);

inline int font_height(int point_size, int dpi = 96) {
    return -MulDiv(point_size, dpi, 72);
}

}
}