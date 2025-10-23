#pragma once

#include <filesystem>

namespace Blackberry::Lua {

    void Initialize();

    void RunFile(const std::filesystem::path& path);

    void Shutdown();

} // namespace Blackberry::Lua