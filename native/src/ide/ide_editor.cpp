#include "ide_editor.hpp"

#include <commctrl.h>
#include <richedit.h>

#include <algorithm>
#include <cstring>
#include <set>

#include "ide_theme.hpp"
#include "words_data.hpp"

namespace og {

namespace {

const wchar_t* EDITOR_CLASS = L"OgaboogaEditor";
const wchar_t* GUTTER_CLASS = L"OgaboogaGutter";
const int GUTTER_PAD = 6;
const wchar_t* OPENERS[] = {L"binga", L"booga", L"zug",   L"zoop", L"clump", L"if",
                            L"repeat", L"while", L"for", L"function", L"fn", L"def"};

struct Range {
    int start;
    int end;
    int type;
};

enum RangeType { RANGE_COMMENT, RANGE_KEYWORD, RANGE_BUILTIN, RANGE_CONSTANT,
                 RANGE_STRING, RANGE_NUMBER, RANGE_OPERATOR };

bool is_word_char(wchar_t c) {
    return (c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z')
           || c == L'_';
}

bool is_digit(wchar_t c) {
    return c >= L'0' && c <= L'9';
}

std::wstring lower_of(const std::wstring& text) {
    std::wstring out = text;
    for (wchar_t& c : out) {
        if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
    }
    return out;
}

const std::set<std::wstring>& keyword_set() {
    static const std::set<std::wstring> table = [] {
        std::set<std::wstring> names;
        for (const auto& word : cmc::words::word_infos()) {
            if (word.is_builtin) continue;
            if (word.name == "plop") continue;
            names.insert(wide_from_utf8(word.name));
            for (const auto& alias : word.aliases) names.insert(wide_from_utf8(alias));
        }
        names.insert(L"and");
        names.insert(L"or");
        names.insert(L"not");
        names.insert(L"in");
        return names;
    }();
    return table;
}

const std::set<std::wstring>& builtin_set() {
    static const std::set<std::wstring> table = [] {
        std::set<std::wstring> names;
        for (const auto& word : cmc::words::word_infos()) {
            if (!word.is_builtin) continue;
            if (word.name == "plop_call") continue;
            names.insert(wide_from_utf8(word.name));
        }
        names.insert(L"snorf");
        names.insert(L"plop");
        return names;
    }();
    return table;
}

const std::set<std::wstring>& constant_set() {
    static const std::set<std::wstring> table = {
        L"gronk", L"nork", L"plop", L"pi", L"true", L"false", L"nothing",
    };
    return table;
}

std::vector<Range> scan_highlights(const std::wstring& text) {
    std::vector<Range> ranges;
    const std::size_t length = text.size();
    std::size_t i = 0;

    for (i = 0; i < length; ++i) {
        if (!is_word_char(text[i])) continue;
        std::size_t end = i;
        while (end < length && is_word_char(text[end])) ++end;
        std::wstring word = lower_of(text.substr(i, end - i));
        if (word == L"ugg") {
            std::size_t line_end = end;
            while (line_end < length && text[line_end] != L'\n' && text[line_end] != L'\r') {
                ++line_end;
            }
            ranges.push_back({static_cast<int>(i), static_cast<int>(line_end), RANGE_COMMENT});
        }
        i = end - 1;
    }

    for (i = 0; i < length; ++i) {
        wchar_t c = text[i];
        if (c == L' ' || c == L'\t') continue;
        std::size_t end = i;
        while (end < length && is_word_char(text[end])) ++end;
        if (end == i) continue;
        std::wstring word = lower_of(text.substr(i, end - i));
        if (keyword_set().count(word)) {
            ranges.push_back({static_cast<int>(i), static_cast<int>(end), RANGE_KEYWORD});
        } else if (builtin_set().count(word)) {
            ranges.push_back({static_cast<int>(i), static_cast<int>(end), RANGE_BUILTIN});
        } else if (constant_set().count(word)) {
            ranges.push_back({static_cast<int>(i), static_cast<int>(end), RANGE_CONSTANT});
        }
        i = end - 1;
    }

    for (i = 0; i < length; ++i) {
        if (is_digit(text[i]) && (i == 0 || !is_word_char(text[i - 1]))) {
            std::size_t end = i;
            while (end < length && is_digit(text[end])) ++end;
            if (end < length && text[end] == L'.' && end + 1 < length && is_digit(text[end + 1])) {
                ++end;
                while (end < length && is_digit(text[end])) ++end;
            }
            ranges.push_back({static_cast<int>(i), static_cast<int>(end), RANGE_NUMBER});
            i = end - 1;
        }
    }

    for (i = 0; i < length; ++i) {
        wchar_t c = text[i];
        if (c != L'"' && c != L'\'') continue;
        wchar_t quote = c;
        std::size_t end = i + 1;
        while (end < length && text[end] != quote && text[end] != L'\n' && text[end] != L'\r') {
            if (text[end] == L'\\' && end + 1 < length) ++end;
            ++end;
        }
        if (end < length && text[end] == quote) ++end;
        ranges.push_back({static_cast<int>(i), static_cast<int>(end), RANGE_STRING});
        i = end - 1;
    }

    for (i = 0; i < length; ++i) {
        wchar_t c = text[i];
        if (c == L'=' || c == L'!' || c == L'<' || c == L'>') {
            if (i + 1 < length && text[i + 1] == L'=') {
                ranges.push_back({static_cast<int>(i), static_cast<int>(i + 2), RANGE_OPERATOR});
                ++i;
                continue;
            }
        }
        if (c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'%' || c == L'<'
            || c == L'>' || c == L'=' || c == L'[' || c == L']' || c == L'(' || c == L')') {
            ranges.push_back({static_cast<int>(i), static_cast<int>(i + 1), RANGE_OPERATOR});
        }
    }

    std::stable_sort(ranges.begin(), ranges.end(),
                     [](const Range& a, const Range& b) { return a.type < b.type; });
    return ranges;
}

}

void ensure_richedit_loaded() {
    static bool loaded = false;
    if (!loaded) {
        LoadLibraryW(L"Msftedit.dll");
        loaded = true;
    }
}

HFONT make_font(const wchar_t* face, int point_size, bool bold) {
    HDC dc = GetDC(nullptr);
    int dpi = GetDeviceCaps(dc, LOGPIXELSY);
    ReleaseDC(nullptr, dc);
    LOGFONTW lf;
    std::memset(&lf, 0, sizeof(lf));
    lf.lfHeight = -MulDiv(point_size, dpi, 72);
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy(lf.lfFaceName, face, LF_FACESIZE - 1);
    return CreateFontIndirectW(&lf);
}

std::wstring wide_from_utf8(const std::string& text) {
    if (text.empty()) return L"";
    int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                     nullptr, 0);
    std::wstring wide(static_cast<std::size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), wide.data(),
                        needed);
    return wide;
}

