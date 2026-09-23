#include "ide_dialogs.hpp"

#include <commctrl.h>
#include <richedit.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#include "ide_editor.hpp"
#include "ide_theme.hpp"
#include "paths.hpp"
#include "textutil.hpp"
#include "words.hpp"
#include "words_data.hpp"

namespace og {

namespace {

const wchar_t* EXAMPLES_CLASS = L"OgaboogaExamples";
const wchar_t* DICTIONARY_CLASS = L"OgaboogaDictionary";
const wchar_t* ABOUT_CLASS = L"OgaboogaAbout";
const wchar_t* INPUT_CLASS = L"OgaboogaInput";
const wchar_t* TEXT_CLASS = L"OgaboogaTextDialog";

HFONT g_dialog_font = nullptr;
HFONT g_dialog_bold = nullptr;

HFONT dialog_font() {
    if (g_dialog_font == nullptr) g_dialog_font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE,
                                                              FALSE, FALSE, DEFAULT_CHARSET,
                                                              OUT_DEFAULT_PRECIS,
                                                              CLIP_DEFAULT_PRECIS,
                                                              CLEARTYPE_QUALITY,
                                                              DEFAULT_PITCH, L"Segoe UI");
    return g_dialog_font;
}

HFONT dialog_bold() {
    if (g_dialog_bold == nullptr) g_dialog_bold = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE,
                                                              FALSE, FALSE, DEFAULT_CHARSET,
                                                              OUT_DEFAULT_PRECIS,
                                                              CLIP_DEFAULT_PRECIS,
                                                              CLEARTYPE_QUALITY,
                                                              DEFAULT_PITCH, L"Segoe UI");
    return g_dialog_bold;
}

std::string read_text_file(const std::string& path) {
    std::ifstream handle(path, std::ios::binary);
    if (!handle) return "";
    std::ostringstream buffer;
    buffer << handle.rdbuf();
    return buffer.str();
}

void set_rich_text(HWND edit, const std::wstring& text) {
    if (edit == nullptr) return;
    CHARRANGE all;
    all.cpMin = 0;
    all.cpMax = -1;
    SendMessageW(edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&all));
    SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
}

void append_rich(HWND edit, const std::wstring& text, COLORREF color, HFONT font,
                 bool bold = false) {
    int length = GetWindowTextLengthW(edit);
    SendMessageW(edit, EM_SETSEL, length, length);
    CHARFORMAT2W format;
    std::memset(&format, 0, sizeof(format));
    format.cbSize = sizeof(format);
    format.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
    format.crTextColor = color;
    format.yHeight = font == dialog_bold() ? 240 : 200;
    wcsncpy(format.szFaceName, L"Segoe UI", LF_FACESIZE - 1);
    if (bold) {
        format.dwMask |= CFM_BOLD;
        format.dwEffects |= CFE_BOLD;
    }
    SendMessageW(edit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    std::wstring normalized;
    for (wchar_t c : text) {
        if (c == L'\n') normalized += L'\r';
        else normalized += c;
    }
    SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(normalized.c_str()));
}

struct DialogState {
    bool modal = true;
};

void run_modal(HWND dialog, HWND parent) {
    EnableWindow(parent, FALSE);
    MSG message;
    while (IsWindow(dialog) && GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(dialog, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
}

// ---------------------------------------------------------------- examples

struct ExamplesState {
    HWND dialog = nullptr;
    HWND list = nullptr;
    HWND preview = nullptr;
    std::vector<std::string> paths;
    std::function<void(const std::string&, const std::string&, const std::string&)> on_pick;
    bool shown = false;
};

void examples_layout(ExamplesState* state) {
    RECT client;
    GetClientRect(state->dialog, &client);
    int width = client.right;
    int height = client.bottom;
    MoveWindow(state->list, 12, 44, 290, height - 110, TRUE);
    MoveWindow(state->preview, 314, 44, width - 326, height - 110, TRUE);
}

void examples_show_selection(ExamplesState* state) {
    int selection = static_cast<int>(SendMessageW(state->list, LB_GETCURSEL, 0, 0));
    if (selection < 0 || selection >= static_cast<int>(state->paths.size())) return;
    std::string source = read_text_file(state->paths[selection]);
    std::wstring wide;
    int needed = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), static_cast<int>(source.size()),
                                     nullptr, 0);
    wide.resize(static_cast<std::size_t>(needed));
    MultiByteToWideChar(CP_UTF8, 0, source.c_str(), static_cast<int>(source.size()), wide.data(),
                        needed);
    set_rich_text(state->preview, wide);
}

