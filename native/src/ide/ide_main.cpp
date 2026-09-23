#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

#include "ide_app.hpp"
#include "ide_editor.hpp"

namespace {

std::vector<std::string> arguments() {
    std::vector<std::string> args;
    int count = 0;
    LPWSTR* wide = CommandLineToArgvW(GetCommandLineW(), &count);
    if (wide == nullptr) return args;
    for (int i = 1; i < count; ++i) {
        args.push_back(og::utf8_from_wide(wide[i]));
    }
    LocalFree(wide);
    return args;
}

}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show) {
    (void)previous;
    (void)command_line;
    (void)show;
    std::vector<std::string> args = arguments();
    bool selftest = false;
    for (const auto& arg : args) {
        if (arg == "--selftest") selftest = true;
    }
    if (selftest) {
        AttachConsole(ATTACH_PARENT_PROCESS);
        std::freopen("CONOUT$", "w", stdout);
        std::freopen("CONOUT$", "w", stderr);
        int code = og::run_selftest();
        std::fflush(stdout);
        return code;
    }
    return og::run_app(instance, args);
}