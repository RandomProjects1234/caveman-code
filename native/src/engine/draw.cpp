#include "draw.hpp"

#include <unordered_map>

namespace cmc {

namespace {

int hex_digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

struct ColorName {
    const char* name;
    int r;
    int g;
    int b;
};

const ColorName COLOR_NAMES[] = {
    {"black", 0, 0, 0},          {"white", 255, 255, 255},   {"red", 255, 0, 0},
    {"green", 0, 128, 0},        {"blue", 0, 0, 255},        {"yellow", 255, 255, 0},
    {"gold", 255, 215, 0},       {"orange", 255, 165, 0},    {"purple", 128, 0, 128},
    {"pink", 255, 192, 203},     {"brown", 165, 42, 42},     {"gray", 128, 128, 128},
    {"grey", 128, 128, 128},     {"cyan", 0, 255, 255},      {"magenta", 255, 0, 255},
    {"lime", 0, 255, 0},         {"navy", 0, 0, 128},        {"teal", 0, 128, 128},
    {"maroon", 128, 0, 0},       {"olive", 128, 128, 0},     {"silver", 192, 192, 192},
    {"aqua", 0, 255, 255},       {"fuchsia", 255, 0, 255},   {"violet", 238, 130, 238},
    {"indigo", 75, 0, 130},      {"tan", 210, 180, 140},     {"salmon", 250, 128, 114},
    {"coral", 255, 127, 80},     {"crimson", 220, 20, 60},   {"khaki", 240, 230, 140},
    {"lavender", 230, 230, 250}, {"beige", 245, 245, 220},   {"turquoise", 64, 224, 208},
    {"chocolate", 210, 105, 30}, {"tomato", 255, 99, 71},    {"orchid", 218, 112, 214},
    {"plum", 221, 160, 221},     {"skyblue", 135, 206, 235}, {"steelblue", 70, 130, 180},
    {"forestgreen", 34, 139, 34}, {"seagreen", 46, 139, 87}, {"darkred", 139, 0, 0},
    {"darkgreen", 0, 100, 0},    {"darkblue", 0, 0, 139},    {"lightblue", 173, 216, 230},
    {"lightgreen", 144, 238, 144}, {"lightgray", 211, 211, 211},
    {"lightgrey", 211, 211, 211}, {"darkgray", 169, 169, 169},
    {"darkgrey", 169, 169, 169},
};

}

bool parse_color_rgb(const std::string& text, int& r, int& g, int& b) {
    r = 0;
    g = 0;
    b = 0;
    if (!text.empty() && text[0] == '#') {
        std::string hex = text.substr(1);
        if (hex.size() == 3) {
            int r1 = hex_digit_value(hex[0]);
            int g1 = hex_digit_value(hex[1]);
            int b1 = hex_digit_value(hex[2]);
            if (r1 >= 0 && g1 >= 0 && b1 >= 0) {
                r = r1 * 17;
                g = g1 * 17;
                b = b1 * 17;
                return true;
            }
        } else if (hex.size() == 6) {
            int r1 = hex_digit_value(hex[0]);
            int r2 = hex_digit_value(hex[1]);
            int g1 = hex_digit_value(hex[2]);
            int g2 = hex_digit_value(hex[3]);
            int b1 = hex_digit_value(hex[4]);
            int b2 = hex_digit_value(hex[5]);
            if (r1 >= 0 && r2 >= 0 && g1 >= 0 && g2 >= 0 && b1 >= 0 && b2 >= 0) {
                r = r1 * 16 + r2;
                g = g1 * 16 + g2;
                b = b1 * 16 + b2;
                return true;
            }
        }
        return false;
    }
    std::string lower = text;
    for (char& c : lower) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    for (const auto& entry : COLOR_NAMES) {
        if (lower == entry.name) {
            r = entry.r;
            g = entry.g;
            b = entry.b;
            return true;
        }
    }
    return false;
}

}

#ifdef _WIN32

#include <windows.h>

#include <cstring>
#include <iostream>