void examples_pick(ExamplesState* state, const std::string& mode) {
    int selection = static_cast<int>(SendMessageW(state->list, LB_GETCURSEL, 0, 0));
    if (selection < 0 || selection >= static_cast<int>(state->paths.size())) return;
    std::string path = state->paths[selection];
    std::string source = read_text_file(path);
    if (state->on_pick) state->on_pick(path, source, mode);
    DestroyWindow(state->dialog);
}

LRESULT CALLBACK examples_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    ExamplesState* state = reinterpret_cast<ExamplesState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        state = static_cast<ExamplesState*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->dialog = hwnd;
    }
    if (state == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    switch (message) {
        case WM_SIZE:
            examples_layout(state);
            return 0;
        case WM_COMMAND: {
            int id = LOWORD(wparam);
            int code = HIWORD(wparam);
            if (id == 101 && code == LBN_SELCHANGE) {
                examples_show_selection(state);
                return 0;
            }
            if (id == 102 && code == LBN_DBLCLK) {
                examples_pick(state, "text");
                return 0;
            }
            if (id == 201) {
                examples_pick(state, "text");
                return 0;
            }
            if (id == 202) {
                examples_pick(state, "blocks");
                return 0;
            }
            if (id == 203) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX: {
            HDC dc = reinterpret_cast<HDC>(wparam);
            SetBkColor(dc, theme::BG);
            SetTextColor(dc, theme::TEXT);
            static HBRUSH brush = CreateSolidBrush(theme::BG);
            return reinterpret_cast<LRESULT>(brush);
        }
        case WM_CTLCOLOREDIT: {
            HDC dc = reinterpret_cast<HDC>(wparam);
            SetBkColor(dc, theme::BG);
            SetTextColor(dc, theme::TEXT);
            static HBRUSH brush = CreateSolidBrush(theme::BG);
            return reinterpret_cast<LRESULT>(brush);
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

}

void show_examples_dialog(
    HWND parent,
    std::function<void(const std::string& path, const std::string& source,
                       const std::string& mode)> on_pick) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = examples_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(theme::PANEL);
        wc.lpszClassName = EXAMPLES_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }

    ExamplesState state;
    state.on_pick = std::move(on_pick);
    state.paths = cmc::example_files();
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, EXAMPLES_CLASS,
                                  L"Example caves", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 900, 560, parent, nullptr,
                                  GetModuleHandleW(nullptr), &state);
    if (dialog == nullptr) return;

    state.list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 12, 44, 290,
                                 400, dialog, reinterpret_cast<HMENU>(101), GetModuleHandleW(nullptr),
                                 nullptr);
    SendMessageW(state.list, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
    state.preview = CreateWindowExW(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                    314, 44, 560, 400, dialog, reinterpret_cast<HMENU>(103),
                                    GetModuleHandleW(nullptr), nullptr);
    SendMessageW(state.preview, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
    SendMessageW(state.preview, EM_SETBKGNDCOLOR, 0, theme::BG);

    HWND open_text = CreateWindowExW(0, L"BUTTON", L"Open in Text tab",
                                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 8, 150, 28,
                                     dialog, reinterpret_cast<HMENU>(201),
                                     GetModuleHandleW(nullptr), nullptr);
    HWND open_blocks = CreateWindowExW(0, L"BUTTON", L"Open as Blocks",
                                       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 8, 140, 28,
                                       dialog, reinterpret_cast<HMENU>(202),
                                       GetModuleHandleW(nullptr), nullptr);
    HWND close_button = CreateWindowExW(0, L"BUTTON", L"Close",
                                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 318, 8, 90, 28,
                                        dialog, reinterpret_cast<HMENU>(203),
                                        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(open_text, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_bold()), TRUE);
    SendMessageW(open_blocks, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_bold()), TRUE);
    SendMessageW(close_button, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_bold()), TRUE);

    for (const auto& path : state.paths) {
        std::string base = path;
        std::size_t slash = base.find_last_of("\\/");
        if (slash != std::string::npos) base = base.substr(slash + 1);
        std::wstring wide;
        int needed = MultiByteToWideChar(CP_UTF8, 0, base.c_str(), static_cast<int>(base.size()),
                                         nullptr, 0);
        wide.resize(static_cast<std::size_t>(needed));
        MultiByteToWideChar(CP_UTF8, 0, base.c_str(), static_cast<int>(base.size()), wide.data(),
                            needed);
        SendMessageW(state.list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(wide.c_str()));
    }
    if (!state.paths.empty()) {
        SendMessageW(state.list, LB_SETCURSEL, 0, 0);
        examples_show_selection(&state);
    }

    ShowWindow(dialog, SW_SHOW);
    run_modal(dialog, parent);
}

