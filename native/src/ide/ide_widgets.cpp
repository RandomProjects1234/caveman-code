#include "ide_widgets.hpp"

#include <richedit.h>
#include <windowsx.h>

#include <cstring>

#include "ide_editor.hpp"
#include "ide_theme.hpp"

namespace og {

namespace {

const wchar_t* BAR_CLASS = L"OgaboogaButtonBar";
const wchar_t* TAB_CLASS = L"OgaboogaTabBar";
const wchar_t* OUTPUT_CLASS = L"OgaboogaOutputPane";
const wchar_t* STATUS_CLASS = L"OgaboogaStatusBar";

int width_of(HDC dc, HFONT font, const std::wstring& text) {
    HFONT old = static_cast<HFONT>(SelectObject(dc, font));
    SIZE size;
    GetTextExtentPoint32W(dc, text.c_str(), static_cast<int>(text.size()), &size);
    SelectObject(dc, old);
    return size.cx;
}

}

ButtonBar::ButtonBar(HWND parent, bool small) : small_(small) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = ButtonBar::wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_HAND);
        wc.lpszClassName = BAR_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    hwnd_ = CreateWindowExW(0, BAR_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, parent,
                            nullptr, GetModuleHandleW(nullptr), this);
}

ButtonBar::~ButtonBar() {
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}

void ButtonBar::add(int id, const std::wstring& text, COLORREF bg, COLORREF fg) {
    Item item;
    item.id = id;
    item.text = text;
    item.bg = bg;
    item.fg = fg;
    items_.push_back(item);
}

void ButtonBar::add_separator() {
    Item item;
    item.separator = true;
    items_.push_back(item);
}

void ButtonBar::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

void ButtonBar::paint(HDC dc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);
    if (font_ == nullptr) return;

    SetBkMode(dc, TRANSPARENT);
    int pad = small_ ? 4 : 8;
    int pad_y = small_ ? 3 : 6;
    int x = pad;
    int height = client.bottom - pad_y * 2;
    int start_y = pad_y;
    for (auto& item : items_) {
        if (item.separator) {
            RECT bar{x + 4, start_y + 4, x + 6, start_y + height - 4};
            HBRUSH panel3 = CreateSolidBrush(theme::PANEL3);
            FillRect(dc, &bar, panel3);
            DeleteObject(panel3);
            item.rect = bar;
            x += 14;
            continue;
        }
        int text_w = width_of(dc, font_, item.text);
        int button_w = text_w + (small_ ? 16 : 26);
        RECT rect{x, start_y, x + button_w, start_y + height};
        COLORREF bg = (hover_ == item.id) ? theme::PANEL3 : item.bg;
        HBRUSH brush = CreateSolidBrush(bg);
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
        item.rect = rect;
        HFONT old = static_cast<HFONT>(SelectObject(dc, font_));
        SetTextColor(dc, item.fg);
        RECT text_rect = rect;
        DrawTextW(dc, item.text.c_str(), -1, &text_rect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(dc, old);
        x += button_w + 6;
    }
}

int ButtonBar::hit(int x, int y) const {
    for (const auto& item : items_) {
        if (item.separator) continue;
        if (x >= item.rect.left && x <= item.rect.right && y >= item.rect.top
            && y <= item.rect.bottom) {
            return item.id;
        }
    }
    return -1;
}

LRESULT ButtonBar::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            paint(dc);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEMOVE: {
            int id = hit(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            if (id != hover_) {
                hover_ = id;
                InvalidateRect(hwnd_, nullptr, TRUE);
            }
            TRACKMOUSEEVENT track;
            track.cbSize = sizeof(track);
            track.dwFlags = TME_LEAVE;
            track.hwndTrack = hwnd_;
            TrackMouseEvent(&track);
            return 0;
        }
        case WM_MOUSELEAVE:
            hover_ = -1;
            InvalidateRect(hwnd_, nullptr, TRUE);
            return 0;
        case WM_LBUTTONUP: {
            int id = hit(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            if (id >= 0 && callback_) callback_(id);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT CALLBACK ButtonBar::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    ButtonBar* bar = reinterpret_cast<ButtonBar*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        bar = static_cast<ButtonBar*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(bar));
    }
    if (bar == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    bar->hwnd_ = hwnd;
    return bar->handle_message(message, wparam, lparam);
}

TabBar::TabBar(HWND parent) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = TabBar::wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_HAND);
        wc.lpszClassName = TAB_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    hwnd_ = CreateWindowExW(0, TAB_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, parent,
                            nullptr, GetModuleHandleW(nullptr), this);
}

