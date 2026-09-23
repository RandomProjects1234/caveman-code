#include "paths.hpp"

#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cmc {

namespace {

std::string wide_to_utf8(const std::wstring& text) {
#ifdef _WIN32
    if (text.empty()) return "";
    int needed = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(needed), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(),
                        needed, nullptr, nullptr);
    return out;
#else
    return std::string(text.begin(), text.end());
#endif
}

std::string parent_of(const std::string& path) {
    std::filesystem::path p = std::filesystem::u8path(path);
    std::filesystem::path parent = p.parent_path();
    if (parent.empty()) return "";
    return parent.u8string();
}

}

std::string exe_dir() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0) return ".";
    std::wstring path(buffer, length);
    std::filesystem::path p(path);
    return wide_to_utf8(p.parent_path().wstring());
#else
    return ".";
#endif
}

std::string repo_root() {
    std::string here = exe_dir();
    for (int step = 0; step < 5; ++step) {
        if (here.empty()) break;
        std::filesystem::path examples = std::filesystem::u8path(here) / "examples";
        if (std::filesystem::is_directory(examples)) return here;
        std::string parent = parent_of(here);
        if (parent == here) break;
        here = parent;
    }
    return "";
}

std::string examples_dir() {
    std::string root = repo_root();
    if (root.empty()) return "";
    std::filesystem::path folder = std::filesystem::u8path(root) / "examples";
    if (std::filesystem::is_directory(folder)) return folder.u8string();
    return "";
}

std::vector<std::string> example_files() {
    std::string folder = examples_dir();
    std::vector<std::string> names;
    if (folder.empty()) return names;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(folder))) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().u8string();
        if (name.size() >= 4 && name.compare(name.size() - 4, 4, ".cmc") == 0) {
            names.push_back(entry.path().u8string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

}
