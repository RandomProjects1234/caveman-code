#include "ide_canvas.hpp"

#include <commctrl.h>
#include <windowsx.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "codegen.hpp"
#include "errors.hpp"
#include "ide_editor.hpp"
#include "ide_theme.hpp"
#include "parser.hpp"

namespace og {

namespace {

const wchar_t* CANVAS_CLASS = L"OgaboogaBlockCanvas";
const wchar_t* PALETTE_CLASS = L"OgaboogaPalette";
const UINT WM_FIELD_COMMIT = WM_APP + 20;
const UINT WM_FIELD_CANCEL = WM_APP + 21;

const int MIN_W = 200;
const int GAP = 10;
const int HEADER = 32;
const int FIELD_H = 24;
const int INDENT = 24;
const int BODY_PAD = 10;
const int BOTTOM_PAD = 10;
const int ELSE_BAR = 24;
const int DRAG_SLOP = 6;
const int PALETTE_ROW = 28;
const int PALETTE_HEADER = 26;

int text_width(const std::string& text, int per_char = 7) {
    return static_cast<int>(text.size()) * per_char;
}

}

BlockCanvas::BlockCanvas(HWND parent, HFONT ui_font, HFONT ui_bold_font, HFONT small_font,
                         HFONT code_font)
    : ui_font_(ui_font), ui_bold_font_(ui_bold_font), small_font_(small_font),
      code_font_(code_font) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW canvas_class;
        std::memset(&canvas_class, 0, sizeof(canvas_class));
        canvas_class.lpfnWndProc = BlockCanvas::canvas_proc;
        canvas_class.hInstance = GetModuleHandleW(nullptr);
        canvas_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        canvas_class.lpszClassName = CANVAS_CLASS;
        RegisterClassW(&canvas_class);

        WNDCLASSW palette_class;
        std::memset(&palette_class, 0, sizeof(palette_class));
        palette_class.lpfnWndProc = BlockCanvas::palette_proc;
        palette_class.hInstance = GetModuleHandleW(nullptr);
        palette_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        palette_class.lpszClassName = PALETTE_CLASS;
        RegisterClassW(&palette_class);
        registered = true;
    }

    palette_ = CreateWindowExW(0, PALETTE_CLASS, L"",
                               WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPCHILDREN, 0, 0,
                               100, 100, parent, nullptr, GetModuleHandleW(nullptr), this);
    hwnd_ = CreateWindowExW(0, CANVAS_CLASS, L"",
                            WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN,
                            0, 0, 100, 100, parent, nullptr, GetModuleHandleW(nullptr), this);
    rebuild_layout();
    update_scroll();

    for (const auto& category : category_order()) {
        std::vector<std::string> kinds;
        for (const auto& kind : spec_order()) {
            if (specs().at(kind).category == category) kinds.push_back(kind);
        }
        if (kinds.empty()) continue;
        PaletteItem header;
        header.is_header = true;
        header.category = category;
        palette_items_.push_back(header);
        for (const auto& kind : kinds) {
            PaletteItem item;
            item.kind = kind;
            item.title = specs().at(kind).title;
            item.category = category;
            palette_items_.push_back(item);
        }
    }
    update_palette_scroll();
}

BlockCanvas::~BlockCanvas() {
    if (field_edit_ != nullptr) DestroyWindow(field_edit_);
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
    if (palette_ != nullptr) DestroyWindow(palette_);
}

void BlockCanvas::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
    update_scroll();
}

void BlockCanvas::layout_palette(int x, int y, int w, int h) {
    if (palette_ != nullptr) MoveWindow(palette_, x, y, w, h, TRUE);
    update_palette_scroll();
}

void BlockCanvas::notify() {
    if (on_change_) on_change_();
}

void BlockCanvas::status(const std::string& message) {
    if (on_status_) on_status_(message);
}

void BlockCanvas::snapshot() {
    history_.push_back(clone_all(blocks_));
    if (history_.size() > 80) history_.erase(history_.begin());
}

void BlockCanvas::set_blocks(std::vector<BlockPtr> blocks, bool remember) {
    if (remember) snapshot();
    blocks_ = std::move(blocks);
    selected_ = nullptr;
    redraw();
    notify();
}

std::string BlockCanvas::source() const {
    return blocks_to_source(blocks_);
}

void BlockCanvas::clear_all() {
    if (blocks_.empty()) return;
    snapshot();
    blocks_.clear();
    selected_ = nullptr;
    redraw();
    notify();
}

void BlockCanvas::undo() {
    if (history_.empty()) {
        status("Nothing to undo.");
        return;
    }
    blocks_ = history_.back();
    history_.pop_back();
    selected_ = nullptr;
    redraw();
    notify();
    status("Undone!");
}

void BlockCanvas::add_block(const std::string& kind) {
    BlockPtr block = new_block(kind);
    snapshot();
    blocks_.push_back(block);
    selected_ = block;
    redraw();
    notify();
    status("Added " + block->title() + " block.");
}

void BlockCanvas::delete_selected() {
    if (!selected_) return;
    snapshot();
    delete_block_from_tree(selected_.get());
    selected_ = nullptr;
    redraw();
    notify();
}

void BlockCanvas::delete_block_from_tree(Block* block) {
    Block* parent = nullptr;
    std::string which;
    int index = 0;
    if (!find_location(block, &blocks_, nullptr, "body", &parent, &which, &index)) return;
    std::vector<BlockPtr>* list = nullptr;
    target_list(parent, which, &list);
    if (list != nullptr && index >= 0 && index < static_cast<int>(list->size())) {
        list->erase(list->begin() + index);
    }
}

