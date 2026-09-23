#include "errors.hpp"

namespace cmc {

CmcError::CmcError(std::string message, int line, std::string hint, std::string where)
    : message(std::move(message)), line(line), hint(std::move(hint)), where(std::move(where)) {}

std::string CmcError::format() const {
    std::string location;
    if (line >= 0) {
        location = " Line " + std::to_string(line) + ":";
    } else if (!where.empty()) {
        location = " " + where + ":";
    }
    std::string out = "OOGA!" + location + " " + message;
    if (!hint.empty()) {
        out += "\nHint: " + hint;
    }
    return out;
}

const char* CmcError::what() const noexcept {
    try {
        cached_ = format();
        return cached_.c_str();
    } catch (...) {
        return "OOGA! something went wrong";
    }
}

CmcStopped::CmcStopped() : CmcError("Stopped!", -1, "") {}

std::string CmcStopped::format() const {
    return "Stopped! (you pushed the stop button)";
}

CmcStepLimit::CmcStepLimit(int line)
    : CmcRuntimeError(
          "This program ran for a very long time. Maybe a loop never stops?",
          line,
          "check your 'zug' loops. Make sure the box they check gets changed inside the loop.") {}

CmcTooDeep::CmcTooDeep(const std::string& name, int line)
    : CmcRuntimeError(
          (name.empty() ? std::string("A clump") : "'" + name + "'")
              + " called itself too many times and went too deep.",
          line,
          "a clump can call itself, but it needs a way to stop. Use 'binga' to check.") {}

}
