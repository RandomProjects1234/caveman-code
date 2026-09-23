#pragma once

#include <windows.h>

#include <functional>
#include <string>
#include <vector>

namespace og {

void ensure_richedit_loaded();
HFONT make_font(const wchar_t* face, int point_size, bool bold = false);
std::wstring wide_from_utf8(const std::string& text);
std::string utf8_from_wide(const std::wstring& text);
void apply_highlight(HWND rich_edit, bool bold_keywords);
void set_rich_text(HWND edit, const std::wstring& text);
std::vector<std::wstring> highlight_keywords();
std::vector<std::wstring> highlight_builtins();
std::vector<std::wstring> highlight_constants();

class CodeEditor {
public:
    using ChangeFn = std::function<void()>;
    using PositionFn = std::function<void()>;

    CodeEditor(HWND parent, HFONT font);
    ~CodeEditor();

    HWND handle() const { return hwnd_; }
    void layout(int x, int y, int w, int h);

    std::string get_text() const;
    void set_text(const std::string& source);
    void clear();
    void focus_editor();
    std::string position_text() const;
    void goto_line(int number);
    void set_font(HFONT font);

    void set_on_change(ChangeFn fn) { on_change_ = std::move(fn); }
    void set_on_position(PositionFn fn) { on_position_ = std::move(fn); }

private:
    static LRESULT CALLBACK container_proc(HWND hwnd, UINT message, WPARAM wparam,
                                           LPARAM lparam);
    static LRESULT CALLBACK gutter_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK edit_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                                      UINT_PTR subclass_id, DWORD_PTR ref_data);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT gutter_message(UINT message, WPARAM wparam, LPARAM lparam);
    void refresh();
    void update_gutter();
    void paint_gutter(HDC dc);
    void handle_tab();
    void handle_return();

    HWND hwnd_ = nullptr;
    HWND gutter_ = nullptr;
    HWND edit_ = nullptr;
    HFONT font_ = nullptr;
    bool loading_ = false;
    int gutter_width_ = 44;
    ChangeFn on_change_;
    PositionFn on_position_;
};

}