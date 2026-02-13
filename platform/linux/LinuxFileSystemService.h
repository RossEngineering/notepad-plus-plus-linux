#pragma once

#include "IFileSystemService.h"

namespace npp::platform {

class LinuxFileSystemService final : public IFileSystemService {
public:
    ~LinuxFileSystemService() override = default;

    StatusOr<bool> Exists(const std::string& utf8Path) const override;
    StatusOr<bool> IsDirectory(const std::string& utf8Path) const override;
    StatusOr<std::string> ReadTextFile(const std::string& utf8Path) const override;
    Status WriteTextFile(
        const std::string& utf8Path,
        const std::string& content,
        const WriteFileOptions& options) override;
    Status CreateDirectories(const std::string& utf8Path) override;
    Status RemoveFile(const std::string& utf8Path) override;
    Status RenameReplace(
        const std::string& srcUtf8Path,
        const std::string& dstUtf8Path) override;
    StatusOr<std::vector<std::string>> ListDirectory(
        const std::string& utf8Path,
        const ListDirectoryOptions& options) const override;
};

}  // namespace npp::platform
