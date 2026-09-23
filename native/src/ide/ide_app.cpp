#include "ide_app.hpp"

#include <commdlg.h>
#include <commctrl.h>
#include <richedit.h>
#include <shellapi.h>

#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include "codegen.hpp"
#include "errors.hpp"
#include "ide_blocks.hpp"
#include "ide_canvas.hpp"
#include "ide_dialogs.hpp"
#include "ide_drawing.hpp"
#include "ide_editor.hpp"
#include "ide_runner.hpp"
#include "ide_theme.hpp"
#include "ide_widgets.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "paths.hpp"
#include "words.hpp"

namespace og {

namespace {

const wchar_t* APP_CLASS = L"OgaboogaMainWindow";
const int TIMER_PUMP = 1;

enum CommandId {
    ID_RUN = 1000,
    ID_STOP,
    ID_NEW,
    ID_OPEN,
    ID_SAVE,
    ID_SAVE_AS,
    ID_EXPORT,
    ID_EXIT,
    ID_EXAMPLES,
    ID_DICTIONARY,
    ID_ABOUT,
    ID_CHEATSHEET,
    ID_WIKI,
    ID_SEND_TEXT,
    ID_LOAD_BLOCKS,
    ID_UNDO,
    ID_CLEAR_BLOCKS,
    ID_WIPE,
    ID_FONT_UP,
    ID_FONT_DOWN,
};

std::wstring to_wide(const std::string& text) {
    return wide_from_utf8(text);
}

std::string from_wide(const std::wstring& text) {
    return utf8_from_wide(text);
}

std::string basename_of(const std::string& path) {
    std::size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string read_file(const std::string& path) {
    std::ifstream handle(path, std::ios::binary);
    if (!handle) return "";
    std::ostringstream buffer;
    buffer << handle.rdbuf();
    return buffer.str();
}

bool write_file(const std::string& path, const std::string& text) {
    std::ofstream handle(path, std::ios::binary);
    if (!handle) return false;
    handle << text;
    return true;
}

struct App {
    HINSTANCE instance = nullptr;
    HWND hwnd = nullptr;
    HFONT ui_font = nullptr;
    HFONT ui_bold = nullptr;
    HFONT ui_big = nullptr;
    HFONT title_font = nullptr;
    HFONT small_font = nullptr;
    HFONT code_font = nullptr;
    HFONT code_small_font = nullptr;

    ButtonBar* toolbar = nullptr;
    ButtonBar* block_bar = nullptr;
    ButtonBar* draw_bar = nullptr;
    TabBar* tabs = nullptr;
    BlockCanvas* canvas = nullptr;
    HWND block_code = nullptr;
    HWND block_header = nullptr;
    CodeEditor* editor = nullptr;
    DrawingPanel* draw = nullptr;
    OutputPane* output = nullptr;
    StatusBar* status = nullptr;

    std::unique_ptr<RunManager> runner;
    std::mutex queue_mutex;
    std::vector<std::pair<std::string, std::string>> out_queue;
    std::vector<RunOutcome> done_queue;

    std::string current_path;
    bool input_open = false;
    int code_size = 12;
    int code_small_size = 10;

    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam);

    void build_fonts();
    void build_menu();
    void build_children();
    void layout();
    void set_status(const std::string& message, COLORREF color = 0);
    void set_position(const std::string& message);

    void run_program();
    void stop_program();
    void new_file();
    void open_file();
    bool save_file();
    bool save_file_as();
    bool write_current(const std::string& path);
    void export_python();
    void open_examples();
    void on_example_picked(const std::string& path, const std::string& source,
                           const std::string& mode);
    void send_blocks_to_text();
    void load_text_to_blocks();
    void clear_blocks();
    void change_font(int delta);
    void open_cheatsheet();
    void open_wiki();
    void open_local(const std::string& path);
    void blocks_changed();
    void editor_changed();
    void pump();
    void finish_run(const RunOutcome& outcome);
    void on_close();
    void set_tab(int index);
};

void App::build_fonts() {
    ui_font = make_font(L"Segoe UI", 11, false);
    ui_bold = make_font(L"Segoe UI", 11, true);
    ui_big = make_font(L"Segoe UI", 14, true);
    title_font = make_font(L"Segoe UI", 20, true);
    small_font = make_font(L"Segoe UI", 9, false);
    code_font = make_font(L"Consolas", code_size, false);
    code_small_font = make_font(L"Consolas", code_small_size, false);
}

void App::build_menu() {
    HMENU menubar = CreateMenu();
    HMENU file_menu = CreatePopupMenu();
    AppendMenuW(file_menu, MF_STRING, ID_NEW, L"New\tCtrl+N");
    AppendMenuW(file_menu, MF_STRING, ID_OPEN, L"Open...\tCtrl+O");
    AppendMenuW(file_menu, MF_STRING, ID_SAVE, L"Save\tCtrl+S");
    AppendMenuW(file_menu, MF_STRING, ID_SAVE_AS, L"Save as...");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(file_menu, MF_STRING, ID_EXPORT, L"Export as Python (.py)...");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(file_menu, MF_STRING, ID_EXIT, L"Exit");
    AppendMenuW(menubar, MF_POPUP, reinterpret_cast<UINT_PTR>(file_menu), L"File");

    HMENU run_menu = CreatePopupMenu();
    AppendMenuW(run_menu, MF_STRING, ID_RUN, L"Run\tF5");
    AppendMenuW(run_menu, MF_STRING, ID_STOP, L"Stop\tEsc");
    AppendMenuW(menubar, MF_POPUP, reinterpret_cast<UINT_PTR>(run_menu), L"Run");

    HMENU help_menu = CreatePopupMenu();
    AppendMenuW(help_menu, MF_STRING, ID_EXAMPLES, L"Example caves...");
    AppendMenuW(help_menu, MF_STRING, ID_DICTIONARY, L"Caveman dictionary...");
    AppendMenuW(help_menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(help_menu, MF_STRING, ID_CHEATSHEET, L"Cheat sheet");
    AppendMenuW(help_menu, MF_STRING, ID_WIKI, L"Open the wiki");
    AppendMenuW(help_menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(help_menu, MF_STRING, ID_ABOUT, L"About OGABOOGA CODER");
    AppendMenuW(menubar, MF_POPUP, reinterpret_cast<UINT_PTR>(help_menu), L"Help");

    SetMenu(hwnd, menubar);
}

void App::build_children() {
    toolbar = new ButtonBar(hwnd, false);
    toolbar->set_font(ui_bold);
    toolbar->add(ID_RUN, L"RUN  (F5)", theme::GREEN, RGB(0x10, 0x26, 0x1a));
    toolbar->add(ID_STOP, L"STOP (Esc)", theme::RED, RGB(0x2b, 0x0f, 0x0f));
    toolbar->add_separator();
    toolbar->add(ID_NEW, L"New", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_OPEN, L"Open", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_SAVE, L"Save", theme::PANEL2, theme::TEXT);
    toolbar->add_separator();
    toolbar->add(ID_EXAMPLES, L"Examples", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_DICTIONARY, L"Dictionary", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_EXPORT, L"Export .py", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_FONT_UP, L"A+", theme::PANEL2, theme::TEXT);
    toolbar->add(ID_FONT_DOWN, L"A-", theme::PANEL2, theme::TEXT);
    toolbar->set_callback([this](int id) {
        switch (id) {
            case ID_RUN: run_program(); break;
            case ID_STOP: stop_program(); break;
            case ID_NEW: new_file(); break;
            case ID_OPEN: open_file(); break;
            case ID_SAVE: save_file(); break;
            case ID_EXAMPLES: open_examples(); break;
            case ID_DICTIONARY: show_dictionary_dialog(hwnd); break;
            case ID_EXPORT: export_python(); break;
            case ID_FONT_UP: change_font(1); break;
            case ID_FONT_DOWN: change_font(-1); break;
            default: break;
        }
    });

    tabs = new TabBar(hwnd);
    tabs->set_font(ui_bold);
    tabs->set_labels({L"Blocks (drag and drop)", L"Text (caveman code)", L"Draw (picture)"});
    tabs->set_callback([this](int index) { set_tab(index); });

    block_bar = new ButtonBar(hwnd, true);
    block_bar->set_font(small_font);
    block_bar->add(ID_RUN, L"Run blocks", theme::GREEN, RGB(0x10, 0x26, 0x1a));
    block_bar->add(ID_STOP, L"Stop", theme::RED, RGB(0x2b, 0x0f, 0x0f));
    block_bar->add(ID_SEND_TEXT, L"Send to Text tab", theme::PANEL2, theme::TEXT);
    block_bar->add(ID_LOAD_BLOCKS, L"Load from Text tab", theme::PANEL2, theme::TEXT);
    block_bar->add(ID_UNDO, L"Undo", theme::PANEL2, theme::TEXT);
    block_bar->add(ID_CLEAR_BLOCKS, L"Clear blocks", theme::PANEL2, theme::TEXT);
    block_bar->set_callback([this](int id) {
        switch (id) {
            case ID_RUN: run_program(); break;
            case ID_STOP: stop_program(); break;
            case ID_SEND_TEXT: send_blocks_to_text(); break;
            case ID_LOAD_BLOCKS: load_text_to_blocks(); break;
            case ID_UNDO: canvas->undo(); break;
            case ID_CLEAR_BLOCKS: clear_blocks(); break;
            default: break;
        }
    });

    canvas = new BlockCanvas(hwnd, ui_font, ui_bold, small_font, code_small_font);
    canvas->set_on_change([this] { blocks_changed(); });
    canvas->set_on_status([this](const std::string& message) { set_status(message); });

    block_header = CreateWindowExW(0, L"STATIC", L"YOUR CODE (it writes itself!)",
                                   WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 100, 24, hwnd, nullptr,
                                   instance, nullptr);
    SendMessageW(block_header, WM_SETFONT, reinterpret_cast<WPARAM>(ui_bold), TRUE);
    ensure_richedit_loaded();
    block_code = CreateWindowExW(0, MSFTEDIT_CLASS, L"",
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                 0, 0, 100, 100, hwnd, nullptr, instance, nullptr);
    SendMessageW(block_code, EM_SETBKGNDCOLOR, 0, theme::BG);
    SendMessageW(block_code, WM_SETFONT, reinterpret_cast<WPARAM>(code_small_font), TRUE);

    editor = new CodeEditor(hwnd, code_font);
    editor->set_on_change([this] { editor_changed(); });
    editor->set_on_position([this] { set_position(editor->position_text()); });

    draw = new DrawingPanel(hwnd);

    draw_bar = new ButtonBar(hwnd, true);
    draw_bar->set_font(small_font);
    draw_bar->add(ID_WIPE, L"Wipe picture", theme::PANEL2, theme::TEXT);
    draw_bar->set_callback([this](int id) {
        if (id == ID_WIPE) draw->wipe();
    });

    output = new OutputPane(hwnd, ui_bold, code_small_font);
    status = new StatusBar(hwnd, small_font);
    set_position("compiler inside - v" + std::string(cmc::words::VERSION));

    runner = std::make_unique<RunManager>(
        [this](const std::string& line, const std::string& tag) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            out_queue.emplace_back(line, tag);
        },
        [this](const RunOutcome& outcome) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            done_queue.push_back(outcome);
        });
}

void App::layout() {
    RECT client;
    GetClientRect(hwnd, &client);
    int width = client.right;
    int height = client.bottom;
    const int toolbar_h = 44;
    const int tabs_h = 36;
    const int status_h = 26;
    const int output_h = 170;

    toolbar->layout(0, 0, width, toolbar_h);
    tabs->layout(0, toolbar_h, width, tabs_h);
    status->layout(0, height - status_h, width, status_h);
    output->layout(0, height - status_h - output_h, width, output_h);

    int body_y = toolbar_h + tabs_h;
    int body_h = height - status_h - output_h - body_y;
    if (body_h < 80) body_h = 80;

    block_bar->layout(0, body_y, width, 32);
    int blocks_y = body_y + 32;
    int blocks_h = body_h - 32;
    canvas->layout_palette(0, blocks_y, 220, blocks_h);
    int preview_w = 300;
    int canvas_w = width - 220 - preview_w;
    if (canvas_w < 200) {
        canvas_w = 200;
        preview_w = width - 220 - canvas_w;
    }
    canvas->layout(220, blocks_y, canvas_w, blocks_h);
    MoveWindow(block_header, width - preview_w, blocks_y, preview_w, 24, TRUE);
    MoveWindow(block_code, width - preview_w, blocks_y + 24, preview_w, blocks_h - 24, TRUE);

    editor->layout(0, body_y, width, body_h);
    draw_bar->layout(0, body_y, width, 32);
    draw->layout(0, body_y + 32, width, body_h - 32);
}

void App::set_status(const std::string& message, COLORREF color) {
    if (status != nullptr) status->set_message(message, color == 0 ? theme::TEXT : color);
}

void App::set_position(const std::string& message) {
    if (status != nullptr) status->set_position(message);
}

void App::set_tab(int index) {
    bool blocks = index == 0;
    bool text = index == 1;
    bool draw_tab = index == 2;
    ShowWindow(block_bar->handle(), blocks ? SW_SHOW : SW_HIDE);
    ShowWindow(canvas->palette_handle(), blocks ? SW_SHOW : SW_HIDE);
    ShowWindow(canvas->handle(), blocks ? SW_SHOW : SW_HIDE);
    ShowWindow(block_header, blocks ? SW_SHOW : SW_HIDE);
    ShowWindow(block_code, blocks ? SW_SHOW : SW_HIDE);
    ShowWindow(editor->handle(), text ? SW_SHOW : SW_HIDE);
    ShowWindow(draw_bar->handle(), draw_tab ? SW_SHOW : SW_HIDE);
    ShowWindow(draw->handle(), draw_tab ? SW_SHOW : SW_HIDE);
    layout();
}

void App::blocks_changed() {
    std::string source = canvas->source();
    set_rich_text(block_code, to_wide(source));
    apply_highlight(block_code, true);
    std::vector<std::string> problems = check_blocks(canvas->get_blocks());
    if (!problems.empty()) {
        set_status("Block check: " + problems[0], theme::YELLOW);
    } else {
        set_status("Blocks look good! Press RUN.");
    }
}

void App::editor_changed() {
    set_position(editor->position_text());
}

void App::run_program() {
    if (runner->running()) {
        set_status("Already running! Press STOP first.", theme::YELLOW);
        return;
    }
    std::string source;
    std::string where;
    if (tabs->selected() == 0) {
        std::vector<std::string> problems = check_blocks(canvas->get_blocks());
        if (!problems.empty()) {
            output->write("The blocks need a little fix first:\n" + describe_problems(problems),
                          "err");
            set_status("Fix the block slots first.", theme::RED);
            return;
        }
        source = canvas->source();
        where = "Blocks";
    } else {
        source = editor->get_text();
        where = "Text";
    }
    if (source.find_first_not_of(" \t\r\n") == std::string::npos) {
        set_status("There is no code to run yet.", theme::YELLOW);
        return;
    }
    std::string problem = validate_source(source);
    if (!problem.empty()) {
        output->write(problem, "err");
        set_status("Ooga! The program needs a fix.", theme::RED);
        return;
    }
    output->clear();
    output->write("--- running (" + where + ") ---", "info");
    draw->wipe();
    set_status("Running...", theme::GREEN);
    std::string name = current_path.empty() ? "<your program>" : current_path;
    runner->start(source, name, draw);
}

void App::stop_program() {
    if (runner->running()) {
        runner->stop();
        set_status("Stopping...", theme::YELLOW);
    }
}

void App::new_file() {
    editor->clear();
    current_path.clear();
    tabs->select(1);
    set_status("New empty cave. Start typing!");
    editor->focus_editor();
}

void App::open_file() {
    wchar_t buffer[MAX_PATH] = L"";
    OPENFILENAMEW dialog;
    std::memset(&dialog, 0, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = hwnd;
    dialog.lpstrFilter = L"Cave Man Code (*.cmc)\0*.cmc\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrTitle = L"Open a CMC program";
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&dialog)) return;
    std::string path = from_wide(buffer);
    std::string source = read_file(path);
    editor->set_text(source);
    current_path = path;
    tabs->select(1);
    set_status("Opened " + basename_of(path));
}

bool App::write_current(const std::string& path) {
    std::string source =
        tabs->selected() == 0 ? canvas->source() : editor->get_text();
    if (!write_file(path, source)) {
        show_info_dialog(hwnd, "OOGA!", "Could not save that file.");
        return false;
    }
    current_path = path;
    set_status("Saved " + basename_of(path), theme::GREEN);
    return true;
}

bool App::save_file() {
    if (current_path.empty()) return save_file_as();
    return write_current(current_path);
}

bool App::save_file_as() {
    wchar_t buffer[MAX_PATH] = L"";
    OPENFILENAMEW dialog;
    std::memset(&dialog, 0, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = hwnd;
    dialog.lpstrFilter = L"Cave Man Code (*.cmc)\0*.cmc\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrTitle = L"Save your program";
    dialog.lpstrDefExt = L"cmc";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&dialog)) return false;
    return write_current(from_wide(buffer));
}