// -------------------------------------------------------------- dictionary

namespace {

struct DictionaryState {
    HWND dialog = nullptr;
    HWND search = nullptr;
    HWND list = nullptr;
    HWND detail = nullptr;
    std::vector<const cmc::words::WordInfo*> visible;
};

std::string clean_text(const std::string& text) {
    std::string out;
    for (char c : text) {
        if (c != '`') out += c;
    }
    return out;
}

std::vector<const cmc::words::WordInfo*> sorted_words() {
    std::vector<const cmc::words::WordInfo*> words;
    for (const auto& word : cmc::words::word_infos()) words.push_back(&word);
    std::sort(words.begin(), words.end(),
              [](const cmc::words::WordInfo* a, const cmc::words::WordInfo* b) {
                  if (a->category != b->category) return a->category < b->category;
                  return a->name < b->name;
              });
    return words;
}

void dictionary_show(DictionaryState* state);

std::wstring utf8_to_wide_local(const std::string& text) {
    if (text.empty()) return L"";
    int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                     nullptr, 0);
    std::wstring out(static_cast<std::size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(),
                        needed);
    return out;
}

void dictionary_fill(DictionaryState* state) {
    std::wstring needle_wide;
    int length = GetWindowTextLengthW(state->search);
    std::wstring needle(static_cast<std::size_t>(length) + 1, L'\0');
    GetWindowTextW(state->search, needle.data(), length + 1);
    needle.resize(static_cast<std::size_t>(length));
    std::wstring lowered = needle;
    for (wchar_t& c : lowered) {
        if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
    }

    SendMessageW(state->list, LB_RESETCONTENT, 0, 0);
    state->visible.clear();
    for (const auto* word : sorted_words()) {
        std::string haystack = word->name + " " + word->display + " " + word->summary;
        for (const auto& alias : word->aliases) haystack += " " + alias;
        std::wstring wide_haystack = utf8_to_wide_local(haystack);
        for (wchar_t& c : wide_haystack) {
            if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
        }
        if (!lowered.empty() && wide_haystack.find(lowered) == std::wstring::npos) continue;
        state->visible.push_back(word);
        std::string label = "  " + (word->display.empty() ? word->name : word->display);
        if (!word->aliases.empty()) label += " (" + word->aliases[0] + ")";
        SendMessageW(state->list, LB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(utf8_to_wide_local(label).c_str()));
    }
    if (!state->visible.empty()) {
        SendMessageW(state->list, LB_SETCURSEL, 0, 0);
        dictionary_show(state);
    } else {
        set_rich_text(state->detail, L"");
    }
}