std::string utf8_from_wide(const std::wstring& text) {
    if (text.empty()) return "";
    int needed = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(needed), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(),
                        needed, nullptr, nullptr);
    return out;
}

std::vector<std::wstring> highlight_keywords() {
    return std::vector<std::wstring>(keyword_set().begin(), keyword_set().end());
}

std::vector<std::wstring> highlight_builtins() {
    return std::vector<std::wstring>(builtin_set().begin(), builtin_set().end());
}

std::vector<std::wstring> highlight_constants() {
    return std::vector<std::wstring>(constant_set().begin(), constant_set().end());
}

void set_rich_text(HWND edit, const std::wstring& text) {
    if (edit == nullptr) return;
    CHARRANGE all;
    all.cpMin = 0;
    all.cpMax = -1;
    SendMessageW(edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&all));
    SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
    CHARRANGE none;
    none.cpMin = 0;
    none.cpMax = 0;
    SendMessageW(edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&none));
}

void apply_highlight(HWND rich_edit, bool bold_keywords) {
    if (rich_edit == nullptr) return;
    int length = GetWindowTextLengthW(rich_edit);
    if (length <= 0) return;
    std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
    GetWindowTextW(rich_edit, text.data(), length + 1);
    text.resize(static_cast<std::size_t>(length));
    if (text.find_first_not_of(L" \t\r\n") == std::wstring::npos) return;

    std::vector<Range> ranges = scan_highlights(text);
    if (ranges.empty()) return;

    CHARRANGE saved;
    SendMessageW(rich_edit, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&saved));
    SendMessageW(rich_edit, WM_SETREDRAW, FALSE, 0);

    COLORREF colors[] = {
        theme::COMMENT, theme::ACCENT, theme::BLUE,   theme::PURPLE,
        theme::GREEN,   theme::PURPLE, theme::DIM,
    };

    for (const auto& range : ranges) {
        SendMessageW(rich_edit, EM_SETSEL, range.start, range.end);
        CHARFORMAT2W format;
        std::memset(&format, 0, sizeof(format));
        format.cbSize = sizeof(format);
        format.dwMask = CFM_COLOR;
        format.crTextColor = colors[range.type];
        if (range.type == RANGE_KEYWORD && bold_keywords) {
            format.dwMask |= CFM_BOLD;
            format.dwEffects |= CFE_BOLD;
        }
        SendMessageW(rich_edit, EM_SETCHARFORMAT, SCF_SELECTION,
                     reinterpret_cast<LPARAM>(&format));
    }

    SendMessageW(rich_edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&saved));
    SendMessageW(rich_edit, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(rich_edit, nullptr, TRUE);
}

