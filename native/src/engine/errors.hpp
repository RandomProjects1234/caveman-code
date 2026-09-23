#pragma once

#include <exception>
#include <string>

namespace cmc {

class CmcError : public std::exception {
public:
    CmcError(std::string message, int line = -1, std::string hint = "", std::string where = "");

    virtual std::string format() const;
    const char* what() const noexcept override;

    std::string message;
    int line = -1;
    std::string hint;
    std::string where;

private:
    mutable std::string cached_;
};

class CmcLexError : public CmcError {
public:
    using CmcError::CmcError;
};

class CmcParseError : public CmcError {
public:
    using CmcError::CmcError;
};

class CmcRuntimeError : public CmcError {
public:
    using CmcError::CmcError;
};

class CmcStopped : public CmcError {
public:
    CmcStopped();
    std::string format() const override;
};

class CmcStepLimit : public CmcRuntimeError {
public:
    explicit CmcStepLimit(int line = -1);
};

class CmcTooDeep : public CmcRuntimeError {
public:
    explicit CmcTooDeep(const std::string& name = "", int line = -1);
};

}
