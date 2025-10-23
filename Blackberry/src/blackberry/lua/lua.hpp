#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry::Lua {

    void Initialize();
    void Shutdown();

    void RunFile(const std::filesystem::path& path, const std::string& moduleName);

    void SetExecutionContext(const std::string& moduleName);

    void PushRetValue();
    void PushMember(const std::string& table, const std::string& member);
    void PushMember(const std::string& member);

    void PushNumber(f64 value);

    void CallFunction(u32 argCount, u32 returnCount);

} // namespace Blackberry::Lua