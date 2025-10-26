#include "Windows.h"
#include "ShlObj.h"

#include "blackberry/os/os.hpp"
#include "blackberry/application/application.hpp"

namespace Blackberry {

    namespace OS {

        std::string OpenFile(const char* filter) {
            OPENFILENAMEA ofn;
            CHAR szFile[260]{};
            CHAR currentDir[256]{};
            ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = static_cast<HWND>(BL_APP.GetWindow().GetNativeHandle());
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            if (GetCurrentDirectoryA(256, currentDir))
                ofn.lpstrInitialDir = currentDir;
            ofn.lpstrFilter = filter;
		    ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
            
            if (GetOpenFileNameA(&ofn) == TRUE) { return ofn.lpstrFile; }

            return std::string{};
        }

        std::string GetAppDataDirectory() {
            CHAR path[MAX_PATH]{};
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, path))) {
                return std::string(path);
            }

            return std::string{};
        }
    
    } // namespace OS

} // namespace Blackberry