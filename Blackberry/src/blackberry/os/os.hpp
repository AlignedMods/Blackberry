#pragma once

#include <string>

namespace Blackberry {

    namespace OS {
        
        std::string OpenFile(const char* filter);

        std::string GetAppDataDirectory();

    } // namespace OS

} // namespace Blackberry