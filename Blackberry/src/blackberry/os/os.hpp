#pragma once

#include "blackberry/core/path.hpp"

#include <string>
#include <vector>

namespace Blackberry {

    namespace OS {
        
        std::string OpenFile(const char* filter);

        std::string GetAppDataDirectory();

        bool PathExists(const std::string& path);
        std::vector<FS::DirectoryFile> RetrieveDirectoryFiles(const std::string& base);

    } // namespace OS

} // namespace Blackberry