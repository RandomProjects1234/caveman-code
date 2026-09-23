#include "cli.hpp"

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "codegen.hpp"
#include "draw.hpp"
#include "errors.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "paths.hpp"
#include "textutil.hpp"
#include "words.hpp"

namespace cmc {

namespace {

volatile std::sig_atomic_t g_stop = 0;

void handle_sigint(int) {
    g_stop = 1;
}

const char* HELP_TEMPLATE = R"CMC_HELP(Cave Man Code (CMC) v{version}

  cmc                    talk to CMC line by line (REPL)
  cmc run <file.cmc>     run a program
  cmc compile <file.cmc> [out.py]
                         turn a program into Python
  cmc init [file.cmc]    make a starter program (default hello.cmc)
  cmc examples           show the example programs
  cmc version            show the version
  cmc help               show this

Learn to talk caveman in 30 minutes: read docs/01_TUTORIAL_30_MINUTES.md
or open the wiki in website/index.html
)CMC_HELP";

const char* STARTER = R"CMC_STARTER(ugg Welcome to Cave Man Code!
ugg This is a note: the computer skips everything after "ugg".

oga "Ooga booga! Me make first program!"

grunk name = blorp "What is your name?"
oga "Hello", name, "! You are a cave coder now."

grunk rocks = snorf()
plop(rocks, "big rock")
plop(rocks, "shiny rock")

booga 3
    oga "Me bang rock!"
unga

oga "Me have", nom(rocks), "rocks."
zoop rock in rocks
    oga "  -", rock
unga
)CMC_STARTER";

std::string help_text() {
    std::string text = HELP_TEMPLATE;
    std::string needle = "{version}";
    std::size_t spot = text.find(needle);
    if (spot != std::string::npos) {
        text.replace(spot, needle.size(), words::VERSION);
    }
    return text;
}

void setup_console() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::signal(SIGINT, handle_sigint);
}

std::string basename_of(const std::string& path) {
    std::filesystem::path p = std::filesystem::u8path(path);
    return p.filename().u8string();
}

std::string strip_extension(const std::string& path) {
    std::filesystem::path p = std::filesystem::u8path(path);
    return p.parent_path().u8string().empty()
               ? p.stem().u8string()
               : (p.parent_path() / p.stem()).u8string();
}

std::optional<std::string> read_file(const std::string& path) {
    std::error_code code;
    if (!std::filesystem::exists(std::filesystem::u8path(path), code)) {
        std::cout << "OOGA! I cannot find the file '" << path << "'." << std::endl;
        std::cout << "Hint: check the name, or make a new one with: cmc init " << path
                  << std::endl;
        return std::nullopt;
    }
    std::ifstream handle(std::filesystem::u8path(path), std::ios::binary);
    if (!handle) {
        std::cout << "OOGA! I could not read '" << path << "'." << std::endl;
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << handle.rdbuf();
    return buffer.str();
}

std::string console_input(const std::string& prompt, int line) {
    if (!prompt.empty()) {
        std::cout << prompt << " ";
        std::cout.flush();
    }
    std::string answer;
    if (!std::getline(std::cin, answer)) {
        if (g_stop) {
            g_stop = 0;
            throw CmcStopped();
        }
        throw CmcRuntimeError(
            "blorp asked a question, but no answer could come in.",
            line,
            "make sure someone can type an answer.");
    }
    if (!answer.empty() && answer.back() == '\r') answer.pop_back();
    return answer;
}

int cmd_run(const std::string& path) {
    std::optional<std::string> source = read_file(path);
    if (!source) return 1;

    std::unique_ptr<DrawSurface> draw;
    std::string title = "CMC picture - " + basename_of(path);
    if (std::getenv("CMC_NO_DRAW") != nullptr) {
        draw = std::make_unique<NullDraw>(title);
    } else {
        draw = std::make_unique<Win32Draw>(title);
    }

    Interpreter interp;
    interp.set_draw(draw.get());
    interp.set_input(console_input);
    interp.set_should_stop([] { return g_stop != 0; });

    try {
        interp.run(*source, path);
    } catch (CmcStopped&) {
        std::cout << "Stopped!" << std::endl;
        return 1;
    } catch (CmcError& error) {
        std::cerr << error.format() << std::endl;
        return 1;
    }
    draw->hold_open();
    return 0;
}

int cmd_compile(const std::string& path, const std::string& out_path = "") {
    std::optional<std::string> source = read_file(path);
    if (!source) return 1;

    std::string python_source;
    try {
        python_source = to_python_source(*source, path);
    } catch (CmcError& error) {
        std::cerr << error.format() << std::endl;
        return 1;
    }

    std::string out = out_path.empty() ? strip_extension(path) + ".py" : out_path;
    std::ofstream handle(std::filesystem::u8path(out));
    if (!handle) {
        std::cout << "OOGA! I could not write '" << out << "'." << std::endl;
        return 1;
    }
    handle << python_source;
    handle.close();
    std::cout << "Made " << out << " from " << path << std::endl;
    std::cout << "Run it with: python " << out << std::endl;
    return 0;
}

int cmd_init(const std::string& path = "") {
    std::string target = path.empty() ? "hello.cmc" : path;
    std::error_code code;
    if (std::filesystem::exists(std::filesystem::u8path(target), code)) {
        std::cout << "'" << target << "' already exists, so I left it alone." << std::endl;
        return 1;
    }
    std::ofstream handle(std::filesystem::u8path(target));
    if (!handle) {
        std::cout << "OOGA! I could not write '" << target << "'." << std::endl;
        return 1;
    }
    handle << STARTER;
    handle.close();
    std::cout << "Made " << target << " -- have a look and run it:" << std::endl;
    std::cout << "  cmc run " << target << std::endl;
    return 0;
}

int cmd_examples() {
    std::string folder = examples_dir();
    if (folder.empty()) {
        std::cout << "I could not find the examples folder in this copy of CMC." << std::endl;
        return 1;
    }
    std::vector<std::string> names;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(folder))) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().u8string();
        if (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cmc") == 0) {
            names.push_back(entry.path().u8string());
        }
    }
    std::sort(names.begin(), names.end());
    std::cout << "Example caves to explore (" << names.size() << "):" << std::endl;
    for (const auto& name : names) {
        std::cout << "  " << name << std::endl;
    }
    std::string first = names.empty() ? (std::filesystem::u8path(folder) / "hello.cmc").u8string()
                                      : names[0];
    std::cout << "\nRun one with: cmc run " << first << std::endl;
    return 0;
}

