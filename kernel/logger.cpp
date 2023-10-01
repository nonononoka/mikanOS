#include "logger.hpp"

#include <cstddef>
#include <cstdio>

#include "console.hpp"

namespace {
    LogLevel log_level = kWarn;
}

extern Console* console; //refer to other file(main.cpp)

void SetLogLevel(LogLevel level) {
    log_level = level;
}

int Log(LogLevel level, const char* format, ...){
    if (level > log_level) { //if level is under log_level, show message.
        return 0;
    }

    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console -> PutString(s);
    return result;
}