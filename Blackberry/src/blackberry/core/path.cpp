#include "blackberry/core/path.hpp"
#include "blackberry/os/os.hpp"
#include "blackberry/core/log.hpp"

namespace Blackberry::FS {

    Path::Path(const std::string& path)
        : m_Path(path) {}

    Path::operator std::string() {
        return m_Path;
    }

    Path::operator const std::string() const {
        return m_Path;
    }

    Path Path::operator/(const Path& otherPath) const {
        Path newPath = m_Path;
        newPath.Append(otherPath);
        return newPath;
    }

    void Path::operator/=(const Path& otherPath) {
        Append(otherPath);
    }

    std::string Path::String() const {
        return m_Path;
    }

    const char* Path::CString() const {
        return m_Path.c_str();
    }

    void Path::Append(const Path& otherPath) {
        // Add '/' if there isn't one at the end already
        if (m_Path.at(m_Path.size()) != '/') {
            m_Path.append(1, '/');
        }

        m_Path.append(otherPath.m_Path);
    }

    Path Path::FileName() const {
        std::string filename;

        std::string::const_iterator lastFileNameStart;
        std::string::const_iterator fileNameStart;

        // Find last occurence of '/'
        for (auto it = m_Path.begin(); it != m_Path.end(); it++) {
            lastFileNameStart = fileNameStart;

            if (*it == '/') {
                // A forward slash could be the end of the path
                if (it + 1 == m_Path.end()) { fileNameStart = lastFileNameStart; break; }
                else fileNameStart = it;
            }
        }

        // Get filename from last forward slash
        for (auto it = fileNameStart; it != m_Path.end(); it++) {
            // Skip forward slashes in the name
            if (*it != '/') filename.append(1, *it);
        }

        return filename;
    }

    Path Path::Stem() const {
        std::string stem;
        Path last = FileName();

        for (auto it = last.m_Path.begin(); it != last.m_Path.end(); it++) {
            if (*it != '.') stem.append(1, *it);
            else break;
        }

        return stem;
    }

    Path Path::Extension() const {
        std::string extension;
        Path last = FileName();

        bool found = false;

        for (auto it = last.m_Path.rbegin(); it != last.m_Path.rend(); it++) {
            if (found) break;

            if (*it != '.') extension.append(1, *it);
            else { found = true; extension.append(1, *it); }
        }

        std::reverse(extension.begin(), extension.end());

        return extension;
    }

    void Path::Validate() {
        for (auto it = m_Path.begin(); it != m_Path.end(); it++) {
            if (*it == '\\') *it = '/';
        }
    }

    DirectoryIterator::DirectoryIterator(const Path& base) {
        std::vector<DirectoryFile> files = OS::RetrieveDirectoryFiles(base.CString());

        for (const auto& file : files) {
            m_Files.push_back(file);

            BL_CORE_INFO("Path: {}", file.Path().String());
        }
    }

} // namespace Blackberry::FS