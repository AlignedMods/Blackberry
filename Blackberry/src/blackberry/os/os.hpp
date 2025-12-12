#pragma once

#include "blackberry/core/path.hpp"

#include <string>
#include <vector>

namespace Blackberry {

    namespace OS {
        
        std::string OpenFile(const char* filter);

        std::string GetAppDataDirectory();

        std::vector<FS::DirectoryFile> RetrieveDirectoryFiles(const char* base);

    } // namespace OS

} // namespace Blackberry