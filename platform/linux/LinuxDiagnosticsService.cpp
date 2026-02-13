#include "LinuxDiagnosticsService.h"

#include <filesystem>

namespace npp::platform {

namespace {

StatusOr<std::string> ValidateFileName(const std::string& fileName) {
    if (fileName.empty()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Diagnostic file name must not be empty"});
    }
    if (fileName.find('/') != std::string::npos) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Diagnostic file name must not contain '/'"});
    }
    return StatusOr<std::string>{Status::Ok(), fileName};
}

}  // namespace

LinuxDiagnosticsService::LinuxDiagnosticsService(
    const IPathService& pathService,
    IFileSystemService& fileSystemService)
    : _pathService(pathService), _fileSystemService(fileSystemService) {}

StatusOr<std::string> LinuxDiagnosticsService::EnsureCrashDirectory(const std::string& appName) {
    return EnsureCategoryDirectory(appName, "crash");
}

StatusOr<std::string> LinuxDiagnosticsService::EnsureLogDirectory(const std::string& appName) {
    return EnsureCategoryDirectory(appName, "log");
}

Status LinuxDiagnosticsService::WriteDiagnostic(
    const std::string& appName,
    const std::string& category,
    const std::string& fileName,
    const std::string& content) {
    const auto validatedName = ValidateFileName(fileName);
    if (!validatedName.ok()) {
        return validatedName.status;
    }

    const auto dir = EnsureCategoryDirectory(appName, category);
    if (!dir.ok()) {
        return dir.status;
    }

    std::filesystem::path fullPath = std::filesystem::path(*dir.value) / *validatedName.value;
    WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    return _fileSystemService.WriteTextFile(fullPath.string(), content, options);
}

StatusOr<std::string> LinuxDiagnosticsService::EnsureCategoryDirectory(
    const std::string& appName,
    const std::string& category) {
    if (appName.empty()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kInvalidArgument, "App name must not be empty"});
    }
    if (category.empty()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Category must not be empty"});
    }

    const auto appStateDir = _pathService.GetAppPath(PathScope::kState, appName);
    if (!appStateDir.ok()) {
        return StatusOr<std::string>::FromStatus(appStateDir.status);
    }

    std::filesystem::path categoryPath = std::filesystem::path(*appStateDir.value) / category;
    const Status createStatus = _fileSystemService.CreateDirectories(categoryPath.string());
    if (!createStatus.ok()) {
        return StatusOr<std::string>::FromStatus(createStatus);
    }

    return StatusOr<std::string>{Status::Ok(), categoryPath.string()};
}

}  // namespace npp::platform