namespace cmc {

namespace {

std::wstring to_wide(const std::string& text) {
    if (text.empty()) return L"";
    int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                     nullptr, 0);
    std::wstring out(static_cast<std::size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(),
                        needed);
    return out;
}

COLORREF parse_color(const std::string& text) {
    int r = 0;
    int g = 0;
    int b = 0;
    parse_color_rgb(text, r, g, b);
    return RGB(r, g, b);
}

int round_int(double value) {
    return static_cast<int>(value >= 0 ? value + 0.5 : value - 0.5);
}

}

struct Win32Draw::Impl {
    HWND hwnd = nullptr;
    HDC mem_dc = nullptr;
    HBITMAP bitmap = nullptr;
    HBITMAP old_bitmap = nullptr;
    int width = 400;
    int height = 400;
    COLORREF color = RGB(0, 0, 0);
    std::wstring title;
    bool registered = false;

    void fill_white() {
        if (mem_dc == nullptr) return;
        RECT rect{0, 0, width, height};
        HBRUSH brush = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        FillRect(mem_dc, &rect, brush);
    }

    void create_buffer() {
        HDC screen = GetDC(nullptr);
        HDC new_dc = CreateCompatibleDC(screen);
        HBITMAP new_bitmap = CreateCompatibleBitmap(screen, width, height);
        ReleaseDC(nullptr, screen);
        if (old_bitmap != nullptr) SelectObject(mem_dc, old_bitmap);
        if (bitmap != nullptr) DeleteObject(bitmap);
        if (mem_dc != nullptr) DeleteDC(mem_dc);
        mem_dc = new_dc;
        bitmap = new_bitmap;
        old_bitmap = static_cast<HBITMAP>(SelectObject(mem_dc, bitmap));
        fill_white();
    }
};

static LRESULT CALLBACK cmc_wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    Win32Draw::Impl* impl =
        reinterpret_cast<Win32Draw::Impl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (message) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            if (impl != nullptr && impl->mem_dc != nullptr) {
                BitBlt(dc, 0, 0, impl->width, impl->height, impl->mem_dc, 0, 0, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (impl != nullptr) impl->hwnd = nullptr;
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

Win32Draw::Win32Draw(const std::string& title) : impl_(new Impl()) {
    impl_->title = to_wide(title);
}

Win32Draw::~Win32Draw() {
    if (impl_->old_bitmap != nullptr && impl_->mem_dc != nullptr) {
        SelectObject(impl_->mem_dc, impl_->old_bitmap);
    }
    if (impl_->bitmap != nullptr) DeleteObject(impl_->bitmap);
    if (impl_->mem_dc != nullptr) DeleteDC(impl_->mem_dc);
    if (impl_->hwnd != nullptr) DestroyWindow(impl_->hwnd);
    delete impl_;
}

void Win32Draw::pump() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32Draw::ensure() {
    if (impl_->hwnd != nullptr) return;

    if (!impl_->registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = cmc_wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wc.lpszClassName = L"CmcDrawWindow";
        RegisterClassW(&wc);
        impl_->registered = true;
    }

    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect{0, 0, impl_->width, impl_->height};
    AdjustWindowRect(&rect, style, FALSE);
    HWND hwnd = CreateWindowExW(
        0, L"CmcDrawWindow", impl_->title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
        GetModuleHandleW(nullptr), nullptr);
    if (hwnd == nullptr) return;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(impl_));
    impl_->hwnd = hwnd;
    impl_->create_buffer();
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    pump();
}

void Win32Draw::repaint() {
    if (impl_->hwnd == nullptr) return;
    InvalidateRect(impl_->hwnd, nullptr, FALSE);
    UpdateWindow(impl_->hwnd);
    pump();
}

void Win32Draw::clear() {
    ensure();
    impl_->fill_white();
    repaint();
}

void Win32Draw::size(int w, int h) {
    impl_->width = w > 0 ? w : 1;
    impl_->height = h > 0 ? h : 1;
    ensure();
    if (impl_->hwnd == nullptr) return;
    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect{0, 0, impl_->width, impl_->height};
    AdjustWindowRect(&rect, style, FALSE);
    SetWindowPos(impl_->hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOREDRAW | SWP_NOMOVE);
    impl_->create_buffer();
    repaint();
}

void Win32Draw::color(const std::string& c) {
    impl_->color = parse_color(c);
    repaint();
}

void Win32Draw::dot(double x, double y, double r) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    HBRUSH brush = CreateSolidBrush(impl_->color);
    HPEN pen = CreatePen(PS_SOLID, 1, impl_->color);
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(impl_->mem_dc, brush));
    HPEN old_pen = static_cast<HPEN>(SelectObject(impl_->mem_dc, pen));
    Ellipse(impl_->mem_dc, round_int(x - r), round_int(y - r), round_int(x + r), round_int(y + r));
    SelectObject(impl_->mem_dc, old_brush);
    SelectObject(impl_->mem_dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
    repaint();
}

