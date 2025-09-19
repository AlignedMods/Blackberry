#pragma once

#include "types.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

enum LogLevel {
    Log_ = 0,
    Log_Info     = 1,
    Log_Debug    = 2,
    Log_Warning  = 3,
    Log_Error    = 4,
    Log_Critical = 5
};

inline void Log(LogLevel level, const char* message, ...) {
    bool showWarning = false;

    va_list args;
    va_start(args, message);

    char buffer[1024]{};

    switch (level) {
        case Log_Info:     strcpy(buffer, "[INFO]: ");    break;
        case Log_Debug:    strcpy(buffer, "[DEBUG]: ");   break;
        case Log_Warning:  strcpy(buffer, "[WARNING]: "); break;
        case Log_Error:    strcpy(buffer, "[ERROR]: ");   break;
        case Log_Critical: strcpy(buffer, "[CRITIAL]: "); break;

        default: strcpy(buffer, "[WARNING]: "); showWarning = true; break;
    }

    if (!showWarning) {
        strcat(buffer, message);
    } else {
        strcat(buffer, "Non valid LogLevel provided!");
    }

    strcat(buffer, "\n");
    vprintf(buffer, args);
    fflush(stdout); // flush the toilet

    va_end(args);
}