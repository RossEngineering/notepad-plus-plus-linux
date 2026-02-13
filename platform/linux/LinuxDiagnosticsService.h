#pragma once

#include "IDiagnosticsService.h"
#include "IFileSystemService.h"
#include "IPathService.h"

namespace npp::platform {

class LinuxDiagnosticsService final : public IDiagnosticsService {
public:
    LinuxDiagnosticsService(const IPathService& pathService, IFileSystemService& fileSystemService);
    ~LinuxDiagnosticsService() override = default;

    StatusOr<std::string> EnsureCrashDirectory(const std::string& appName) override;
    StatusOr<std::string> EnsureLogDirectory(const std::string& appName) override;
    Status WriteDiagnostic(
        const std::string& appName,
        const std::string& category,
        const std::string& fileName,
        const std::string& content) override;

private:
    StatusOr<std::string> EnsureCategoryDirectory(
        const std::string& appName,
        const std::string& category);

    const IPathService& _pathService;
    IFileSystemService& _fileSystemService;
};

}  // namespace npp::platform