int block_depth(const std::vector<std::string>& buffer) {
    std::string source;
    for (std::size_t i = 0; i < buffer.size(); ++i) {
        if (i > 0) source += "\n";
        source += buffer[i];
    }
    std::vector<Token> tokens;
    try {
        tokens = tokenize(source);
    } catch (CmcError&) {
        return 0;
    }
    static const std::vector<std::string> openers = {"binga", "booga", "zug", "zoop", "clump"};
    int depth = 0;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const Token& tok = tokens[i];
        if (tok.kind != TokKind::Kw) continue;
        bool opener = false;
        for (const auto& name : openers) {
            if (tok.value == name) {
                opener = true;
                break;
            }
        }
        if (opener) {
            ++depth;
        } else if (tok.value == "unga") {
            --depth;
        } else if (tok.value == "wonga" && i + 1 < tokens.size()
                   && tokens[i + 1].kind == TokKind::Kw && tokens[i + 1].value == "binga") {
            ++depth;
        }
    }
    return depth;
}

int cmd_repl() {
    std::cout << "Cave Man Code (CMC) v" << words::VERSION << " -- talk to me!" << std::endl;
    std::cout << "Try: oga \"hello cave\"   (type quit when you are done)" << std::endl;

    Interpreter interp;
    interp.set_output([](const std::string& line) {
        std::cout << line << "\n";
        std::cout.flush();
    });
    interp.set_should_stop([] { return g_stop != 0; });

    std::vector<std::string> buffer;
    while (true) {
        std::string prompt = buffer.empty() ? "oga> " : "...> ";
        std::cout << prompt;
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) {
            if (g_stop) {
                g_stop = 0;
                std::cout << std::endl;
                buffer.clear();
                continue;
            }
            std::cout << std::endl;
            break;
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string trimmed = trim_ascii(line);
        std::string lowered = text_lower(trimmed);
        if (buffer.empty() && (lowered == "quit" || lowered == "exit" || lowered == "bye")) {
            break;
        }
        if (trimmed.empty() && buffer.empty()) continue;

        buffer.push_back(line);
        if (block_depth(buffer) > 0) continue;

        std::string source;
        for (std::size_t i = 0; i < buffer.size(); ++i) {
            if (i > 0) source += "\n";
            source += buffer[i];
        }
        buffer.clear();
        try {
            interp.run(source, "<talk>");
        } catch (CmcStopped&) {
            std::cout << "Stopped!" << std::endl;
        } catch (CmcError& error) {
            std::cout << error.format() << std::endl;
        }
    }
    std::cout << "Bye bye, cave friend!" << std::endl;
    return 0;
}

}

int cli_main(int argc, char** argv) {
    setup_console();
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

    if (args.empty()) return cmd_repl();

    std::string command = text_lower(args[0]);

    if (command == "help" || command == "-h" || command == "--help" || command == "?") {
        std::cout << help_text() << "\n";
        return 0;
    }

    if (command == "version" || command == "-v" || command == "--version") {
        std::cout << "Cave Man Code (CMC) v" << words::VERSION << std::endl;
        return 0;
    }

    if (command == "run") {
        if (args.size() < 2) {
            std::cout << "Which program should I run? Try: cmc run hello.cmc" << std::endl;
            return 1;
        }
        return cmd_run(args[1]);
    }

    if (command == "compile") {
        if (args.size() < 2) {
            std::cout << "Which program should I compile? Try: cmc compile hello.cmc" << std::endl;
            return 1;
        }
        return cmd_compile(args[1], args.size() > 2 ? args[2] : "");
    }

    if (command == "init") {
        return cmd_init(args.size() > 1 ? args[1] : "");
    }

    if (command == "examples") {
        return cmd_examples();
    }

    if (command.size() >= 4 && command.compare(command.size() - 4, 4, ".cmc") == 0) {
        return cmd_run(args[0]);
    }

    std::cout << "OOGA! I do not know the command '" << args[0] << "'." << std::endl;
    std::cout << "Try: cmc help" << std::endl;
    return 1;
}

}
