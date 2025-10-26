#pragma once

#include "types.hpp"

#include "spdlog/spdlog.h"

#include <memory>

namespace Blackberry {

    class Logger {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger> GetCoreLogger();
        static std::shared_ptr<spdlog::logger> GetClientLogger();
    };

} // namespace Blackberry

#define BL_CORE_TRACE(...)    Blackberry::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define BL_CORE_INFO(...)     Blackberry::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define BL_CORE_WARN(...)     Blackberry::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define BL_CORE_ERROR(...)    Blackberry::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define BL_CORE_CRITICAL(...) Blackberry::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define BL_TRACE(...)    Blackberry::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define BL_INFO(...)     Blackberry::Logger::GetClientLogger()->info(__VA_ARGS__)
#define BL_WARN(...)     Blackberry::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define BL_ERROR(...)    Blackberry::Logger::GetClientLogger()->error(__VA_ARGS__)
#define BL_CRITICAL(...) Blackberry::Logger::GetClientLogger()->critical(__VA_ARGS__)