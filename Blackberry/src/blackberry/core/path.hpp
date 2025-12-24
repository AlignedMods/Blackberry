#pragma once

#include "blackberry/core/types.hpp"

#include <string>
#include <vector>
#include <xhash>

namespace Blackberry::FS {

    // Path class used for consistent paths across systems, such as:
    // 1. Consistent directory separators (always '/')
    // 2. No wchar_t and char mishaps
    class Path {
    public:
        Path() = default;
        Path(const std::vector<std::string>& components);
        Path(const std::string& path);
        Path(const char* path);

        operator std::string();
        operator const std::string() const;

        Path operator/(const Path& otherPath) const;
        void operator/=(const Path& otherPath);

        std::string String() const;

        void Append(const Path& otherPath);

        Path FileName() const;
        Path Stem() const;
        Path Extension() const;

        const bool operator==(const Path& otherPath) const;
        Path ParentPath() const;

        Path Relative(const Path& base) const;

        // Validates a path and turns an invalid path into a valid one if needed
        void Validate();

    private:
        std::vector<std::string> m_Components;
    };

    enum FileType { File, Directory };

    class DirectoryFile {
    public:
        DirectoryFile() = default;
        DirectoryFile(FileType type, const FS::Path& path)
            : m_Type(type), m_Path(path) {}

        const FS::Path& Path() const { return m_Path; };

        bool IsFile() const { return m_Type == FileType::File; }
        bool IsDirectory() const { return m_Type == FileType::Directory; }

    private:
        FileType m_Type = FileType::File;
        FS::Path m_Path;
    };

    class DirectoryIterator {
    public:
        // Standard c++ compatible iterators
        using iterator = std::vector<DirectoryFile>::iterator;
        using const_iterator = std::vector<DirectoryFile>::const_iterator;

        DirectoryIterator() = default;
        DirectoryIterator(const Path& base);

        iterator begin();
        iterator end();

        const_iterator cbegin() const;
        const_iterator cend() const;

    private:
        std::vector<DirectoryFile> m_Files;
    };

    bool Exists(const Path& path);

    Path Relative(const Path& path, const Path& base);

} // namespace Blackberry::FS

template<>
struct std::hash<Blackberry::FS::Path> {
    std::size_t operator()(const Blackberry::FS::Path& p) const {
        return std::hash<std::string>()(p.String());
    }
};