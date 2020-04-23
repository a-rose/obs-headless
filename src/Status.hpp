#pragma once

#include <string>
#include <map>

class Status {
private:
    int code;
    std::string message;

protected:
    std::map<int, std::string> codeToStr;

public:
    Status(int code, std::string message)
        : code(code)
        , message(message) {
    }

    Status(int code)
        : Status(code, "") {
    }

    Status() 
        : Status(0) {
    }

    ~Status() {
    }

    bool ok() {
        return code == 0;
    }

    std::string error_message() {
        // TODO get from map securely
        return codeToStr[code] + ": " + message;
    }
};