bool BlockCanvas::find_location(Block* block, std::vector<BlockPtr>* blocks, Block* parent,
                                const std::string& which, Block** out_parent,
                                std::string* out_which, int* out_index) {
    for (std::size_t i = 0; i < blocks->size(); ++i) {
        Block* candidate = (*blocks)[i].get();
        if (candidate == block) {
            *out_parent = parent;
            *out_which = which;
            *out_index = static_cast<int>(i);
            return true;
        }
        if (find_location(block, &candidate->body, candidate, "body", out_parent, out_which,
                          out_index)) {
            return true;
        }
        if (find_location(block, &candidate->else_body, candidate, "else", out_parent, out_which,
                          out_index)) {
            return true;
        }
    }
    return false;
}

void BlockCanvas::target_list(Block* parent, const std::string& which,
                              std::vector<BlockPtr>** out_list) {
    if (parent == nullptr) {
        *out_list = &blocks_;
        return;
    }
    *out_list = (which == "body") ? &parent->body : &parent->else_body;
}

BlockCanvas::Entry* BlockCanvas::entry_for(const Block* block) const {
    auto it = entries_.find(block);
    return it == entries_.end() ? nullptr : it->second;
}

BlockCanvas::Entry* BlockCanvas::layout_block(const BlockPtr& block, int x, int y) {
    Entry* entry = new Entry();
    entry->block = block;
    layout_.emplace_back(entry);
    entries_[block.get()] = entry;

    const BlockSpec& spec = specs().at(block->kind);
    std::string title = block->title();
    int title_w = std::max(52, text_width(title) + 18);
    int cursor = x + 8 + title_w + 4;
    for (const auto& field : spec.fields) {
        if (!field.label.empty()) cursor += text_width(field.label) + 4;
        RECT rect{cursor, y + (HEADER - FIELD_H) / 2, cursor + field.width,
                  y + (HEADER - FIELD_H) / 2 + FIELD_H};
        entry->fields[field.key] = rect;
        cursor += field.width + 8;
    }
    int header_right = cursor + 6;
    int width = std::max(MIN_W, header_right - x);

    int bottom = y + HEADER;
    bool container = !spec.container.empty();

    if (container) {
        int body_x = x + INDENT;
        int inner_y = y + HEADER + BODY_PAD;
        std::vector<Entry*> child_entries;
        layout_blocks(block->body, body_x, inner_y, child_entries);
        entry->body_children = child_entries;
        bottom = inner_y;
        for (Entry* child : child_entries) {
            bottom = std::max(bottom, static_cast<int>(child->outer.bottom));
        }
        int body_w = 160;
        if (!child_entries.empty()) {
            int child_right = 0;
            for (Entry* child : child_entries) {
                child_right = std::max(child_right, static_cast<int>(child->outer.right));
            }
            body_w = std::max(140, child_right - (body_x - 8));
        }
        entry->body_rect = {body_x - 8, y + HEADER, body_x - 8 + body_w,
                            std::max(y + HEADER + 28, bottom + 6)};
        entry->has_body = true;

        if (block->kind == "if") {
            int else_y = bottom + ELSE_BAR + 6;
            std::vector<Entry*> else_entries;
            layout_blocks(block->else_body, body_x, else_y, else_entries);
            entry->else_children = else_entries;
            int else_bottom = else_y;
            for (Entry* child : else_entries) {
                else_bottom = std::max(else_bottom, static_cast<int>(child->outer.bottom));
            }
            int else_w = 160;
            if (!else_entries.empty()) {
                int child_right = 0;
                for (Entry* child : else_entries) {
                    child_right = std::max(child_right, static_cast<int>(child->outer.right));
                }
                else_w = std::max(140, child_right - (body_x - 8));
            }
            entry->else_rect = {body_x - 8, bottom + 2, body_x - 8 + else_w,
                                std::max(24, else_bottom - (bottom + 2)) + bottom + 2};
            entry->else_rect.bottom = std::max(bottom + 2 + 24, else_bottom);
            entry->has_else = true;
            bottom = else_bottom;
            width = std::max({width, body_w + INDENT + 24, else_w + INDENT + 24});
        } else {
            width = std::max(width, body_w + INDENT + 24);
        }

        int body_right = body_x - 8 + body_w;
        (void)body_right;
    }

    int height = bottom + BOTTOM_PAD - y;
    entry->outer = {x, y, x + width, y + height};
    return entry;
}

void BlockCanvas::layout_blocks(const std::vector<BlockPtr>& blocks, int x, int y,
                                std::vector<Entry*>& out) {
    for (const auto& block : blocks) {
        Entry* entry = layout_block(block, x, y);
        out.push_back(entry);
        y = entry->outer.bottom + GAP;
    }
}

void BlockCanvas::rebuild_layout() {
    layout_.clear();
    entries_.clear();
    std::vector<Entry*> top;
    layout_blocks(blocks_, 18, 18, top);
}