void App::export_python() {
    std::string source = tabs->selected() == 0 ? canvas->source() : editor->get_text();
    if (source.find_first_not_of(" \t\r\n") == std::string::npos) {
        set_status("Nothing to export yet.", theme::YELLOW);
        return;
    }
    std::string python_source;
    try {
        python_source = cmc::to_python_source(
            source, current_path.empty() ? "<your program>" : current_path);
    } catch (cmc::CmcError& error) {
        output->write(error.format(), "err");
        return;
    }
    wchar_t buffer[MAX_PATH] = L"";
    OPENFILENAMEW dialog;
    std::memset(&dialog, 0, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = hwnd;
    dialog.lpstrFilter = L"Python (*.py)\0*.py\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrTitle = L"Export as Python";
    dialog.lpstrDefExt = L"py";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&dialog)) return;
    std::string path = from_wide(buffer);
    if (!write_file(path, python_source)) {
        show_info_dialog(hwnd, "OOGA!", "Could not save that file.");
        return;
    }
    set_status("Exported " + basename_of(path), theme::GREEN);
    output->write("Made a real Python file: " + path, "ok");
}

void App::open_examples() {
    show_examples_dialog(hwnd, [this](const std::string& path, const std::string& source,
                                      const std::string& mode) {
        on_example_picked(path, source, mode);
    });
}