void Win32Draw::circle(double x, double y, double r) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    HPEN pen = CreatePen(PS_SOLID, 2, impl_->color);
    HPEN old_pen = static_cast<HPEN>(SelectObject(impl_->mem_dc, pen));
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(impl_->mem_dc, GetStockObject(NULL_BRUSH)));
    Ellipse(impl_->mem_dc, round_int(x - r), round_int(y - r), round_int(x + r), round_int(y + r));
    SelectObject(impl_->mem_dc, old_pen);
    SelectObject(impl_->mem_dc, old_brush);
    DeleteObject(pen);
    repaint();
}

void Win32Draw::line(double x1, double y1, double x2, double y2) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    HPEN pen = CreatePen(PS_SOLID, 2, impl_->color);
    HPEN old_pen = static_cast<HPEN>(SelectObject(impl_->mem_dc, pen));
    MoveToEx(impl_->mem_dc, round_int(x1), round_int(y1), nullptr);
    LineTo(impl_->mem_dc, round_int(x2), round_int(y2));
    SelectObject(impl_->mem_dc, old_pen);
    DeleteObject(pen);
    repaint();
}

void Win32Draw::box(double x, double y, double w, double h) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    HPEN pen = CreatePen(PS_SOLID, 2, impl_->color);
    HPEN old_pen = static_cast<HPEN>(SelectObject(impl_->mem_dc, pen));
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(impl_->mem_dc, GetStockObject(NULL_BRUSH)));
    Rectangle(impl_->mem_dc, round_int(x), round_int(y), round_int(x + w), round_int(y + h));
    SelectObject(impl_->mem_dc, old_pen);
    SelectObject(impl_->mem_dc, old_brush);
    DeleteObject(pen);
    repaint();
}

void Win32Draw::blob(double x, double y, double w, double h) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    HBRUSH brush = CreateSolidBrush(impl_->color);
    HPEN pen = CreatePen(PS_SOLID, 1, impl_->color);
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(impl_->mem_dc, brush));
    HPEN old_pen = static_cast<HPEN>(SelectObject(impl_->mem_dc, pen));
    Rectangle(impl_->mem_dc, round_int(x), round_int(y), round_int(x + w), round_int(y + h));
    SelectObject(impl_->mem_dc, old_brush);
    SelectObject(impl_->mem_dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
    repaint();
}

void Win32Draw::write(const std::string& text, double x, double y) {
    ensure();
    if (impl_->mem_dc == nullptr) return;
    std::wstring wide = to_wide(text);
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    HFONT old_font = static_cast<HFONT>(SelectObject(impl_->mem_dc, font));
    SetBkMode(impl_->mem_dc, TRANSPARENT);
    SetTextColor(impl_->mem_dc, impl_->color);
    TextOutW(impl_->mem_dc, round_int(x), round_int(y), wide.c_str(),
             static_cast<int>(wide.size()));
    SelectObject(impl_->mem_dc, old_font);
    repaint();
}

bool Win32Draw::used() const {
    return impl_->hwnd != nullptr;
}

void Win32Draw::hold_open() {
    if (impl_->hwnd == nullptr) return;
    std::cout << "Open picture window! Close it when you are done looking." << std::endl;
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

}

#endif
