#pragma once

#include <string>
#include <vector>

#include "Types.h"

namespace npp::platform {

struct WriteFileOptions {
    bool atomic = true;
    bool createParentDirs = true;
};

struct ListDirectoryOptions {
    bool recursive = false;
    bool includeDirectories = false;
};

class IFileSystemService {
public:
    virtual ~IFileSystemService() = default;

    virtual StatusOr<bool> Exists(const std::string& utf8Path) const = 0;
    virtual StatusOr<bool> IsDirectory(const std::string& utf8Path) const = 0;
    virtual StatusOr<std::string> ReadTextFile(const std::string& utf8Path) const = 0;
    virtual Status WriteTextFile(
        const std::string& utf8Path,
        const std::string& content,
        const WriteFileOptions& options) = 0;
    virtual Status CreateDirectories(const std::string& utf8Path) = 0;
    virtual Status RemoveFile(const std::string& utf8Path) = 0;
    virtual Status RenameReplace(
        const std::string& srcUtf8Path,
        const std::string& dstUtf8Path) = 0;
    virtual StatusOr<std::vector<std::string>> ListDirectory(
        const std::string& utf8Path,
        const ListDirectoryOptions& options) const = 0;
};

}  // namespace npp::platform