void dictionary_show(DictionaryState* state) {
    int selection = static_cast<int>(SendMessageW(state->list, LB_GETCURSEL, 0, 0));
    if (selection < 0 || selection >= static_cast<int>(state->visible.size())) return;
    const cmc::words::WordInfo* word = state->visible[selection];
    std::wstring display = utf8_to_wide_local(word->display.empty() ? word->name : word->display);
    set_rich_text(state->detail, L"");
    append_rich(state->detail, display + L"\r\n", theme::ACCENT, dialog_bold(), true);
    if (!word->aliases.empty()) {
        std::string aka;
        for (std::size_t i = 0; i < word->aliases.size(); ++i) {
            if (i > 0) aka += ", ";
            aka += word->aliases[i];
        }
        append_rich(state->detail, L"aka: " + utf8_to_wide_local(aka) + L"\r\n", theme::DIM,
                    dialog_font());
    }
    append_rich(state->detail, utf8_to_wide_local(word->category) + L"\r\n\r\n", theme::DIM,
                dialog_font());
    append_rich(state->detail, utf8_to_wide_local(word->summary) + L"\r\n\r\n", theme::TEXT,
                dialog_font());
    append_rich(state->detail, L"HOW TO WRITE IT\r\n", theme::GREEN, dialog_bold(), true);
    append_rich(state->detail, L"    " + utf8_to_wide_local(word->syntax) + L"\r\n", theme::YELLOW,
                dialog_font());
    for (const auto& extra : word->extra_syntax) {
        append_rich(state->detail, L"    " + utf8_to_wide_local(extra) + L"\r\n", theme::YELLOW,
                    dialog_font());
    }
    append_rich(state->detail, L"\r\n" + utf8_to_wide_local(clean_text(word->doc)) + L"\r\n",
                theme::TEXT, dialog_font());
    append_rich(state->detail, L"\r\nEXAMPLE\r\n", theme::GREEN, dialog_bold(), true);
    append_rich(state->detail, utf8_to_wide_local(word->example) + L"\r\n", theme::YELLOW,
                dialog_font());
    if (!word->tips.empty()) {
        append_rich(state->detail, L"\r\nTIPS\r\n", theme::GREEN, dialog_bold(), true);
        for (const auto& tip : word->tips) {
            append_rich(state->detail, L"  - " + utf8_to_wide_local(clean_text(tip)) + L"\r\n",
                        theme::TEXT, dialog_font());
        }
    }
    if (!word->related.empty()) {
        std::string related;
        for (std::size_t i = 0; i < word->related.size(); ++i) {
            if (i > 0) related += ", ";
            related += word->related[i];
        }
        append_rich(state->detail, L"\r\nSEE ALSO: " + utf8_to_wide_local(related), theme::DIM,
                    dialog_font());
    }
}

LRESULT CALLBACK dictionary_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    DictionaryState* state =
        reinterpret_cast<DictionaryState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        state = static_cast<DictionaryState*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->dialog = hwnd;
    }
    if (state == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    switch (message) {
        case WM_SIZE: {
            RECT client;
            GetClientRect(hwnd, &client);
            MoveWindow(state->search, 70, 42, client.right - 82, 26, TRUE);
            MoveWindow(state->list, 12, 78, 260, client.bottom - 90, TRUE);
            MoveWindow(state->detail, 284, 78, client.right - 296, client.bottom - 90, TRUE);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wparam);
            int code = HIWORD(wparam);
            if (id == 301 && (code == LBN_SELCHANGE)) {
                dictionary_show(state);
                return 0;
            }
            if (id == 302 && code == EN_CHANGE) {
                dictionary_fill(state);
                return 0;
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC dc = reinterpret_cast<HDC>(wparam);
            SetBkColor(dc, theme::BG);
            SetTextColor(dc, theme::TEXT);
            static HBRUSH brush = CreateSolidBrush(theme::BG);
            return reinterpret_cast<LRESULT>(brush);
        }
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wparam);
            SetBkColor(dc, theme::BG);
            SetTextColor(dc, theme::TEXT);
            static HBRUSH brush = CreateSolidBrush(theme::BG);
            return reinterpret_cast<LRESULT>(brush);
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

}

void show_dictionary_dialog(HWND parent) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = dictionary_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(theme::PANEL);
        wc.lpszClassName = DICTIONARY_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }

    DictionaryState state;
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, DICTIONARY_CLASS,
                                  L"Caveman dictionary", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 880, 600, parent, nullptr,
                                  GetModuleHandleW(nullptr), &state);
    if (dialog == nullptr) return;

    HWND title = CreateWindowExW(0, L"STATIC", L"Caveman dictionary -- every CMC word",
                                 WS_CHILD | WS_VISIBLE, 12, 10, 500, 24, dialog, nullptr,
                                 GetModuleHandleW(nullptr), nullptr);
    SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_bold()), TRUE);
    HWND label = CreateWindowExW(0, L"STATIC", L"Search:", WS_CHILD | WS_VISIBLE, 12, 44, 54,
                                 22, dialog, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
    state.search = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 70, 42, 700, 26,
                                   dialog, reinterpret_cast<HMENU>(302), GetModuleHandleW(nullptr),
                                   nullptr);
    SendMessageW(state.search, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
    state.list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 12, 78, 260, 460,
                                 dialog, reinterpret_cast<HMENU>(301), GetModuleHandleW(nullptr),
                                 nullptr);
    SendMessageW(state.list, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
    state.detail = CreateWindowExW(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
                                   WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                   284, 78, 570, 460, dialog, nullptr, GetModuleHandleW(nullptr),
                                   nullptr);
    SendMessageW(state.detail, EM_SETBKGNDCOLOR, 0, theme::BG);
    SendMessageW(state.detail, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);

    dictionary_fill(&state);
    ShowWindow(dialog, SW_SHOW);
    run_modal(dialog, parent);
}