TabBar::~TabBar() {
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}

void TabBar::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

void TabBar::set_labels(const std::vector<std::wstring>& labels) {
    labels_ = labels;
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

void TabBar::select(int index) {
    if (index < 0 || index >= static_cast<int>(labels_.size())) return;
    if (selected_ == index) return;
    selected_ = index;
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
    if (callback_) callback_(index);
}

void TabBar::paint(HDC dc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);

    rects_.assign(labels_.size(), RECT{});
    int x = 0;
    SetBkMode(dc, TRANSPARENT);
    HFONT old = font_ != nullptr ? static_cast<HFONT>(SelectObject(dc, font_)) : nullptr;
    for (std::size_t i = 0; i < labels_.size(); ++i) {
        int text_w = width_of(dc, font_, labels_[i]);
        int tab_w = text_w + 36;
        RECT rect{x, 0, x + tab_w, client.bottom};
        rects_[i] = rect;
        bool active = static_cast<int>(i) == selected_;
        if (active) {
            HBRUSH bg = CreateSolidBrush(theme::BG);
            FillRect(dc, &rect, bg);
            DeleteObject(bg);
        } else {
            HBRUSH bg = CreateSolidBrush(theme::PANEL2);
            RECT upper{x + 6, 6, x + tab_w - 6, client.bottom - 4};
            FillRect(dc, &upper, bg);
            DeleteObject(bg);
        }
        SetTextColor(dc, active ? theme::ACCENT : theme::TEXT);
        DrawTextW(dc, labels_[i].c_str(), -1, &rect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        x += tab_w + 2;
    }
    if (old != nullptr) SelectObject(dc, old);
}

int TabBar::hit(int x, int y) const {
    (void)y;
    for (std::size_t i = 0; i < rects_.size(); ++i) {
        if (x >= rects_[i].left && x <= rects_[i].right) return static_cast<int>(i);
    }
    return -1;
}

LRESULT TabBar::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            paint(dc);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONUP: {
            int index = hit(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            if (index >= 0) select(index);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT CALLBACK TabBar::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    TabBar* bar = reinterpret_cast<TabBar*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        bar = static_cast<TabBar*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(bar));
    }
    if (bar == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    bar->hwnd_ = hwnd;
    return bar->handle_message(message, wparam, lparam);
}

OutputPane::OutputPane(HWND parent, HFONT header_font, HFONT text_font)
    : header_font_(header_font), text_font_(text_font), status_color_(theme::DIM) {
    ensure_richedit_loaded();
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = OutputPane::wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = OUTPUT_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    hwnd_ = CreateWindowExW(0, OUTPUT_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100, 100,
                            parent, nullptr, GetModuleHandleW(nullptr), this);
}

OutputPane::~OutputPane() {
    if (edit_ != nullptr) DestroyWindow(edit_);
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}

void OutputPane::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

void OutputPane::clear() {
    if (edit_ != nullptr) {
        SetWindowTextW(edit_, L"");
        has_text_ = false;
    }
    set_status("ready", theme::DIM);
}

void OutputPane::set_text_font(HFONT font) {
    text_font_ = font;
    if (edit_ != nullptr) {
        SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    }
}

void OutputPane::set_status(const std::string& message, COLORREF color) {
    status_ = wide_from_utf8(message);
    status_color_ = color;
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

void OutputPane::append_wide(const std::wstring& text, COLORREF color) {
    if (edit_ == nullptr) return;
    int length = GetWindowTextLengthW(edit_);
    SendMessageW(edit_, EM_SETSEL, length, length);
    CHARFORMAT2W format;
    std::memset(&format, 0, sizeof(format));
    format.cbSize = sizeof(format);
    format.dwMask = CFM_COLOR;
    format.crTextColor = color;
    SendMessageW(edit_, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    std::wstring insert = text;
    if (has_text_) insert = L"\r\n" + insert;
    std::wstring normalized;
    for (wchar_t c : insert) {
        if (c == L'\n') normalized += L'\r';
        else normalized += c;
    }
    SendMessageW(edit_, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(normalized.c_str()));
    SendMessageW(edit_, EM_SETSEL, length + static_cast<int>(normalized.size()),
                 length + static_cast<int>(normalized.size()));
    SendMessageW(edit_, EM_SCROLLCARET, 0, 0);
    has_text_ = true;
}

void OutputPane::append_text(const std::string& message, COLORREF color) {
    append_wide(wide_from_utf8(message), color);
}

void OutputPane::write(const std::string& message, const std::string& tag) {
    COLORREF color = theme::TEXT;
    if (tag == "err") color = theme::RED;
    else if (tag == "info") color = theme::DIM;
    else if (tag == "ok") color = theme::GREEN;
    else if (tag == "ooga") color = theme::RED;
    append_text(message, color);
}

void OutputPane::paint(HDC dc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);
    SetBkMode(dc, TRANSPARENT);

    HFONT old = header_font_ != nullptr
                    ? static_cast<HFONT>(SelectObject(dc, header_font_))
                    : nullptr;
    SetTextColor(dc, theme::ACCENT);
    RECT title{10, 0, 100, 26};
    DrawTextW(dc, L"OUTPUT", -1, &title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    if (old != nullptr) SelectObject(dc, old);

    HBRUSH button = CreateSolidBrush(theme::PANEL2);
    clear_rect_ = RECT{client.right - 70, 3, client.right - 8, 23};
    FillRect(dc, &clear_rect_, button);
    DeleteObject(button);
    HFONT small = static_cast<HFONT>(SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT)));
    SetTextColor(dc, theme::TEXT);
    RECT clear_text = clear_rect_;
    DrawTextW(dc, L"Clear", -1, &clear_text,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(dc, small);

    SetTextColor(dc, status_color_);
    RECT status_rect{client.right - 200, 0, client.right - 80, 26};
    DrawTextW(dc, status_.c_str(), -1, &status_rect,
              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

LRESULT OutputPane::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            edit_ = CreateWindowExW(
                0, MSFTEDIT_CLASS, L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL
                    | WS_CLIPSIBLINGS,
                0, 0, 100, 100, hwnd_, nullptr, GetModuleHandleW(nullptr), nullptr);
            SendMessageW(edit_, EM_SETBKGNDCOLOR, 0, theme::BG);
            SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(text_font_), TRUE);
            return 0;
        case WM_SIZE: {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);
            MoveWindow(edit_, 0, 28, width, height - 28, TRUE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            paint(dc);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            if (x >= clear_rect_.left && x <= clear_rect_.right && y >= clear_rect_.top
                && y <= clear_rect_.bottom) {
                clear();
            }
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT CALLBACK OutputPane::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    OutputPane* pane = reinterpret_cast<OutputPane*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        pane = static_cast<OutputPane*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pane));
    }
    if (pane == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    pane->hwnd_ = hwnd;
    return pane->handle_message(message, wparam, lparam);
}

StatusBar::StatusBar(HWND parent, HFONT font) : font_(font), message_color_(theme::TEXT) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = StatusBar::wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = STATUS_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    hwnd_ = CreateWindowExW(0, STATUS_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100, 100,
                            parent, nullptr, GetModuleHandleW(nullptr), this);
}