CodeEditor::CodeEditor(HWND parent, HFONT font) : font_(font) {
    ensure_richedit_loaded();
    static bool classes_registered = false;
    if (!classes_registered) {
        WNDCLASSW editor_class;
        std::memset(&editor_class, 0, sizeof(editor_class));
        editor_class.lpfnWndProc = CodeEditor::container_proc;
        editor_class.hInstance = GetModuleHandleW(nullptr);
        editor_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        editor_class.hbrBackground = nullptr;
        editor_class.lpszClassName = EDITOR_CLASS;
        RegisterClassW(&editor_class);

        WNDCLASSW gutter_class;
        std::memset(&gutter_class, 0, sizeof(gutter_class));
        gutter_class.lpfnWndProc = CodeEditor::gutter_proc;
        gutter_class.hInstance = GetModuleHandleW(nullptr);
        gutter_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        gutter_class.hbrBackground = nullptr;
        gutter_class.lpszClassName = GUTTER_CLASS;
        RegisterClassW(&gutter_class);
        classes_registered = true;
    }

    hwnd_ = CreateWindowExW(0, EDITOR_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                            0, 0, 100, 100, parent, nullptr, GetModuleHandleW(nullptr), this);
}

LRESULT CALLBACK CodeEditor::container_proc(HWND hwnd, UINT message, WPARAM wparam,
                                            LPARAM lparam) {
    CodeEditor* editor = reinterpret_cast<CodeEditor*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        editor = static_cast<CodeEditor*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(editor));
    }
    if (editor == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    editor->hwnd_ = hwnd;
    return editor->handle_message(message, wparam, lparam);
}

LRESULT CALLBACK CodeEditor::gutter_proc(HWND hwnd, UINT message, WPARAM wparam,
                                         LPARAM lparam) {
    CodeEditor* editor = reinterpret_cast<CodeEditor*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        editor = static_cast<CodeEditor*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(editor));
    }
    if (editor == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    editor->gutter_ = hwnd;
    return editor->gutter_message(message, wparam, lparam);
}

LRESULT CALLBACK CodeEditor::edit_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                                       UINT_PTR subclass_id, DWORD_PTR ref_data) {
    (void)subclass_id;
    (void)ref_data;
    if (message == WM_KEYDOWN) {
        CodeEditor* editor = reinterpret_cast<CodeEditor*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (editor != nullptr) {
            if (wparam == VK_TAB) {
                editor->handle_tab();
                return 0;
            }
            if (wparam == VK_RETURN) {
                editor->handle_return();
                return 0;
            }
        }
    }
    return DefSubclassProc(hwnd, message, wparam, lparam);
}

CodeEditor::~CodeEditor() {
    if (edit_ != nullptr) DestroyWindow(edit_);
    if (gutter_ != nullptr) DestroyWindow(gutter_);
    if (hwnd_ != nullptr) DestroyWindow(hwnd_);
    if (font_ != nullptr) DeleteObject(font_);
}

void CodeEditor::layout(int x, int y, int w, int h) {
    if (hwnd_ != nullptr) MoveWindow(hwnd_, x, y, w, h, TRUE);
}

