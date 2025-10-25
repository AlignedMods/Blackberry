#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry::Lua {

    void Initialize();
    void Shutdown();

    void RunFile(const std::filesystem::path& path, const std::string& moduleName);

    void SetExecutionContext(const std::string& moduleName);

    void NewTable();
    void SetTable(i32 index);

    void GetGlobal(const std::string& name);
    void GetMember(const std::string& table, const std::string& member);
    void GetMember(const std::string& member);
    i32 GetTop();

    void PushString(const std::string& value);
    void PushNumber(f64 value);
    void PushLightUserData(void* data);
    void PushValue(i32 value);
    void PushVec2(BlVec2 vec);
    void PushVec3(BlVec3 vec);

    i32 ToInteger(i32 index);
    f64 ToNumber(i32 index);
    std::string ToTypename(i32 index);

    void Pop(i32 count);
    void Remove(i32 index);

    void CallFunction(u32 argCount, u32 returnCount);

    u32 GetScriptRef(const std::string& moduleName);
    void* GetLuaState();

} // namespace Blackberry::Lua