#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "IFileSystemService.h"
#include "IPathService.h"
#include "Types.h"

namespace npp::platform {

enum class ExtensionType {
    kLanguagePack = 0,
    kCommandPlugin,
    kToolIntegration,
};

enum class PermissionGrantMode {
    kDenied = 0,
    kAllowOnce,
    kAllowSession,
    kAllowAlways,
};

struct ExtensionManifest {
    int schemaVersion = 1;
    std::string id;
    std::string name;
    std::string version;
    ExtensionType type = ExtensionType::kCommandPlugin;
    std::string apiVersion;
    std::string description;
    std::string author;
    std::string homepage;
    std::string entrypoint;
    std::vector<std::string> permissions;
    std::vector<std::string> categories;
};

struct InstalledExtension {
    ExtensionManifest manifest;
    std::string installPath;
    bool enabled = true;
};

struct PermissionPromptRequest {
    std::string extensionId;
    std::string extensionName;
    std::string permission;
    std::string reason;
};

using PermissionPromptCallback = std::function<PermissionGrantMode(const PermissionPromptRequest&)>;

class LinuxExtensionService final {
public:
    LinuxExtensionService(
        IPathService* pathService,
        IFileSystemService* fileSystemService,
        std::string appName);

    void SetPermissionPrompt(PermissionPromptCallback callback);

    Status Initialize();
    StatusOr<ExtensionManifest> LoadManifestFromDirectory(const std::string& directoryUtf8) const;
    StatusOr<std::vector<InstalledExtension>> DiscoverInstalled() const;
    Status InstallFromDirectory(const std::string& sourceDirectoryUtf8);
    Status EnableExtension(const std::string& extensionId);
    Status DisableExtension(const std::string& extensionId);
    Status RemoveExtension(const std::string& extensionId);
    StatusOr<bool> IsPermissionGranted(
        const std::string& extensionId,
        const std::string& permission) const;
    StatusOr<bool> RequestPermission(
        const std::string& extensionId,
        const std::string& permission,
        const std::string& reason);

private:
    struct ExtensionStateEntry {
        bool enabled = true;
        std::string version;
        std::string installPath;
    };

    IPathService* _pathService = nullptr;
    IFileSystemService* _fileSystemService = nullptr;
    std::string _appName;
    PermissionPromptCallback _permissionPrompt;
    mutable std::map<std::string, std::set<std::string>> _sessionGrantedPermissions;

    Status EnsureServiceReady() const;
    Status EnsureBaseDirectories();
    std::string ExtensionsRootPath() const;
    std::string ExtensionInstallPath(const std::string& extensionId) const;
    std::string ExtensionStateFilePath() const;
    std::string ExtensionPermissionFilePath() const;

    static bool RequiresInteractivePermissionPrompt(const std::string& permission);

    Status CopyDirectoryRecursively(
        const std::string& sourceDirectoryUtf8,
        const std::string& destinationDirectoryUtf8) const;
    Status RemoveDirectoryRecursively(const std::string& directoryUtf8) const;

    StatusOr<ExtensionStateEntry> ReadStateEntry(const std::string& extensionId) const;
    Status WriteStateEntry(const std::string& extensionId, const ExtensionStateEntry& stateEntry);
    Status RemoveStateEntry(const std::string& extensionId);

    StatusOr<std::set<std::string>> ReadPersistedPermissions(const std::string& extensionId) const;
    Status WritePersistedPermission(
        const std::string& extensionId,
        const std::string& permission,
        PermissionGrantMode grantMode);
    Status RemovePersistedPermissions(const std::string& extensionId);
};

}  // namespace npp::platform