void App::on_example_picked(const std::string& path, const std::string& source,
                            const std::string& mode) {
    if (mode == "blocks") {
        try {
            std::vector<BlockPtr> loaded = source_to_blocks(source);
            canvas->set_blocks(loaded);
            tabs->select(0);
            set_status("Loaded " + basename_of(path) + " as blocks!");
            return;
        } catch (BlockError& error) {
            show_info_dialog(hwnd, "Almost!",
                             std::string("This example uses ideas that do not have blocks yet:\n\n")
                                 + error.message() + "\n\nOpening it in the Text tab instead.");
        }
    }
    editor->set_text(source);
    current_path.clear();
    tabs->select(1);
    set_status("Loaded example " + basename_of(path) + " (Save as to keep changes)");
}

void App::send_blocks_to_text() {
    std::string source = canvas->source();
    if (source.find_first_not_of(" \t\r\n") == std::string::npos) {
        set_status("No blocks yet.", theme::YELLOW);
        return;
    }
    editor->set_text(source);
    tabs->select(1);
    set_status("Blocks sent to the Text tab.");
}

void App::load_text_to_blocks() {
    std::string source = editor->get_text();
    if (source.find_first_not_of(" \t\r\n") == std::string::npos) {
        set_status("The Text tab is empty.", theme::YELLOW);
        return;
    }
    try {
        std::vector<BlockPtr> loaded = source_to_blocks(source);
        canvas->set_blocks(loaded);
        tabs->select(0);
        set_status("Text loaded into blocks!");
    } catch (BlockError& error) {
        show_info_dialog(hwnd, "Almost!", error.message());
    }
}

