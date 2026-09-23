#pragma once

#include <windows.h>

#include <functional>
#include <string>

namespace og {

void show_about_dialog(HWND parent);
void show_examples_dialog(
    HWND parent,
    std::function<void(const std::string& path, const std::string& source,
                       const std::string& mode)> on_pick);
void show_dictionary_dialog(HWND parent);
bool ask_string_dialog(HWND parent, const std::string& prompt, std::string& answer);
void show_text_dialog(HWND parent, const std::string& title, const std::string& body);
void show_info_dialog(HWND parent, const std::string& title, const std::string& message);
bool ask_yes_no_dialog(HWND parent, const std::string& title, const std::string& message);

}