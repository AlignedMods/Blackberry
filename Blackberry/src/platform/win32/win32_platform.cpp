#include "Windows.h"
#include "ShlObj.h"
#include "shlwapi.h"

#include "blackberry/os/os.hpp"
#include "blackberry/application/application.hpp"

// WARNING: Absolute Win32 API hell ahead
// Nearly all of the code here has been scraped from the deepest darkest places on the internet
// Nothing here is original since it fucking sucks
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

        bool PathExists(const char* path) {
            return PathFileExistsA(path);
        }

        std::vector<FS::DirectoryFile> RetrieveDirectoryFiles(const char* base) {
            // Remove potential '/' at the end of directory name
            char baseDir[MAX_PATH];
            memset(baseDir, 0, sizeof(baseDir));
            
            strcpy(baseDir, base);
            int length = strlen(base);
            if (baseDir[length - 1] == '/') baseDir[length - 1] = '\0';

            std::vector<FS::DirectoryFile> files;

            HANDLE hFind = 0;
            WIN32_FIND_DATAA findData;

            char searchName[MAX_PATH], fullPath[MAX_PATH];

            memset(searchName, 0, sizeof(searchName));
            memset(&findData, 0, sizeof(WIN32_FIND_DATAA));

            sprintf(searchName, "%s/*", baseDir);

            hFind = FindFirstFileA(searchName, &findData);

            if (hFind != INVALID_HANDLE_VALUE) {
                while (FindNextFileA(hFind, &findData)) {
                    if (findData.cFileName[0] == '.') continue;

                    memset(fullPath, 0, sizeof(fullPath));
                    sprintf(fullPath, "%s/%s", baseDir, findData.cFileName);

                    FS::FileType type;

                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) type = FS::FileType::Directory;
                    else type = FS::FileType::File;

                    FS::DirectoryFile file(type, std::string(fullPath));

                    files.push_back(file);
                }

                FindClose(hFind);
            }

            return files;
        }
    
    } // namespace OS

} // namespace Blackberry