bool BlockCanvas::inside_rect(const RECT& rect, int x, int y) const {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

POINT BlockCanvas::canvas_point(int client_x, int client_y) const {
    return POINT{client_x + scroll_x_, client_y + scroll_y_};
}

BlockCanvas::Entry* BlockCanvas::hit_entry(int x, int y) const {
    for (auto it = layout_.rbegin(); it != layout_.rend(); ++it) {
        if (inside_rect((*it)->outer, x, y)) return it->get();
    }
    return nullptr;
}

void BlockCanvas::redraw() {
    rebuild_layout();
    update_scroll();
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

void BlockCanvas::update_scroll() {
    if (hwnd_ == nullptr) return;
    extent_x_ = 500;
    extent_y_ = 400;
    for (const auto& entry : layout_) {
        extent_x_ = std::max(extent_x_, static_cast<int>(entry->outer.right) + 60);
        extent_y_ = std::max(extent_y_, static_cast<int>(entry->outer.bottom) + 60);
    }
    RECT client;
    GetClientRect(hwnd_, &client);
    int page_x = client.right;
    int page_y = client.bottom;
    SCROLLINFO info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    info.nMin = 0;
    info.nMax = extent_x_ - 1;
    info.nPage = page_x;
    info.nPos = scroll_x_;
    SetScrollInfo(hwnd_, SB_HORZ, &info, TRUE);
    info.nMax = extent_y_ - 1;
    info.nPage = page_y;
    info.nPos = scroll_y_;
    SetScrollInfo(hwnd_, SB_VERT, &info, TRUE);
    scroll_x_ = std::min(scroll_x_, std::max(0, extent_x_ - page_x));
    scroll_y_ = std::min(scroll_y_, std::max(0, extent_y_ - page_y));
}

void BlockCanvas::update_palette_scroll() {
    if (palette_ == nullptr) return;
    int y = 4;
    for (auto& item : palette_items_) {
        int height = item.is_header ? PALETTE_HEADER : PALETTE_ROW;
        item.rect = {0, y, 1000, y + height};
        y += height + (item.is_header ? 0 : 2);
    }
    palette_height_ = y + 8;
    RECT client;
    GetClientRect(palette_, &client);
    SCROLLINFO info;
    std::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    info.nMin = 0;
    info.nMax = palette_height_ - 1;
    info.nPage = client.bottom;
    info.nPos = palette_scroll_;
    SetScrollInfo(palette_, SB_VERT, &info, TRUE);
    palette_scroll_ = std::min(palette_scroll_, std::max(0, palette_height_ - (int)client.bottom));
}

void BlockCanvas::paint_canvas(HDC dc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH bg = CreateSolidBrush(theme::BG);
    FillRect(dc, &client, bg);
    DeleteObject(bg);

    int saved = SaveDC(dc);
    SetViewportOrgEx(dc, -scroll_x_, -scroll_y_, nullptr);

    if (blocks_.empty()) {
        const wchar_t* text =
            L"This is your block workshop!\r\n\r\n"
            L"1. Click a block on the left to add it here.\r\n"
            L"2. Or drag it where you want it.\r\n"
            L"3. Drag boxes / numbers / text into the little slots.\r\n"
            L"4. Press RUN to run your blocks.\r\n\r\n"
            L"Right-click a block to delete or copy it.";
        RECT area{40, 40, 520, 320};
        HFONT old = static_cast<HFONT>(SelectObject(dc, ui_font_));
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, theme::DIM);
        DrawTextW(dc, text, -1, &area, DT_LEFT | DT_TOP | DT_NOPREFIX);
        SelectObject(dc, old);
    }

    for (const auto& entry : layout_) {
        draw_container(dc, entry.get());
    }
    for (const auto& entry : layout_) {
        draw_header(dc, entry.get());
        draw_fields(dc, entry.get());
    }

    if (selected_) {
        Entry* entry = entry_for(selected_.get());
        if (entry != nullptr) {
            HPEN pen = CreatePen(PS_DOT, 2, theme::WHITE);
            HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
            HBRUSH old_brush =
                static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
            Rectangle(dc, entry->outer.left - 3, entry->outer.top - 3, entry->outer.right + 3,
                      entry->outer.bottom + 3);
            SelectObject(dc, old_pen);
            SelectObject(dc, old_brush);
            DeleteObject(pen);
        }
    }

    if (has_preview_) draw_preview(dc);
    if (has_ghost_) draw_ghost(dc, ghost_.x, ghost_.y);

    RestoreDC(dc, saved);
}

void BlockCanvas::draw_container(HDC dc, Entry* entry) {
    if (entry->block == nullptr) return;
    const BlockSpec& spec = specs().at(entry->block->kind);
    if (spec.container.empty()) return;
    const RECT& outer = entry->outer;
    HBRUSH brush = CreateSolidBrush(theme::CONTAINER_BG);
    HPEN pen = CreatePen(PS_SOLID, 2, theme::PANEL3);
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
    HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
    RoundRect(dc, outer.left - 2, outer.top - 2, outer.right + 6, outer.bottom + 4, 12, 12);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);

    HFONT old_font = static_cast<HFONT>(SelectObject(dc, small_font_));
    SetBkMode(dc, TRANSPARENT);
    if (entry->has_body && entry->body_children.empty()) {
        SetTextColor(dc, theme::HINT_TEXT);
        RECT rect{entry->body_rect.left + 12, entry->body_rect.top, entry->body_rect.right,
                  entry->body_rect.top + 28};
        DrawTextW(dc, L"drop blocks here", -1, &rect, DT_LEFT | DT_TOP | DT_NOPREFIX);
    }
    if (entry->block->kind == "if" && entry->has_else) {
        SetTextColor(dc, theme::PURPLE);
        HFONT bold = ui_bold_font_;
        HFONT previous = static_cast<HFONT>(SelectObject(dc, bold));
        RECT label{outer.left + 14, entry->else_rect.top + 2, outer.right, entry->else_rect.top + 20};
        DrawTextW(dc, L"wonga", -1, &label, DT_LEFT | DT_TOP | DT_NOPREFIX);
        SelectObject(dc, previous);
        if (entry->else_children.empty()) {
            SetTextColor(dc, theme::HINT_TEXT);
            RECT hint{entry->else_rect.left + 12, entry->else_rect.top + 12,
                      entry->else_rect.right, entry->else_rect.top + 34};
            DrawTextW(dc, L"drop blocks here for 'else'", -1, &hint,
                      DT_LEFT | DT_TOP | DT_NOPREFIX);
        }
    }
    SelectObject(dc, old_font);
}

