#pragma once

#include <windows.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ide_blocks.hpp"

namespace og {

class BlockCanvas {
public:
    using ChangeFn = std::function<void()>;
    using StatusFn = std::function<void(const std::string&)>;

    BlockCanvas(HWND parent, HFONT ui_font, HFONT ui_bold_font, HFONT small_font,
                HFONT code_font);
    ~BlockCanvas();

    HWND handle() const { return hwnd_; }
    HWND palette_handle() const { return palette_; }
    void layout(int x, int y, int w, int h);
    void layout_palette(int x, int y, int w, int h);

    std::vector<BlockPtr> get_blocks() const { return blocks_; }
    void set_blocks(std::vector<BlockPtr> blocks, bool remember = true);
    std::string source() const;
    void clear_all();
    void undo();
    void add_block(const std::string& kind);
    void delete_selected();

    void set_on_change(ChangeFn fn) { on_change_ = std::move(fn); }
    void set_on_status(StatusFn fn) { on_status_ = std::move(fn); }

private:
    struct Entry {
        BlockPtr block;
        std::map<std::string, RECT> fields;
        std::vector<Entry*> body_children;
        std::vector<Entry*> else_children;
        RECT outer{};
        RECT body_rect{};
        RECT else_rect{};
        bool has_body = false;
        bool has_else = false;
    };

    struct DragState {
        bool active = false;
        BlockPtr block;
        bool from_palette = false;
        bool moved = false;
        POINT press{};
        POINT pointer{};
        std::string kind;
    };

    struct PaletteItem {
        std::string kind;
        std::string title;
        std::string category;
        RECT rect{};
        bool is_header = false;
    };

    static LRESULT CALLBACK canvas_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK palette_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK field_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                                       UINT_PTR subclass_id, DWORD_PTR ref_data);

    LRESULT canvas_message(UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT palette_message(UINT message, WPARAM wparam, LPARAM lparam);

    void notify();
    void status(const std::string& message);
    void snapshot();
    void rebuild_layout();
    Entry* layout_block(const BlockPtr& block, int x, int y);
    void layout_blocks(const std::vector<BlockPtr>& blocks, int x, int y,
                       std::vector<Entry*>& out);
    Entry* entry_for(const Block* block) const;
    void target_list(Block* parent, const std::string& which,
                     std::vector<BlockPtr>** out_list);
    bool find_location(Block* block, std::vector<BlockPtr>* blocks, Block* parent,
                       const std::string& which, Block** out_parent, std::string* out_which,
                       int* out_index);
    void delete_block_from_tree(Block* block);

    void redraw();
    void paint_canvas(HDC dc);
    void paint_palette(HDC dc);
    void draw_container(HDC dc, Entry* entry);
    void draw_header(HDC dc, Entry* entry);
    void draw_fields(HDC dc, Entry* entry);
    void draw_ghost(HDC dc, int x, int y);
    void draw_preview(HDC dc);
    void update_scroll();
    void update_palette_scroll();

    POINT canvas_point(int client_x, int client_y) const;
    bool inside_rect(const RECT& rect, int x, int y) const;
    Entry* hit_entry(int x, int y) const;

    struct DropTarget {
        enum class Kind { Field, Insert, None } kind = Kind::None;
        Block* block = nullptr;
        std::string key;
        Block* parent = nullptr;
        std::string which;
        int index = 0;
    };
    DropTarget drop_target(int x, int y) const;

    void canvas_press(int x, int y);
    void canvas_motion(int x, int y);
    void canvas_release(int x, int y, bool inside);
    void context_menu(int x, int y);

    void palette_press(int x, int y);
    void palette_motion(int x, int y);
    void palette_release(int x, int y);

    void edit_field(BlockPtr block, const std::string& key, const RECT& rect);
    void commit_field_editor();
    void cancel_field_editor();
    bool valid_field(const BlockPtr& block, const std::string& key, const std::string& text);
    const FieldSpec* field_spec(const Block& block, const std::string& key) const;

    void duplicate_block(Block* block);
    void move_block(Block* block, int direction);

    HWND hwnd_ = nullptr;
    HWND palette_ = nullptr;
    HFONT ui_font_ = nullptr;
    HFONT ui_bold_font_ = nullptr;
    HFONT small_font_ = nullptr;
    HFONT code_font_ = nullptr;

    std::vector<BlockPtr> blocks_;
    std::vector<std::vector<BlockPtr>> history_;
    std::vector<std::unique_ptr<Entry>> layout_;
    std::map<const Block*, Entry*> entries_;
    std::vector<PaletteItem> palette_items_;
    int palette_height_ = 0;
    int palette_scroll_ = 0;
    int scroll_x_ = 0;
    int scroll_y_ = 0;
    int extent_x_ = 500;
    int extent_y_ = 400;

    BlockPtr selected_;
    DragState drag_;
    DropTarget preview_target_;
    bool has_preview_ = false;
    POINT ghost_{0, 0};
    bool has_ghost_ = false;

    HWND field_edit_ = nullptr;
    BlockPtr field_block_;
    std::string field_key_;
    RECT field_rect_{};

    ChangeFn on_change_;
    StatusFn on_status_;
};

}