// ------------------------------------------------------------------- about

namespace {

LRESULT CALLBACK about_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    HWND edit = reinterpret_cast<HWND>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (message) {
        case WM_CREATE: {
            edit = CreateWindowExW(0, MSFTEDIT_CLASS, L"",
                                   WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                   0, 0, 540, 380, hwnd, nullptr, GetModuleHandleW(nullptr),
                                   nullptr);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(edit));
            SendMessageW(edit, EM_SETBKGNDCOLOR, 0, theme::BG);
            SendMessageW(edit, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            append_rich(edit, L"OGABOOGA CODER\r\n", theme::ACCENT, dialog_bold(), true);
            append_rich(edit,
                        utf8_to_wide_local(std::string("the home cave of Cave Man Code (CMC) v")
                                           + cmc::words::VERSION + "\n\n"),
                        theme::DIM, dialog_font());
            append_rich(edit,
                        L"The compiler lives right inside this window.  When you press RUN, "
                        L"the same CMC engine that powers the `cmc` command runs your "
                        L"program.\r\n\r\nTwo ways to code:\r\n"
                        L"  - Blocks: drag colorful blocks and watch the code write itself\r\n"
                        L"  - Text: type CMC words like oga, grunk, binga, and unga\r\n\r\n"
                        L"Made for brand new coders, but with real power underneath:\r\n"
                        L"numbers, text, piles, clumps, recursion, and drawing.\r\n\r\n",
                        theme::TEXT, dialog_font());
            append_rich(edit, L"oga \"Ooga booga!\"\r\n", theme::YELLOW, dialog_font());
            append_rich(edit, L"\r\nMIT License. Go make something silly.\r\n", theme::DIM,
                        dialog_font());
            return 0;
        }
        case WM_SIZE: {
            RECT client;
            GetClientRect(hwnd, &client);
            MoveWindow(edit, 0, 0, client.right, client.bottom, TRUE);
            return 0;
        }
        case WM_COMMAND:
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

}

void show_about_dialog(HWND parent) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = about_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(theme::PANEL);
        wc.lpszClassName = ABOUT_CLASS;
        RegisterClassW(&wc);
        registered = true;
    }
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, ABOUT_CLASS, L"About OGABOOGA CODER",
                                  WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 560, 440, parent, nullptr,
                                  GetModuleHandleW(nullptr), nullptr);
    if (dialog == nullptr) return;
    ShowWindow(dialog, SW_SHOW);
    run_modal(dialog, parent);
}

// ------------------------------------------------------------- text dialog

namespace {

struct TextDialogState {
    std::wstring title;
    std::wstring body;
    HFONT title_font = nullptr;
    HFONT body_font = nullptr;
};

LRESULT CALLBACK text_dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    TextDialogState* state =
        reinterpret_cast<TextDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        state = static_cast<TextDialogState*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }
    switch (message) {
        case WM_CREATE: {
            HWND title = CreateWindowExW(0, L"STATIC", state->title.c_str(), WS_CHILD | WS_VISIBLE,
                                         16, 14, 480, 26, hwnd, nullptr, GetModuleHandleW(nullptr),
                                         nullptr);
            SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_bold()), TRUE);
            HWND body = CreateWindowExW(0, L"STATIC", state->body.c_str(),
                                        WS_CHILD | WS_VISIBLE | SS_LEFT, 16, 48, 500, 260, hwnd,
                                        nullptr, GetModuleHandleW(nullptr), nullptr);
            SendMessageW(body, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            HWND ok = CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                      430, 320, 90, 30, hwnd, reinterpret_cast<HMENU>(1),
                                      GetModuleHandleW(nullptr), nullptr);
            SendMessageW(ok, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            return 0;
        }
        case WM_COMMAND:
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