void BlockCanvas::draw_header(HDC dc, Entry* entry) {
    const RECT& outer = entry->outer;
    COLORREF color = theme::block_color(entry->block->kind);
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0x1b, 0x12, 0x0c));
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
    HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
    RoundRect(dc, outer.left, outer.top, outer.right, outer.top + HEADER, 9, 9);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);

    std::wstring title = wide_from_utf8(entry->block->title());
    HFONT old_font = static_cast<HFONT>(SelectObject(dc, ui_bold_font_));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, theme::BLOCK_TEXT);
    RECT rect{outer.left + 12, outer.top, outer.right, outer.top + HEADER};
    DrawTextW(dc, title.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(dc, old_font);
}

void BlockCanvas::draw_fields(HDC dc, Entry* entry) {
    const BlockSpec& spec = specs().at(entry->block->kind);
    COLORREF color = theme::block_color(entry->block->kind);
    HFONT old_font = static_cast<HFONT>(SelectObject(dc, code_font_));
    SetBkMode(dc, TRANSPARENT);
    for (const auto& field : spec.fields) {
        auto it = entry->fields.find(field.key);
        if (it == entry->fields.end()) continue;
        const RECT& rect = it->second;
        if (!field.label.empty()) {
            HFONT previous = static_cast<HFONT>(SelectObject(dc, small_font_));
            SetTextColor(dc, theme::BLOCK_TEXT);
            RECT label{rect.left - 90, rect.top, rect.left - 5, rect.bottom};
            DrawTextW(dc, wide_from_utf8(field.label).c_str(), -1, &label,
                      DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(dc, previous);
        }
        HBRUSH brush = CreateSolidBrush(theme::FIELD_BG);
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
        HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
        Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
        SelectObject(dc, old_brush);
        SelectObject(dc, old_pen);
        DeleteObject(brush);
        DeleteObject(pen);

        std::string value = entry->block->field(field.key);
        if (value.empty()) value = "...";
        SetTextColor(dc, theme::TEXT);
        RECT text_rect{rect.left + 5, rect.top, rect.right - 4, rect.bottom};
        DrawTextW(dc, wide_from_utf8(value).c_str(), -1, &text_rect,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }
    SelectObject(dc, old_font);
}

void BlockCanvas::draw_ghost(HDC dc, int x, int y) {
    HPEN pen = CreatePen(PS_DOT, 2, theme::WHITE);
    HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
    Rectangle(dc, x - 60, y - 14, x + 60, y + 14);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(pen);
}

void BlockCanvas::draw_preview(HDC dc) {
    if (!has_preview_) return;
    int x1 = 0;
    int x2 = 0;
    int y = 0;
    if (preview_target_.kind == BlockCanvas::DropTarget::Kind::Field) {
        Entry* entry = entry_for(preview_target_.block);
        if (entry == nullptr) return;
        auto it = entry->fields.find(preview_target_.key);
        if (it == entry->fields.end()) return;
        const RECT& rect = it->second;
        HPEN pen = CreatePen(PS_SOLID, 2, theme::WHITE);
        HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
        HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
        Rectangle(dc, rect.left - 2, rect.top - 2, rect.right + 2, rect.bottom + 2);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
        DeleteObject(pen);
        return;
    }
    if (preview_target_.kind != BlockCanvas::DropTarget::Kind::Insert) return;

    if (preview_target_.parent == nullptr) {
        if (blocks_.empty()) {
            y = 24;
        } else if (preview_target_.index >= static_cast<int>(blocks_.size())) {
            Entry* entry = entry_for(blocks_.back().get());
            if (entry != nullptr) y = entry->outer.bottom + GAP / 2;
        } else {
            Entry* entry = entry_for(blocks_[preview_target_.index].get());
            if (entry != nullptr) y = entry->outer.top - GAP / 2;
        }
        x1 = 20;
        x2 = 420;
    } else {
        Entry* entry = entry_for(preview_target_.parent);
        if (entry == nullptr) return;
        const std::vector<Entry*>& children =
            preview_target_.which == "body" ? entry->body_children : entry->else_children;
        if (children.empty()) {
            const RECT& rect = preview_target_.which == "body" ? entry->body_rect
                                                               : entry->else_rect;
            y = rect.top + 14;
            x1 = rect.left + 6;
            x2 = rect.right + 6;
        } else if (preview_target_.index >= static_cast<int>(children.size())) {
            Entry* child = children.back();
            y = child->outer.bottom + GAP / 2;
            x1 = child->outer.left;
            x2 = child->outer.right;
        } else {
            Entry* child = children[preview_target_.index];
            y = child->outer.top - GAP / 2;
            x1 = child->outer.left;
            x2 = child->outer.right;
        }
    }

    HPEN pen = CreatePen(PS_SOLID, 3, theme::WHITE);
    HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
    MoveToEx(dc, x1, y, nullptr);
    LineTo(dc, x2, y);
    SelectObject(dc, old_pen);
    DeleteObject(pen);
}

BlockCanvas::DropTarget BlockCanvas::drop_target(int x, int y) const {
    DropTarget target;
    for (auto it = layout_.rbegin(); it != layout_.rend(); ++it) {
        Entry* entry = it->get();
        for (const auto& field : entry->fields) {
            if (inside_rect(field.second, x, y)) {
                target.kind = DropTarget::Kind::Field;
                target.block = entry->block.get();
                target.key = field.first;
                return target;
            }
        }
    }
    for (auto it = layout_.rbegin(); it != layout_.rend(); ++it) {
        Entry* entry = it->get();
        if (entry->has_else && inside_rect(entry->else_rect, x, y)) {
            int index = static_cast<int>(entry->else_children.size());
            for (std::size_t i = 0; i < entry->else_children.size(); ++i) {
                const RECT& rect = entry->else_children[i]->outer;
                if (y < rect.top + (rect.bottom - rect.top) / 2) {
                    index = static_cast<int>(i);
                    break;
                }
            }
            target.kind = DropTarget::Kind::Insert;
            target.parent = entry->block.get();
            target.which = "else";
            target.index = index;
            return target;
        }
        if (entry->has_body && inside_rect(entry->body_rect, x, y)) {
            int index = static_cast<int>(entry->body_children.size());
            for (std::size_t i = 0; i < entry->body_children.size(); ++i) {
                const RECT& rect = entry->body_children[i]->outer;
                if (y < rect.top + (rect.bottom - rect.top) / 2) {
                    index = static_cast<int>(i);
                    break;
                }
            }
            target.kind = DropTarget::Kind::Insert;
            target.parent = entry->block.get();
            target.which = "body";
            target.index = index;
            return target;
        }
    }

    int index = static_cast<int>(blocks_.size());
    for (std::size_t i = 0; i < blocks_.size(); ++i) {
        Entry* entry = entry_for(blocks_[i].get());
        if (entry == nullptr) continue;
        const RECT& rect = entry->outer;
        if (y < rect.top + (rect.bottom - rect.top) / 2) {
            index = static_cast<int>(i);
            break;
        }
    }
    target.kind = DropTarget::Kind::Insert;
    target.parent = nullptr;
    target.which = "body";
    target.index = index;
    return target;
}

void BlockCanvas::canvas_press(int x, int y) {
    SetFocus(hwnd_);
    commit_field_editor();
    POINT point = canvas_point(x, y);

    for (auto it = layout_.rbegin(); it != layout_.rend(); ++it) {
        Entry* entry = it->get();
        if (!inside_rect(entry->outer, point.x, point.y)) continue;
        for (const auto& field : entry->fields) {
            if (inside_rect(field.second, point.x, point.y)) {
                edit_field(entry->block, field.first, field.second);
                return;
            }
        }
        selected_ = entry->block;
        drag_ = DragState();
        drag_.active = true;
        drag_.block = entry->block;
        drag_.pointer = point;
        drag_.press = point;
        redraw();
        return;
    }
    selected_ = nullptr;
    redraw();
}

void BlockCanvas::canvas_motion(int x, int y) {
    if (!drag_.active) return;
    POINT point = canvas_point(x, y);
    if (!drag_.moved) {
        int dx = std::abs(point.x - drag_.press.x);
        int dy = std::abs(point.y - drag_.press.y);
        if (dx < DRAG_SLOP && dy < DRAG_SLOP) return;
        drag_.moved = true;
    }
    drag_.pointer = point;
    ghost_ = point;
    has_ghost_ = true;
    DropTarget target = drop_target(point.x, point.y);
    if (target.kind == DropTarget::Kind::Field && !drag_.block->is_value()) {
        target.kind = DropTarget::Kind::None;
    }
    preview_target_ = target;
    has_preview_ = target.kind != DropTarget::Kind::None;
    redraw();
}

void BlockCanvas::canvas_release(int x, int y, bool inside) {
    if (!drag_.active) return;
    DragState drag = drag_;
    drag_ = DragState();
    has_ghost_ = false;
    has_preview_ = false;

    if (!drag.moved) {
        redraw();
        return;
    }

    if (!inside) {
        snapshot();
        delete_block_from_tree(drag.block.get());
        selected_ = nullptr;
        redraw();
        notify();
        status("Deleted block.");
        return;
    }

    POINT point = canvas_point(x, y);
    DropTarget target = drop_target(point.x, point.y);
    snapshot();
    delete_block_from_tree(drag.block.get());
    if (target.kind == DropTarget::Kind::Field && drag.block->is_value()) {
        Block* block = target.block;
        block->fields[target.key] = value_expression(drag.block);
        Entry* target_entry = entry_for(block);
        selected_ = target_entry != nullptr ? target_entry->block : nullptr;
        redraw();
        notify();
        return;
    }
    if (target.kind == DropTarget::Kind::Field) {
        redraw();
        status("Only values (number, text, box, random) fit in a slot.");
        return;
    }
    std::vector<BlockPtr>* list = nullptr;
    target_list(target.parent, target.which, &list);
    if (list != nullptr) {
        int index = std::min(target.index, static_cast<int>(list->size()));
        if (index < 0) index = 0;
        list->insert(list->begin() + index, drag.block);
        selected_ = drag.block;
        redraw();
        notify();
    }
}

void BlockCanvas::context_menu(int x, int y) {
    commit_field_editor();
    POINT point = canvas_point(x, y);
    Entry* entry = hit_entry(point.x, point.y);
    if (entry == nullptr) return;
    selected_ = entry->block;
    redraw();

    Block* hit = entry->block.get();
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"Delete");
    AppendMenuW(menu, MF_STRING, 2, L"Copy");
    AppendMenuW(menu, MF_STRING, 3, L"Move up");
    AppendMenuW(menu, MF_STRING, 4, L"Move down");
    POINT screen;
    GetCursorPos(&screen);
    int command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen.x, screen.y, 0,
                                 hwnd_, nullptr);
    DestroyMenu(menu);
    if (command == 1) {
        delete_selected();
    } else if (command == 2) {
        duplicate_block(hit);
    } else if (command == 3) {
        move_block(hit, -1);
    } else if (command == 4) {
        move_block(hit, 1);
    }
}

