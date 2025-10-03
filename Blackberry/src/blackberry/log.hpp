#pragma once

#include "types.hpp"

#include <format>
#include <iostream>

#define BL_INFO(...) Blackberry::Log(Blackberry::LogLevel::Info, __VA_ARGS__)
#define BL_DEBUG(...) Blackberry::Log(Blackberry::LogLevel::Debug, __VA_ARGS__)
#define BL_WARNING(...) Blackberry::Log(Blackberry::LogLevel::Warning, __VA_ARGS__)
#define BL_ERROR(...) Blackberry::Log(Blackberry::LogLevel::Error, __VA_ARGS__)
#define BL_CRITICAL(...) Blackberry::Log(Blackberry::LogLevel::Critical, __VA_ARGS__); exit(1)

namespace Blackberry {

    enum class LogLevel {
        Info,
        Debug,
        Warning,
        Error,
        Critical 
    };
    
    template<typename... T>
    constexpr void Log(LogLevel level, const std::string_view fmt, T&&... args) {
        bool showWarning = false;
    
        std::string formatted = std::vformat(fmt, std::make_format_args(args...));
        std::string buffer;
    
        switch (level) {
            case LogLevel::Info:     buffer.append("Blackberry [INFO]: ");    break;
            case LogLevel::Debug:    buffer.append("Blackberry [DEBUG]: ");   break;
            case LogLevel::Warning:  buffer.append("Blackberry [WARNING]: "); break;
            case LogLevel::Error:    buffer.append("Blackberry [ERROR]: ");   break;
            case LogLevel::Critical: buffer.append("Blackberry [CRITIAL]: "); break;
        }
    
        buffer.append(formatted);
        buffer.append('\n', 1);
    
        std::cout << buffer;
    }

} // namespace Blackberry