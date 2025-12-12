#pragma once

#include "blackberry/core/types.hpp"

#include <string>
#include <vector>

namespace Blackberry::FS {

    // Path class used for consistent paths across systems, such as:
    // 1. Consistent directory separators (always '/')
    // 2. No wchar_t and char mishaps
    class Path {
    public:
        Path() = default;
        Path(const std::string& path);

        operator std::string();
        operator const std::string() const;

        Path operator/(const Path& otherPath) const;
        void operator/=(const Path& otherPath);

        std::string String() const;
        const char* CString() const;

        void Append(const Path& otherPath);

        Path FileName() const;
        Path Stem() const;
        Path Extension() const;

        // Validates a path and turns an invalid path into a valid one if needed
        void Validate();

    private:
        std::string m_Path;
    };

    enum FileType { File, Directory };

    class DirectoryFile {
    public:
        DirectoryFile() = default;
        DirectoryFile(FileType type, const FS::Path& path)
            : m_Type(type), m_Path(path) {}

        FS::Path Path() const { return m_Path; };

        bool IsFile() const { return m_Type == FileType::File; }
        bool IsDirectory() const { return m_Type == FileType::Directory; }

    private:
        FileType m_Type = FileType::File;
        FS::Path m_Path;
    };

    class DirectoryIterator {
    public:
        // Standard c++ compatible iterators
        using iterator = DirectoryFile*;
        using const_iterator = const DirectoryFile*;

        DirectoryIterator() = default;
        DirectoryIterator(const Path& base);

        iterator begin();
        iterator end();

        const_iterator cbegin() const;
        const_iterator cend() const;

    private:
        std::vector<DirectoryFile> m_Files;
        u32 m_CurrentPath = 0;
    };

} // namespace Blackberry::FS