void BlockCanvas::duplicate_block(Block* block) {
    Block* parent = nullptr;
    std::string which;
    int index = 0;
    if (!find_location(block, &blocks_, nullptr, "body", &parent, &which, &index)) return;
    snapshot();
    std::vector<BlockPtr>* list = nullptr;
    target_list(parent, which, &list);
    if (list != nullptr) {
        Entry* entry = entry_for(block);
        if (entry != nullptr) {
            list->insert(list->begin() + index + 1, clone_block(entry->block));
        }
    }
    redraw();
    notify();
}

void BlockCanvas::move_block(Block* block, int direction) {
    Block* parent = nullptr;
    std::string which;
    int index = 0;
    if (!find_location(block, &blocks_, nullptr, "body", &parent, &which, &index)) return;
    std::vector<BlockPtr>* list = nullptr;
    target_list(parent, which, &list);
    if (list == nullptr) return;
    int new_index = index + direction;
    if (new_index < 0 || new_index >= static_cast<int>(list->size())) return;
    snapshot();
    std::swap((*list)[index], (*list)[new_index]);
    redraw();
    notify();
}

void BlockCanvas::palette_press(int x, int y) {
    (void)x;
    (void)y;
    int spot = y + palette_scroll_;
    for (const auto& item : palette_items_) {
        if (item.is_header) continue;
        if (spot >= item.rect.top && spot <= item.rect.bottom) {
            POINT screen;
            GetCursorPos(&screen);
            drag_ = DragState();
            drag_.active = true;
            drag_.from_palette = true;
            drag_.kind = item.kind;
            drag_.block = new_block(item.kind);
            drag_.press = screen;
            drag_.pointer = screen;
            SetCapture(palette_);
            return;
        }
    }
}