void App::clear_blocks() {
    canvas->clear_all();
    set_status("Blocks cleared. Undo still works!");
}

void App::change_font(int delta) {
    code_size = code_size + delta;
    if (code_size < 8) code_size = 8;
    if (code_size > 26) code_size = 26;
    code_small_size = code_small_size + delta;
    if (code_small_size < 8) code_small_size = 8;
    if (code_small_size > 26) code_small_size = 26;
    HFONT old_code = code_font;
    HFONT old_small = code_small_font;
    code_font = make_font(L"Consolas", code_size, false);
    code_small_font = make_font(L"Consolas", code_small_size, false);
    editor->set_font(code_font);
    SendMessageW(block_code, WM_SETFONT, reinterpret_cast<WPARAM>(code_small_font), TRUE);
    output->set_text_font(code_small_font);
    if (old_code != nullptr) DeleteObject(old_code);
    if (old_small != nullptr) DeleteObject(old_small);
    set_status("Font size changed.");
}

void App::open_cheatsheet() {
    std::string root = cmc::repo_root();
    if (root.empty()) {
        show_info_dialog(hwnd, "Not found", "I could not find that file in this copy of CMC.");
        return;
    }
    open_local(root + "\\CHEATSHEET.md");
}

void App::open_wiki() {
    std::string root = cmc::repo_root();
    if (root.empty()) {
        show_info_dialog(hwnd, "Not found", "I could not find that file in this copy of CMC.");
        return;
    }
    open_local(root + "\\website\\index.html");
}

