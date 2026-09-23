#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace og {

int run_app(HINSTANCE instance, const std::vector<std::string>& args);
int run_selftest();

}