LRESULT CodeEditor::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE: {
            gutter_ = CreateWindowExW(0, GUTTER_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 40,
                                      100, hwnd_, nullptr, GetModuleHandleW(nullptr), this);
            edit_ = CreateWindowExW(
                0, MSFTEDIT_CLASS, L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL
                    | ES_NOHIDESEL | ES_WANTRETURN,
                0, 0, 100, 100, hwnd_, nullptr, GetModuleHandleW(nullptr), nullptr);
            SetWindowLongPtrW(edit_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            SetWindowSubclass(edit_, CodeEditor::edit_proc, 1, 0);
            SendMessageW(edit_, EM_SETBKGNDCOLOR, 0, theme::BG);
            SendMessageW(edit_, EM_SETLIMITTEXT, 0x7FFFFFFE, 0);
            return 0;
        }
        case WM_SIZE: {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);
            MoveWindow(gutter_, 0, 0, gutter_width_, height, TRUE);
            MoveWindow(edit_, gutter_width_, 0, width - gutter_width_, height, TRUE);
            return 0;
        }
        case WM_COMMAND: {
            int code = HIWORD(wparam);
            if (reinterpret_cast<HWND>(lparam) == edit_) {
                if (code == EN_CHANGE) {
                    if (!loading_) {
                        refresh();
                        if (on_change_) on_change_();
                    }
                } else if (code == EN_SELCHANGE) {
                    if (on_position_) on_position_();
                } else if (code == EN_VSCROLL) {
                    update_gutter();
                    InvalidateRect(gutter_, nullptr, TRUE);
                }
            }
            return 0;
        }
        case WM_NOTIFY: {
            NMHDR* header = reinterpret_cast<NMHDR*>(lparam);
            if (header != nullptr && header->hwndFrom == edit_) {
                if (header->code == EN_VSCROLL) {
                    update_gutter();
                    InvalidateRect(gutter_, nullptr, TRUE);
                }
            }
            return 0;
        }
        case WM_SETFOCUS:
            if (edit_ != nullptr) SetFocus(edit_);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        default:
            break;
    }
    return DefWindowProcW(hwnd_, message, wparam, lparam);
}

void CodeEditor::refresh() {
    apply_highlight(edit_, true);
    update_gutter();
    InvalidateRect(gutter_, nullptr, TRUE);
}

void CodeEditor::update_gutter() {
    if (edit_ == nullptr || gutter_ == nullptr) return;
    int line_count = static_cast<int>(SendMessageW(edit_, EM_GETLINECOUNT, 0, 0));
    if (line_count < 1) line_count = 1;
    int width = 3;
    int digits = 1;
    for (int n = line_count; n >= 10; n /= 10) ++digits;
    if (digits > width) width = digits;
    int gutter_width = 16 + width * 8;
    if (gutter_width < 34) gutter_width = 34;
    RECT client;
    GetClientRect(hwnd_, &client);
    if (gutter_width != gutter_width_) {
        gutter_width_ = gutter_width;
        MoveWindow(gutter_, 0, 0, gutter_width_, client.bottom, TRUE);
        MoveWindow(edit_, gutter_width_, 0, client.right - gutter_width_, client.bottom, TRUE);
    }
}

void CodeEditor::paint_gutter(HDC dc) {
    RECT client;
    GetClientRect(gutter_, &client);
    HBRUSH panel = CreateSolidBrush(theme::PANEL);
    FillRect(dc, &client, panel);
    DeleteObject(panel);

    int line_count = static_cast<int>(SendMessageW(edit_, EM_GETLINECOUNT, 0, 0));
    int first_visible = static_cast<int>(SendMessageW(edit_, EM_GETFIRSTVISIBLELINE, 0, 0));
    int line_height = 16;
    HFONT old_font = nullptr;
    if (font_ != nullptr) {
        old_font = static_cast<HFONT>(SelectObject(dc, font_));
        TEXTMETRICW metrics;
        if (GetTextMetricsW(dc, &metrics)) {
            line_height = metrics.tmHeight + metrics.tmExternalLeading;
        }
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, theme::DIM);
    int y = 2;
    for (int line = first_visible; line < line_count && y < client.bottom;
         ++line, y += line_height) {
        std::wstring number = std::to_wstring(line + 1);
        RECT row{0, y, client.right - 6, y + line_height};
        DrawTextW(dc, number.c_str(), static_cast<int>(number.size()), &row,
                  DT_RIGHT | DT_TOP | DT_SINGLELINE);
    }
    if (old_font != nullptr) SelectObject(dc, old_font);
}

LRESULT CodeEditor::gutter_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(gutter_, &ps);
            paint_gutter(dc);
            EndPaint(gutter_, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONDOWN:
            if (edit_ != nullptr) SetFocus(edit_);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(gutter_, message, wparam, lparam);
}