void App::open_local(const std::string& path) {
    std::ifstream probe(path);
    if (!probe) {
        show_info_dialog(hwnd, "Not found", "I could not find that file in this copy of CMC.");
        return;
    }
    ShellExecuteW(hwnd, L"open", to_wide(path).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void App::pump() {
    std::vector<std::pair<std::string, std::string>> outputs;
    std::vector<RunOutcome> dones;
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        outputs.swap(out_queue);
        dones.swap(done_queue);
    }
    for (const auto& item : outputs) output->write(item.first, item.second);
    for (const auto& outcome : dones) finish_run(outcome);

    auto pending = runner->pending_input();
    if (pending && !input_open) {
        std::string prompt = pending->first;
        runner->clear_pending_input();
        input_open = true;
        std::string answer;
        bool ok = ask_string_dialog(hwnd, prompt.empty() ? "Type an answer:" : prompt, answer);
        runner->answer_input(ok ? answer : "");
        input_open = false;
    }
    if (runner->running()) set_position("running...");
}

void App::finish_run(const RunOutcome& outcome) {
    if (outcome.kind == RunOutcome::Kind::Finished) {
        output->write("Program finished! Well done, cave coder.", "ok");
        set_status("Program finished!", theme::GREEN);
    } else if (outcome.kind == RunOutcome::Kind::Stopped) {
        output->write("Stopped! (you pressed stop)", "err");
        set_status("Stopped.", theme::RED);
    } else {
        output->write(outcome.message, "err");
        set_status("Ooga! The program needs a fix.", theme::RED);
    }
    if (!current_path.empty()) {
        set_position(current_path);
    } else {
        set_position("compiler inside - v" + std::string(cmc::words::VERSION));
    }
}

void App::on_close() {
    if (runner) runner->stop();
    KillTimer(hwnd, TIMER_PUMP);
    DestroyWindow(hwnd);
}

LRESULT App::handle_message(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            build_fonts();
            build_children();
            layout();
            set_tab(0);
            SetTimer(hwnd, TIMER_PUMP, 40, nullptr);
            return 0;
        case WM_SIZE:
            layout();
            return 0;
        case WM_TIMER:
            if (wparam == TIMER_PUMP) {
                pump();
                return 0;
            }
            break;
        case WM_COMMAND: {
            int id = LOWORD(wparam);
            if (id >= 1000) {
                switch (id) {
                    case ID_RUN: run_program(); break;
                    case ID_STOP: stop_program(); break;
                    case ID_NEW: new_file(); break;
                    case ID_OPEN: open_file(); break;
                    case ID_SAVE: save_file(); break;
                    case ID_SAVE_AS: save_file_as(); break;
                    case ID_EXPORT: export_python(); break;
                    case ID_EXIT: on_close(); break;
                    case ID_EXAMPLES: open_examples(); break;
                    case ID_DICTIONARY: show_dictionary_dialog(hwnd); break;
                    case ID_ABOUT: show_about_dialog(hwnd); break;
                    case ID_CHEATSHEET: open_cheatsheet(); break;
                    case ID_WIKI: open_wiki(); break;
                    case ID_SEND_TEXT: send_blocks_to_text(); break;
                    case ID_LOAD_BLOCKS: load_text_to_blocks(); break;
                    case ID_UNDO: canvas->undo(); break;
                    case ID_CLEAR_BLOCKS: clear_blocks(); break;
                    case ID_WIPE: draw->wipe(); break;
                    case ID_FONT_UP: change_font(1); break;
                    case ID_FONT_DOWN: change_font(-1); break;
                    default: break;
                }
                return 0;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wparam);
            HWND control = reinterpret_cast<HWND>(lparam);
            if (control == block_header) {
                SetBkColor(dc, theme::PANEL);
                SetTextColor(dc, theme::ACCENT);
                static HBRUSH brush = CreateSolidBrush(theme::PANEL);
                return reinterpret_cast<LRESULT>(brush);
            }
            break;
        }
        case WM_CLOSE:
            on_close();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT CALLBACK App::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    App* app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    if (app == nullptr) return DefWindowProcW(hwnd, message, wparam, lparam);
    app->hwnd = hwnd;
    return app->handle_message(message, wparam, lparam);
}

}

