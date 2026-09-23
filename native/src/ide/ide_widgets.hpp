#pragma once

#include <windows.h>

#include <functional>
#include <string>
#include <vector>

namespace og {

class ButtonBar {
public:
    explicit ButtonBar(HWND parent, bool small = false);
    ~ButtonBar();

    HWND handle() const { return hwnd_; }
    void add(int id, const std::wstring& text, COLORREF bg, COLORREF fg);
    void add_separator();
    void layout(int x, int y, int w, int h);
    void set_callback(std::function<void(int)> callback) { callback_ = std::move(callback); }
    void set_font(HFONT font) { font_ = font; }

private:
    struct Item {
        int id = 0;
        std::wstring text;
        COLORREF bg = 0;
        COLORREF fg = 0;
        bool separator = false;
        RECT rect{};
    };

    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);
    void paint(HDC dc);
    int hit(int x, int y) const;

    HWND hwnd_ = nullptr;
    std::vector<Item> items_;
    std::function<void(int)> callback_;
    HFONT font_ = nullptr;
    bool small_ = false;
    int hover_ = -1;
};

class TabBar {
public:
    explicit TabBar(HWND parent);
    ~TabBar();

    HWND handle() const { return hwnd_; }
    void layout(int x, int y, int w, int h);
    void set_labels(const std::vector<std::wstring>& labels);
    void select(int index);
    int selected() const { return selected_; }
    void set_callback(std::function<void(int)> callback) { callback_ = std::move(callback); }
    void set_font(HFONT font) { font_ = font; }

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);
    void paint(HDC dc);
    int hit(int x, int y) const;

    HWND hwnd_ = nullptr;
    std::vector<std::wstring> labels_;
    std::vector<RECT> rects_;
    int selected_ = 0;
    std::function<void(int)> callback_;
    HFONT font_ = nullptr;
};

class OutputPane {
public:
    explicit OutputPane(HWND parent, HFONT header_font, HFONT text_font);
    ~OutputPane();

    HWND handle() const { return hwnd_; }
    void layout(int x, int y, int w, int h);
    void clear();
    void write(const std::string& message, const std::string& tag);
    void set_status(const std::string& message, COLORREF color);
    void set_text_font(HFONT font);

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);
    void paint(HDC dc);
    void append_text(const std::string& message, COLORREF color);
    void append_wide(const std::wstring& text, COLORREF color);

    HWND hwnd_ = nullptr;
    HWND edit_ = nullptr;
    HFONT header_font_ = nullptr;
    HFONT text_font_ = nullptr;
    std::wstring status_ = L"ready";
    COLORREF status_color_ = 0;
    RECT clear_rect_{};
    bool has_text_ = false;
};

class StatusBar {
public:
    explicit StatusBar(HWND parent, HFONT font);
    ~StatusBar();

    HWND handle() const { return hwnd_; }
    void layout(int x, int y, int w, int h);
    void set_message(const std::string& message, COLORREF color);
    void set_position(const std::string& message);

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);

    HWND hwnd_ = nullptr;
    HFONT font_ = nullptr;
    std::wstring message_ = L"Welcome to OGABOOGA CODER! Press RUN to try your code.";
    COLORREF message_color_ = 0;
    std::wstring position_;
};

}