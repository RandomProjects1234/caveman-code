#include "ide_drawing.hpp"

#include <cstring>

#include "ide_theme.hpp"

namespace og {

namespace {

const wchar_t* DRAW_CLASS = L"OgaboogaDrawPanel";
const UINT WM_DRAW_QUEUE = WM_APP + 1;
const int PAD = 16;

int round_int(double value) {
    return static_cast<int>(value >= 0 ? value + 0.5 : value - 0.5);
}

COLORREF color_of(const std::string& text) {
    int r = 0;
    int g = 0;
    int b = 0;
    cmc::parse_color_rgb(text, r, g, b);
    return RGB(r, g, b);
}

}

DrawingPanel::DrawingPanel(HWND parent) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = DrawingPanel::wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = DRAW_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    hwnd_ = CreateWindowExW(0, DRAW_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                            0, 0, 100, 100, parent, nullptr, GetModuleHandleW(nullptr), this);
}

DrawingPanel::~DrawingPanel() {
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
}

void DrawingPanel::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

void DrawingPanel::enqueue(Command command) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(command));
        used_ = true;
    }
    if (hwnd_ != nullptr) PostMessageW(hwnd_, WM_DRAW_QUEUE, 0, 0);
}

void DrawingPanel::drain() {
    std::vector<Command> incoming;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        incoming.swap(queue_);
    }
    for (auto& command : incoming) commands_.push_back(std::move(command));
}

void DrawingPanel::clear() {
    Command command;
    command.name = "clear";
    enqueue(std::move(command));
}

void DrawingPanel::size(int w, int h) {
    Command command;
    command.name = "size";
    command.a = w;
    command.b = h;
    enqueue(std::move(command));
}

void DrawingPanel::color(const std::string& c) {
    Command command;
    command.name = "color";
    command.text = c;
    enqueue(std::move(command));
}

void DrawingPanel::dot(double x, double y, double r) {
    Command command;
    command.name = "dot";
    command.a = x;
    command.b = y;
    command.c = r;
    enqueue(std::move(command));
}

void DrawingPanel::circle(double x, double y, double r) {
    Command command;
    command.name = "circle";
    command.a = x;
    command.b = y;
    command.c = r;
    enqueue(std::move(command));
}

void DrawingPanel::line(double x1, double y1, double x2, double y2) {
    Command command;
    command.name = "line";
    command.a = x1;
    command.b = y1;
    command.c = x2;
    command.d = y2;
    enqueue(std::move(command));
}

void DrawingPanel::box(double x, double y, double w, double h) {
    Command command;
    command.name = "box";
    command.a = x;
    command.b = y;
    command.c = w;
    command.d = h;
    enqueue(std::move(command));
}

void DrawingPanel::blob(double x, double y, double w, double h) {
    Command command;
    command.name = "blob";
    command.a = x;
    command.b = y;
    command.c = w;
    command.d = h;
    enqueue(std::move(command));
}

void DrawingPanel::write(const std::string& text, double x, double y) {
    Command command;
    command.name = "write";
    command.a = x;
    command.b = y;
    command.text = text;
    enqueue(std::move(command));
}

void DrawingPanel::wipe() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }
    commands_.clear();
    color_ = "black";
    if (hwnd_ != nullptr) InvalidateRect(hwnd_, nullptr, TRUE);
}

