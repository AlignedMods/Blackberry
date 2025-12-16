#include "blackberry/core/path.hpp"
#include "blackberry/os/os.hpp"
#include "blackberry/core/log.hpp"

namespace Blackberry::FS {

    Path::Path(const std::vector<std::string>& components)
        : m_Components(components) {}

    Path::Path(const std::string& path) {
        std::string normalized;
        for (auto c : path) {
            if (c == '\\') normalized += '/';
            else normalized += c;
        }

        // NOTE: '/' at the end of path does NOT signify directory, it's there so we know when the path is about to end
        if (normalized.back() != '/') normalized += '/';

        // Split path up into components
        std::string buf;

        for (auto it = normalized.begin(); it != normalized.end(); it++) {
            if (*it != '/') buf += *it;
            else { m_Components.push_back(buf); buf.clear(); }
        }
    }

    Path::Path(const char* path)
        : Path(std::string(path)) {}

    Path::operator std::string() {
        return String();
    }

    Path::operator const std::string() const {
        return String();
    }

    Path Path::operator/(const Path& otherPath) const {
        Path newPath(m_Components);
        newPath.Append(otherPath);
        return newPath;
    }

    void Path::operator/=(const Path& otherPath) {
        Append(otherPath);
    }

    const bool Path::operator==(const Path& otherPath) const {
        if (m_Components.size() != otherPath.m_Components.size()) return false;

        for (u32 i = 0; i < m_Components.size(); i++) {
            if (m_Components.at(i) != otherPath.m_Components.at(i)) return false;
        }

        return true;
    }

    std::string Path::String() const {
        std::string str;

        for (auto it = m_Components.begin(); it != m_Components.end(); it++) {
            str += *it;

            if (it + 1 != m_Components.end()) str += '/';
        }

        return str;
    }

    void Path::Append(const Path& otherPath) {
        for (const auto& comp : otherPath.m_Components) {
            m_Components.push_back(comp);
        }
    }

    Path Path::FileName() const {
        return m_Components.back();
    }

    Path Path::Stem() const {
        std::string stem;
        std::string last = FileName().String();

        for (auto comp : last) {
            if (comp != '.') stem += comp;
            else break;
        }

        return stem;
    }

    Path Path::Extension() const {
        std::string extension;
        std::string last = FileName().String();

        bool found = false;

        for (auto it = last.rbegin(); it != last.rend(); it++) {
            if (found) break;

            if (*it != '.') extension.append(1, *it);
            else { found = true; extension.append(1, *it); }
        }

        std::reverse(extension.begin(), extension.end());

        return extension;
    }

    Path Path::ParentPath() const {
        Path p = m_Components;
        p.m_Components.pop_back();
        
        return p;
    }

    Path Path::Relative(const Path& base) const {
        const auto& a = m_Components;
        const auto& b = base.m_Components;
        
        u32 i = 0;
        while (i < a.size() && i < b.size() && a[i] == b[i])
            i++;

        std::vector<std::string> result;

        for (u32 j = i; j < b.size(); ++j)
            result.push_back("..");

        for (u32 j = i; j < a.size(); ++j)
            result.push_back(a[j]);

        if (result.empty())
            result.push_back(".");

        return result;
    }

    void Path::Validate() {
        // for (auto it = m_Path.begin(); it != m_Path.end(); it++) {
        //     if (*it == '\\') *it = '/';
        // }
    }

    DirectoryIterator::DirectoryIterator(const Path& base) {
        m_Files = OS::RetrieveDirectoryFiles(base.String());
    }

    DirectoryIterator::iterator DirectoryIterator::begin() {
        return m_Files.begin();
    }

    DirectoryIterator::iterator DirectoryIterator::end() {
        return m_Files.end();
    }

    bool Exists(const Path& path) {
        return OS::PathExists(path.String());
    }

    Path Relative(const Path& path, const Path& base) {
        Path relative = path;
        return relative.Relative(base);
    }

} // namespace Blackberry::FS