namespace {

}

int run_app(HINSTANCE instance, const std::vector<std::string>& args) {
    (void)args;
    WNDCLASSW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = App::wnd_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(theme::BG);
    wc.lpszClassName = APP_CLASS;
    RegisterClassW(&wc);

    App app;
    app.instance = instance;
    std::wstring title = L"OGABOOGA CODER  -  Cave Man Code v";
    title += to_wide(cmc::words::VERSION);
    HWND hwnd = CreateWindowExW(0, APP_CLASS, title.c_str(), WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1260, 850, nullptr, nullptr,
                                instance, &app);
    if (hwnd == nullptr) return 1;
    app.hwnd = hwnd;
    app.build_menu();

    HACCEL accelerators = CreateAcceleratorTableW(
        new ACCEL[4]{
            {FVIRTKEY, VK_F5, ID_RUN},
            {FVIRTKEY, VK_ESCAPE, ID_STOP},
            {FVIRTKEY | FCONTROL, 'N', ID_NEW},
            {FVIRTKEY | FCONTROL, 'S', ID_SAVE},
        },
        4);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!TranslateAcceleratorW(hwnd, accelerators, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    DestroyAcceleratorTable(accelerators);
    return 0;
}

int run_selftest() {
    std::vector<std::string> problems;

    std::vector<std::string> lines;
    try {
        cmc::Interpreter interpreter;
        interpreter.set_output([&lines](const std::string& line) { lines.push_back(line); });
        interpreter.run("oga \"hello from selftest\"\ngrunk xs = snorf(1, 2)\nplop(xs, 3)\noga join(xs, \"-\")\n");
    } catch (std::exception& error) {
        problems.push_back(std::string("interpreter failed: ") + error.what());
    }
    if (lines != std::vector<std::string>{"hello from selftest", "1-2-3"}) {
        problems.push_back("interpreter output was wrong");
    }

    std::string source = "grunk x = 1\nbooga 3\n    oga lap\nunga\n";
    try {
        std::vector<BlockPtr> loaded = source_to_blocks(source);
        std::string generated = blocks_to_source(loaded);
        lines.clear();
        cmc::Interpreter interpreter;
        interpreter.set_output([&lines](const std::string& line) { lines.push_back(line); });
        interpreter.run(generated);
        if (lines != std::vector<std::string>{"1", "2", "3"}) {
            problems.push_back("generated block code printed the wrong thing");
        }
    } catch (std::exception& error) {
        problems.push_back(std::string("block round trip failed: ") + error.what());
    }

    if (!problems.empty()) {
        std::fprintf(stdout, "OGABOOGA SELFTEST FAILED:\n");
        for (const auto& problem : problems) std::fprintf(stdout, "  - %s\n", problem.c_str());
        std::fflush(stdout);
        return 1;
    }
    std::fprintf(stdout, "OGABOOGA SELFTEST OK\n");
    std::fflush(stdout);
    return 0;
}

}