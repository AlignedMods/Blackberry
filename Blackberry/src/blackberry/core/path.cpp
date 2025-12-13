#include "blackberry/core/path.hpp"
#include "blackberry/os/os.hpp"
#include "blackberry/core/log.hpp"

namespace Blackberry::FS {

    Path::Path(const std::string& path)
        : m_Path(path) { Validate(); }

    Path::Path(const char* path)
        : m_Path(path) { Validate(); }

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

    const bool Path::operator==(const Path& otherPath) const {
        return m_Path == otherPath.m_Path;
    }

    std::string Path::String() const {
        return m_Path;
    }

    const char* Path::CString() const {
        return m_Path.c_str();
    }

    void Path::Append(const Path& otherPath) {
        // Add '/' if there isn't one at the end already
        if (m_Path.at(m_Path.size() - 1) != '/') {
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

    Path Path::ParentPath() const {
        std::string properPath;
        std::string parent;

        properPath = m_Path;
        if (properPath.at(properPath.size() - 1) == '/') properPath.pop_back(); // Remove '/' at the end of the path if it exists

        std::string::const_iterator start = properPath.begin();
        std::string::const_iterator end = properPath.end();

        for (auto it = properPath.begin(); it != properPath.end(); it++) {
            if (*it == '/') end = it;
        }

        for (auto it = start; it != end; it++) {
            parent.append(1, *it);
        }

        return parent;
    }

    void Path::Validate() {
        for (auto it = m_Path.begin(); it != m_Path.end(); it++) {
            if (*it == '\\') *it = '/';
        }
    }

    DirectoryIterator::DirectoryIterator(const Path& base) {
        m_Files = OS::RetrieveDirectoryFiles(base.CString());
    }

    DirectoryIterator::iterator DirectoryIterator::begin() {
        return m_Files.begin();
    }

    DirectoryIterator::iterator DirectoryIterator::end() {
        return m_Files.end();
    }

    bool Exists(const Path& path) {
        return OS::PathExists(path.CString());
    }

} // namespace Blackberry::FS