void CodeEditor::handle_tab() {
    SendMessageW(edit_, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(L"    "));
}

void CodeEditor::handle_return() {
    CHARRANGE selection;
    SendMessageW(edit_, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    int line = static_cast<int>(SendMessageW(edit_, EM_LINEFROMCHAR, selection.cpMin, 0));
    int line_start = static_cast<int>(SendMessageW(edit_, EM_LINEINDEX, line, 0));
    int line_length = static_cast<int>(SendMessageW(edit_, EM_LINELENGTH, selection.cpMin, 0));
    std::wstring line_text(static_cast<std::size_t>(line_length) + 1, L'\0');
    *reinterpret_cast<WORD*>(line_text.data()) = static_cast<WORD>(line_length + 1);
    SendMessageW(edit_, EM_GETLINE, line, reinterpret_cast<LPARAM>(line_text.data()));
    line_text.resize(static_cast<std::size_t>(line_length));

    int before_length = selection.cpMin - line_start;
    if (before_length < 0) before_length = 0;
    if (before_length > static_cast<int>(line_text.size())) {
        before_length = static_cast<int>(line_text.size());
    }
    std::wstring before = line_text.substr(0, static_cast<std::size_t>(before_length));

    std::wstring indent;
    for (wchar_t c : before) {
        if (c == L' ') indent += c;
        else break;
    }
    std::size_t start = before.find_first_not_of(L" \t");
    std::wstring word;
    if (start != std::wstring::npos) {
        std::size_t end = start;
        while (end < before.size() && before[end] != L' ' && before[end] != L'\t') ++end;
        word = lower_of(before.substr(start, end - start));
    }
    for (const wchar_t* opener : OPENERS) {
        if (word == opener) {
            indent += L"    ";
            break;
        }
    }
    std::wstring insert = L"\r\n" + indent;
    SendMessageW(edit_, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(insert.c_str()));
}

std::string CodeEditor::get_text() const {
    if (edit_ == nullptr) return "";
    int length = GetWindowTextLengthW(edit_);
    if (length <= 0) return "";
    std::wstring text(static_cast<std::size_t>(length) + 1, L'\0');
    GetWindowTextW(edit_, text.data(), length + 1);
    text.resize(static_cast<std::size_t>(length));
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == L'\r') text[i] = L'\n';
    }
    return utf8_from_wide(text);
}

void CodeEditor::set_text(const std::string& source) {
    if (edit_ == nullptr) return;
    loading_ = true;
    std::wstring wide = wide_from_utf8(source);
    std::wstring normalized;
    for (std::size_t i = 0; i < wide.size(); ++i) {
        if (wide[i] == L'\n' && (i == 0 || wide[i - 1] != L'\r')) normalized += L'\r';
        normalized += wide[i];
    }
    SetWindowTextW(edit_, normalized.c_str());
    loading_ = false;
    SendMessageW(edit_, EM_SETMODIFY, FALSE, 0);
    refresh();
}

void CodeEditor::clear() {
    set_text("");
}

void CodeEditor::focus_editor() {
    if (edit_ != nullptr) SetFocus(edit_);
}

std::string CodeEditor::position_text() const {
    if (edit_ == nullptr) return "Line 1, Col 1";
    CHARRANGE selection;
    SendMessageW(edit_, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    int line = static_cast<int>(SendMessageW(edit_, EM_LINEFROMCHAR, selection.cpMin, 0));
    int line_start = static_cast<int>(SendMessageW(edit_, EM_LINEINDEX, line, 0));
    int column = selection.cpMin - line_start + 1;
    return "Line " + std::to_string(line + 1) + ", Col " + std::to_string(column);
}

void CodeEditor::goto_line(int number) {
    if (edit_ == nullptr) return;
    int index = static_cast<int>(SendMessageW(edit_, EM_LINEINDEX, number - 1, 0));
    if (index < 0) index = 0;
    CHARRANGE selection{index, index};
    SendMessageW(edit_, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    SendMessageW(edit_, EM_SCROLLCARET, 0, 0);
    focus_editor();
}

void CodeEditor::set_font(HFONT font) {
    if (font_ != nullptr) DeleteObject(font_);
    font_ = font;
    if (edit_ != nullptr) {
        SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    }
    if (gutter_ != nullptr) {
        SendMessageW(gutter_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    }
    apply_highlight(edit_, true);
    InvalidateRect(gutter_, nullptr, TRUE);
}

}