void BlockCanvas::palette_motion(int x, int y) {
    (void)x;
    (void)y;
    if (!drag_.active || !drag_.from_palette) return;
    POINT screen;
    GetCursorPos(&screen);
    if (!drag_.moved) {
        int dx = std::abs(screen.x - drag_.press.x);
        int dy = std::abs(screen.y - drag_.press.y);
        if (dx < DRAG_SLOP && dy < DRAG_SLOP) return;
        drag_.moved = true;
    }
    drag_.pointer = screen;
    POINT client = screen;
    ScreenToClient(hwnd_, &client);
    RECT canvas_rect;
    GetClientRect(hwnd_, &canvas_rect);
    bool inside = client.x >= 0 && client.y >= 0 && client.x < canvas_rect.right
                  && client.y < canvas_rect.bottom;
    if (inside) {
        POINT point = canvas_point(client.x, client.y);
        ghost_ = point;
        has_ghost_ = true;
        DropTarget target = drop_target(point.x, point.y);
        if (target.kind == DropTarget::Kind::Field && !drag_.block->is_value()) {
            target.kind = DropTarget::Kind::None;
        }
        preview_target_ = target;
        has_preview_ = target.kind != DropTarget::Kind::None;
    } else {
        has_ghost_ = false;
        has_preview_ = false;
    }
    InvalidateRect(hwnd_, nullptr, TRUE);
}

void BlockCanvas::palette_release(int x, int y) {
    (void)x;
    (void)y;
    if (!drag_.active || !drag_.from_palette) return;
    ReleaseCapture();
    DragState drag = drag_;
    drag_ = DragState();
    has_ghost_ = false;
    has_preview_ = false;

    if (!drag.moved) {
        add_block(drag.kind);
        return;
    }

    POINT screen;
    GetCursorPos(&screen);
    POINT client = screen;
    ScreenToClient(hwnd_, &client);
    RECT canvas_rect;
    GetClientRect(hwnd_, &canvas_rect);
    bool inside = client.x >= 0 && client.y >= 0 && client.x < canvas_rect.right
                  && client.y < canvas_rect.bottom;
    if (!inside) {
        redraw();
        status("Drag cancelled.");
        return;
    }

    POINT point = canvas_point(client.x, client.y);
    DropTarget target = drop_target(point.x, point.y);
    if (target.kind == DropTarget::Kind::Field && drag.block->is_value()) {
        snapshot();
        Block* block = target.block;
        block->fields[target.key] = value_expression(drag.block);
        Entry* target_entry = entry_for(block);
        selected_ = target_entry != nullptr ? target_entry->block : nullptr;
    } else if (target.kind == DropTarget::Kind::Field) {
        redraw();
        status("Only values (number, text, box, random) fit in a slot.");
        return;
    } else {
        snapshot();
        std::vector<BlockPtr>* list = nullptr;
        target_list(target.parent, target.which, &list);
        if (list != nullptr) {
            int index = std::min(target.index, static_cast<int>(list->size()));
            if (index < 0) index = 0;
            list->insert(list->begin() + index, drag.block);
            selected_ = drag.block;
        }
    }
    redraw();
    notify();
}

const FieldSpec* BlockCanvas::field_spec(const Block& block, const std::string& key) const {
    auto it = specs().find(block.kind);
    if (it == specs().end()) return nullptr;
    for (const auto& field : it->second.fields) {
        if (field.key == key) return &field;
    }
    return nullptr;
}