StatusBar::~StatusBar() {
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}

void StatusBar::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

void StatusBar::set_message(const std::string& message, COLORREF color) {
    message_ = wide_from_utf8(message);
    message_color_ = color;
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

void StatusBar::set_position(const std::string& message) {
    position_ = wide_from_utf8(message);
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

LRESULT StatusBar::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            RECT client;
            GetClientRect(hwnd_, &client);
            HBRUSH panel = CreateSolidBrush(theme::PANEL);
            FillRect(dc, &client, panel);
            DeleteObject(panel);
            SetBkMode(dc, TRANSPARENT);
            HFONT old = font_ != nullptr ? static_cast<HFONT>(SelectObject(dc, font_)) : nullptr;
            SetTextColor(dc, message_color_);
            RECT message_rect{10, 0, client.right - 260, client.bottom};
            DrawTextW(dc, message_.c_str(), -1, &message_rect,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            SetTextColor(dc, theme::DIM);
            RECT position_rect{client.right - 250, 0, client.right - 10, client.bottom};
            DrawTextW(dc, position_.c_str(), -1, &position_rect,
                      DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            if (old != nullptr) SelectObject(dc, old);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT CALLBACK StatusBar::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    StatusBar* bar = reinterpret_cast<StatusBar*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        bar = static_cast<StatusBar*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(bar));
    }
    if (bar == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    bar->hwnd_ = hwnd;
    return bar->handle_message(message, wparam, lparam);
}

}