void DrawingPanel::paint(HDC dc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);

    RECT picture{0, 0, PAD * 2 + width_, PAD * 2 + height_};
    HBRUSH white = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    FillRect(dc, &picture, white);

    HRGN clip = CreateRectRgn(PAD, PAD, PAD + width_, PAD + height_);
    SelectClipRgn(dc, clip);

    COLORREF color = RGB(0, 0, 0);
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    HFONT old_font = static_cast<HFONT>(SelectObject(dc, font));
    int saved = SaveDC(dc);
    SetViewportOrgEx(dc, PAD, PAD, nullptr);

    for (const auto& command : commands_) {
        const std::string& name = command.name;
        if (name == "clear") {
            RECT area{0, 0, width_, height_};
            HBRUSH brush = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
            FillRect(dc, &area, brush);
        } else if (name == "size") {
            width_ = static_cast<int>(command.a);
            height_ = static_cast<int>(command.b);
            if (width_ < 1) width_ = 1;
            if (height_ < 1) height_ = 1;
            RECT area{0, 0, width_, height_};
            HBRUSH brush = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
            FillRect(dc, &area, brush);
            InvalidateRect(hwnd_, nullptr, TRUE);
        } else if (name == "color") {
            color = color_of(command.text);
        } else if (name == "dot" || name == "circle") {
            int x = round_int(command.a) - round_int(command.c);
            int y = round_int(command.b) - round_int(command.c);
            int w = round_int(command.c) * 2;
            int h = round_int(command.c) * 2;
            if (name == "dot") {
                HBRUSH brush = CreateSolidBrush(color);
                HPEN pen = CreatePen(PS_SOLID, 1, color);
                HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
                HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
                Ellipse(dc, x, y, x + w, y + h);
                SelectObject(dc, old_brush);
                SelectObject(dc, old_pen);
                DeleteObject(brush);
                DeleteObject(pen);
            } else {
                HPEN pen = CreatePen(PS_SOLID, 2, color);
                HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
                HBRUSH old_brush =
                    static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
                Ellipse(dc, x, y, x + w, y + h);
                SelectObject(dc, old_pen);
                SelectObject(dc, old_brush);
                DeleteObject(pen);
            }
        } else if (name == "line") {
            HPEN pen = CreatePen(PS_SOLID, 2, color);
            HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
            MoveToEx(dc, round_int(command.a), round_int(command.b), nullptr);
            LineTo(dc, round_int(command.c), round_int(command.d));
            SelectObject(dc, old_pen);
            DeleteObject(pen);
        } else if (name == "box" || name == "blob") {
            int x = round_int(command.a);
            int y = round_int(command.b);
            int w = round_int(command.c);
            int h = round_int(command.d);
            if (name == "box") {
                HPEN pen = CreatePen(PS_SOLID, 2, color);
                HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
                HBRUSH old_brush =
                    static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
                Rectangle(dc, x, y, x + w, y + h);
                SelectObject(dc, old_pen);
                SelectObject(dc, old_brush);
                DeleteObject(pen);
            } else {
                HBRUSH brush = CreateSolidBrush(color);
                HPEN pen = CreatePen(PS_SOLID, 1, color);
                HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
                HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
                Rectangle(dc, x, y, x + w, y + h);
                SelectObject(dc, old_brush);
                SelectObject(dc, old_pen);
                DeleteObject(brush);
                DeleteObject(pen);
            }
        } else if (name == "write") {
            std::wstring wide;
            int needed = MultiByteToWideChar(CP_UTF8, 0, command.text.c_str(),
                                             static_cast<int>(command.text.size()), nullptr, 0);
            wide.resize(static_cast<std::size_t>(needed));
            MultiByteToWideChar(CP_UTF8, 0, command.text.c_str(),
                                static_cast<int>(command.text.size()), wide.data(), needed);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, color);
            TextOutW(dc, round_int(command.a), round_int(command.b), wide.c_str(),
                     static_cast<int>(wide.size()));
        }
    }

    RestoreDC(dc, saved);
    SelectObject(dc, old_font);
    SelectClipRgn(dc, nullptr);
    DeleteObject(clip);

    HPEN border = CreatePen(PS_SOLID, 2, theme::PANEL3);
    HPEN old_pen = static_cast<HPEN>(SelectObject(dc, border));
    HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, GetStockObject(NULL_BRUSH)));
    Rectangle(dc, PAD - 1, PAD - 1, PAD + width_ + 1, PAD + height_ + 1);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(border);
}

LRESULT DrawingPanel::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_DRAW_QUEUE:
            drain();
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd_, &ps);
            HDC mem = CreateCompatibleDC(dc);
            RECT client;
            GetClientRect(hwnd_, &client);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HBITMAP old_bitmap = static_cast<HBITMAP>(SelectObject(mem, bitmap));
            paint(mem);
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old_bitmap);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(hwnd_, &ps);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

LRESULT CALLBACK DrawingPanel::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    DrawingPanel* panel =
        reinterpret_cast<DrawingPanel*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        panel = static_cast<DrawingPanel*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(panel));
    }
    if (panel == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    panel->hwnd_ = hwnd;
    return panel->handle_message(message, wparam, lparam);
}

}