void BlockCanvas::edit_field(BlockPtr block, const std::string& key, const RECT& rect) {
    commit_field_editor();
    field_block_ = block;
    field_key_ = key;
    field_rect_ = rect;
    int x = rect.left - scroll_x_;
    int y = rect.top - scroll_y_;
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    field_edit_ = CreateWindowExW(
        0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y, w, h, hwnd_, nullptr,
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(field_edit_, WM_SETFONT, reinterpret_cast<WPARAM>(code_font_), TRUE);
    SetWindowTextW(field_edit_, wide_from_utf8(block->field(key)).c_str());
    SendMessageW(field_edit_, EM_SETSEL, 0, -1);
    SetWindowLongPtrW(field_edit_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    SetWindowSubclass(field_edit_, BlockCanvas::field_proc, 1, 0);
    SetFocus(field_edit_);
}

void BlockCanvas::commit_field_editor() {
    if (field_edit_ == nullptr) return;
    HWND editor = field_edit_;
    BlockPtr block = field_block_;
    std::string key = field_key_;
    field_edit_ = nullptr;
    field_block_ = nullptr;
    field_key_.clear();
    int length = GetWindowTextLengthW(editor);
    std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
    GetWindowTextW(editor, text.data(), length + 1);
    text.resize(static_cast<std::size_t>(length));
    DestroyWindow(editor);
    std::string value = utf8_from_wide(text);

    if (block == nullptr) return;
    if (valid_field(block, key, value)) {
        std::string old = block->field(key);
        if (old != value) {
            snapshot();
            block->fields[key] = value;
            redraw();
            notify();
        } else {
            redraw();
        }
    } else {
        status("That does not fit in the slot. Try again!");
        redraw();
    }
}

void BlockCanvas::cancel_field_editor() {
    if (field_edit_ == nullptr) return;
    HWND editor = field_edit_;
    field_edit_ = nullptr;
    field_block_ = nullptr;
    field_key_.clear();
    DestroyWindow(editor);
    redraw();
}

bool BlockCanvas::valid_field(const BlockPtr& block, const std::string& key,
                              const std::string& text) {
    const FieldSpec* field = field_spec(*block, key);
    if (field == nullptr) return true;
    std::string trimmed = text;
    std::size_t start = trimmed.find_first_not_of(" \t\r\n");
    std::size_t end = trimmed.find_last_not_of(" \t\r\n");
    trimmed = (start == std::string::npos) ? "" : trimmed.substr(start, end - start + 1);
    const std::string& type = field->type;
    if (type == "name") {
        return is_good_name(trimmed) && !is_forbidden_block_name(trimmed);
    }
    if (type == "params") {
        if (trimmed.empty()) return true;
        std::size_t pos = 0;
        while (pos <= trimmed.size()) {
            std::size_t comma = trimmed.find(',', pos);
            std::string piece = trimmed.substr(
                pos, comma == std::string::npos ? std::string::npos : comma - pos);
            std::size_t ps = piece.find_first_not_of(" \t\r\n");
            std::size_t pe = piece.find_last_not_of(" \t\r\n");
            piece = (ps == std::string::npos) ? "" : piece.substr(ps, pe - ps + 1);
            if (!piece.empty() && !is_good_name(piece)) return false;
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
        return true;
    }
    if (type == "expr") {
        if (trimmed.empty()
            && ((block->kind == "ork" && key == "expr")
                || (block->kind == "ask" && key == "prompt"))) {
            return true;
        }
        try {
            cmc::parse_expression_source(trimmed);
            return true;
        } catch (cmc::CmcError&) {
            return false;
        }
    }
    if (type == "args") {
        if (trimmed.empty()) return true;
        try {
            cmc::parse_source("f(" + trimmed + ")");
            return true;
        } catch (cmc::CmcError&) {
            return false;
        }
    }
    if (type == "number") {
        if (trimmed.empty()) return false;
        char* stop = nullptr;
        std::strtod(trimmed.c_str(), &stop);
        return stop != nullptr && *stop == '\0' && stop != trimmed.c_str();
    }
    return true;
}

LRESULT CALLBACK BlockCanvas::field_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                                         UINT_PTR subclass_id, DWORD_PTR ref_data) {
    (void)subclass_id;
    (void)ref_data;
    if (message == WM_KEYDOWN) {
        BlockCanvas* canvas =
            reinterpret_cast<BlockCanvas*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (canvas != nullptr) {
            if (wparam == VK_RETURN) {
                PostMessageW(canvas->hwnd_, WM_FIELD_COMMIT, 0, 0);
                return 0;
            }
            if (wparam == VK_ESCAPE) {
                PostMessageW(canvas->hwnd_, WM_FIELD_CANCEL, 0, 0);
                return 0;
            }
        }
    }
    if (message == WM_KILLFOCUS) {
        BlockCanvas* canvas =
            reinterpret_cast<BlockCanvas*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (canvas != nullptr) PostMessageW(canvas->hwnd_, WM_FIELD_COMMIT, 0, 0);
    }
    return DefSubclassProc(hwnd, message, wparam, lparam);
}

LRESULT BlockCanvas::canvas_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            RECT client;
            GetClientRect(hwnd_, &client);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mem, bitmap));
            paint_canvas(mem);
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old_bitmap);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
            update_scroll();
            return 0;
        case WM_LBUTTONDOWN:
            canvas_press(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            if (drag_.active) SetCapture(hwnd_);
            return 0;
        case WM_MOUSEMOVE:
            if (drag_.active && !drag_.from_palette) {
                canvas_motion(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            }
            return 0;
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            ReleaseCapture();
            RECT client;
            GetClientRect(hwnd_, &client);
            bool inside = x >= 0 && y >= 0 && x < client.right && y < client.bottom;
            canvas_release(x, y, inside);
            return 0;
        }
        case WM_RBUTTONUP:
            context_menu(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;
        case WM_KEYDOWN:
            if (wparam == VK_DELETE) {
                delete_selected();
                return 0;
            }
            if (wparam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                undo();
                return 0;
            }
            return 0;
        case WM_HSCROLL:
        case WM_VSCROLL: {
            SCROLLINFO info;
            std::memset(&info, 0, sizeof(info));
            info.cbSize = sizeof(info);
            info.fMask = SIF_ALL;
            int bar = message == WM_HSCROLL ? SB_HORZ : SB_VERT;
            GetScrollInfo(hwnd_, bar, &info);
            int pos = info.nPos;
            switch (LOWORD(wparam)) {
                case SB_LINEUP:
                    pos -= 20;
                    break;
                case SB_LINEDOWN:
                    pos += 20;
                    break;
                case SB_PAGEUP:
                    pos -= static_cast<int>(info.nPage);
                    break;
                case SB_PAGEDOWN:
                    pos += static_cast<int>(info.nPage);
                    break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    pos = info.nTrackPos;
                    break;
                default:
                    break;
            }
            int max_pos = std::max(0, static_cast<int>(info.nMax) - static_cast<int>(info.nPage) + 1);
            pos = std::max(0, std::min(pos, max_pos));
            if (bar == SB_HORZ) {
                scroll_x_ = pos;
            } else {
                scroll_y_ = pos;
            }
            SetScrollPos(hwnd_, bar, pos, TRUE);
            InvalidateRect(hwnd_, nullptr, TRUE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            scroll_y_ = std::max(0, scroll_y_ - delta / 120 * 40);
            SetScrollPos(hwnd_, SB_VERT, scroll_y_, TRUE);
            InvalidateRect(hwnd_, nullptr, TRUE);
            return 0;
        }
        case WM_FIELD_COMMIT:
            commit_field_editor();
            return 0;
        case WM_FIELD_CANCEL:
            cancel_field_editor();
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT BlockCanvas::palette_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(palette_, &ps);
            paint_palette(dc);
            EndPaint(palette_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONDOWN:
            palette_press(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;
        case WM_MOUSEMOVE:
            if (drag_.active && drag_.from_palette) {
                palette_motion(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            }
            return 0;
        case WM_LBUTTONUP:
            palette_release(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;
        case WM_VSCROLL: {
            SCROLLINFO info;
            std::memset(&info, 0, sizeof(info));
            info.cbSize = sizeof(info);
            info.fMask = SIF_ALL;
            GetScrollInfo(palette_, SB_VERT, &info);
            int pos = info.nPos;
            switch (LOWORD(wparam)) {
                case SB_LINEUP:
                    pos -= 24;
                    break;
                case SB_LINEDOWN:
                    pos += 24;
                    break;
                case SB_PAGEUP:
                    pos -= static_cast<int>(info.nPage);
                    break;
                case SB_PAGEDOWN:
                    pos += static_cast<int>(info.nPage);
                    break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    pos = info.nTrackPos;
                    break;
                default:
                    break;
            }
            int max_pos = std::max(0, static_cast<int>(info.nMax) - static_cast<int>(info.nPage) + 1);
            palette_scroll_ = std::max(0, std::min(pos, max_pos));
            SetScrollPos(palette_, SB_VERT, palette_scroll_, TRUE);
            InvalidateRect(palette_, nullptr, TRUE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            palette_scroll_ = std::max(0, palette_scroll_ - delta / 120 * 40);
            SetScrollPos(palette_, SB_VERT, palette_scroll_, TRUE);
            InvalidateRect(palette_, nullptr, TRUE);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(palette_, message, wparam, lparam);
}

void BlockCanvas::paint_palette(HDC dc) {
    RECT client;
    GetClientRect(palette_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);

    int saved = SaveDC(dc);
    SetViewportOrgEx(dc, 0, -palette_scroll_, nullptr);
    SetBkMode(dc, TRANSPARENT);
    for (const auto& item : palette_items_) {
        if (item.rect.bottom < palette_scroll_ || item.rect.top > palette_scroll_ + client.bottom) {
            continue;
        }
        if (item.is_header) {
            HFONT old = static_cast<HFONT>(SelectObject(dc, small_font_));
            SetTextColor(dc, theme::category_color(item.category));
            RECT rect{8, item.rect.top, client.right, item.rect.bottom};
            DrawTextW(dc, wide_from_utf8(item.category).c_str(), -1, &rect,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(dc, old);
        } else {
            COLORREF color = theme::block_color(item.kind);
            HBRUSH brush = CreateSolidBrush(color);
            RECT rect{8, item.rect.top, client.right - 10, item.rect.bottom};
            FillRect(dc, &rect, brush);
            DeleteObject(brush);
            HFONT old = static_cast<HFONT>(SelectObject(dc, ui_bold_font_));
            SetTextColor(dc, theme::BLOCK_TEXT);
            RECT text_rect{14, item.rect.top, client.right - 12, item.rect.bottom};
            DrawTextW(dc, wide_from_utf8(item.title).c_str(), -1, &text_rect,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            SelectObject(dc, old);
        }
    }
    RestoreDC(dc, saved);
}

LRESULT CALLBACK BlockCanvas::canvas_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    BlockCanvas* canvas = reinterpret_cast<BlockCanvas*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        canvas = static_cast<BlockCanvas*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(canvas));
    }
    if (canvas == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    canvas->hwnd_ = hwnd;
    return canvas->canvas_message(message, wparam, lparam);
}

LRESULT CALLBACK BlockCanvas::palette_proc(HWND hwnd, UINT message, WPARAM wparam,
                                           LPARAM lparam) {
    BlockCanvas* canvas = reinterpret_cast<BlockCanvas*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        canvas = static_cast<BlockCanvas*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(canvas));
    }
    if (canvas == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    canvas->palette_ = hwnd;
    return canvas->palette_message(message, wparam, lparam);
}

}