struct InputDialogState {
    std::wstring prompt;
    std::wstring answer;
    HWND edit = nullptr;
    bool accepted = false;
};

LRESULT CALLBACK input_dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    InputDialogState* state =
        reinterpret_cast<InputDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        state = static_cast<InputDialogState*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }
    if (state == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    switch (message) {
        case WM_CREATE: {
            HWND label = CreateWindowExW(0, L"STATIC", state->prompt.c_str(), WS_CHILD | WS_VISIBLE,
                                         16, 16, 400, 44, hwnd, nullptr, GetModuleHandleW(nullptr),
                                         nullptr);
            SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            state->edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                          WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 16, 66, 400, 26,
                                          hwnd, reinterpret_cast<HMENU>(10), GetModuleHandleW(nullptr),
                                          nullptr);
            SendMessageW(state->edit, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            HWND ok = CreateWindowExW(0, L"BUTTON", L"OK",
                                      WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 240, 108, 84, 30,
                                      hwnd, reinterpret_cast<HMENU>(1), GetModuleHandleW(nullptr),
                                      nullptr);
            HWND cancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE, 332, 108,
                                          84, 30, hwnd, reinterpret_cast<HMENU>(2),
                                          GetModuleHandleW(nullptr), nullptr);
            SendMessageW(ok, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            SendMessageW(cancel, WM_SETFONT, reinterpret_cast<WPARAM>(dialog_font()), TRUE);
            SetFocus(state->edit);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wparam) == 1) {
                int length = GetWindowTextLengthW(state->edit);
                std::wstring answer(static_cast<std::size_t>(length) + 1, L'\0');
                GetWindowTextW(state->edit, answer.data(), length + 1);
                answer.resize(static_cast<std::size_t>(length));
                state->answer = answer;
                state->accepted = true;
                DestroyWindow(hwnd);
                return 0;
            }
            if (LOWORD(wparam) == 2) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

}

bool ask_string_dialog(HWND parent, const std::string& prompt, std::string& answer) {
    static bool input_registered = false;
    if (!input_registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = input_dialog_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(theme::PANEL);
        wc.lpszClassName = INPUT_CLASS;
        RegisterClassW(&wc);
        input_registered = true;
    }
    InputDialogState state;
    state.prompt = utf8_to_wide_local(prompt.empty() ? "Type an answer:" : prompt);
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, INPUT_CLASS, L"Ooga! A question",
                                  WS_CAPTION | WS_SYSMENU | WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                                  460, 190, parent, nullptr, GetModuleHandleW(nullptr), &state);
    if (dialog == nullptr) return false;
    ShowWindow(dialog, SW_SHOW);
    run_modal(dialog, parent);
    if (!state.accepted) return false;
    answer = utf8_from_wide(state.answer);
    return true;
}

void show_text_dialog(HWND parent, const std::string& title, const std::string& body) {
    static bool text_registered = false;
    if (!text_registered) {
        WNDCLASSW wc;
        std::memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = text_dialog_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(theme::PANEL);
        wc.lpszClassName = TEXT_CLASS;
        RegisterClassW(&wc);
        text_registered = true;
    }
    TextDialogState state;
    state.title = utf8_to_wide_local(title);
    state.body = utf8_to_wide_local(body);
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, TEXT_CLASS, state.title.c_str(),
                                  WS_CAPTION | WS_SYSMENU | WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                                  560, 410, parent, nullptr, GetModuleHandleW(nullptr), &state);
    if (dialog == nullptr) return;
    ShowWindow(dialog, SW_SHOW);
    run_modal(dialog, parent);
}

void show_info_dialog(HWND parent, const std::string& title, const std::string& message) {
    MessageBoxW(parent, utf8_to_wide_local(message).c_str(), utf8_to_wide_local(title).c_str(),
                MB_OK | MB_ICONINFORMATION);
}

bool ask_yes_no_dialog(HWND parent, const std::string& title, const std::string& message) {
    return MessageBoxW(parent, utf8_to_wide_local(message).c_str(),
                       utf8_to_wide_local(title).c_str(),
                       MB_YESNO | MB_ICONQUESTION) == IDYES;
}

}