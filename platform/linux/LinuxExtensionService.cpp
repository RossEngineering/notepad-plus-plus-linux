#include "LinuxExtensionService.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>

#include "VscodeLanguageCompatibility.h"
#include "json/json.hpp"

namespace npp::platform {

namespace {

using Json = nlohmann::json;

Status MakeInvalidStatus(const std::string& message) {
    return Status{StatusCode::kInvalidArgument, message};
}

StatusOr<std::string> ReadTextFile(IFileSystemService& fileSystemService, const std::string& pathUtf8) {
    return fileSystemService.ReadTextFile(pathUtf8);
}

Status WriteTextFile(IFileSystemService& fileSystemService, const std::string& pathUtf8, const std::string& content) {
    WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    return fileSystemService.WriteTextFile(pathUtf8, content, options);
}

StatusOr<Json> ReadJsonFile(IFileSystemService& fileSystemService, const std::string& pathUtf8) {
    const auto fileText = ReadTextFile(fileSystemService, pathUtf8);
    if (!fileText.ok()) {
        return StatusOr<Json>::FromStatus(fileText.status);
    }
    try {
        const Json parsed = Json::parse(*fileText.value);
        return StatusOr<Json>{Status::Ok(), parsed};
    } catch (const std::exception& ex) {
        return StatusOr<Json>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Failed parsing JSON from " + pathUtf8 + ": " + ex.what()});
    }
}

std::string ExtensionTypeToString(ExtensionType type) {
    switch (type) {
        case ExtensionType::kLanguagePack:
            return "language-pack";
        case ExtensionType::kCommandPlugin:
            return "command-plugin";
        case ExtensionType::kToolIntegration:
            return "tool-integration";
    }
    return "command-plugin";
}

StatusOr<ExtensionType> ExtensionTypeFromString(const std::string& typeText) {
    if (typeText == "language-pack") {
        return StatusOr<ExtensionType>{Status::Ok(), ExtensionType::kLanguagePack};
    }
    if (typeText == "command-plugin") {
        return StatusOr<ExtensionType>{Status::Ok(), ExtensionType::kCommandPlugin};
    }
    if (typeText == "tool-integration") {
        return StatusOr<ExtensionType>{Status::Ok(), ExtensionType::kToolIntegration};
    }
    return StatusOr<ExtensionType>::FromStatus(
        MakeInvalidStatus("Unsupported extension type: " + typeText));
}

std::string PermissionGrantModeToString(PermissionGrantMode mode) {
    switch (mode) {
        case PermissionGrantMode::kDenied:
            return "denied";
        case PermissionGrantMode::kAllowOnce:
            return "once";
        case PermissionGrantMode::kAllowSession:
            return "session";
        case PermissionGrantMode::kAllowAlways:
            return "always";
    }
    return "denied";
}

std::vector<std::string> JsonStringArray(const Json& root, const char* key) {
    std::vector<std::string> out;
    const auto it = root.find(key);
    if (it == root.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (!item.is_string()) {
            continue;
        }
        out.push_back(item.get<std::string>());
    }
    return out;
}

Status ValidateExtensionId(const std::string& extensionId) {
    if (extensionId.empty()) {
        return MakeInvalidStatus("Extension id must not be empty");
    }
    for (char ch : extensionId) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch) != 0 || ch == '.' || ch == '-' || ch == '_') {
            continue;
        }
        return MakeInvalidStatus("Extension id contains unsupported character: " + std::string(1, ch));
    }
    return Status::Ok();
}

}  // namespace

LinuxExtensionService::LinuxExtensionService(
    IPathService* pathService,
    IFileSystemService* fileSystemService,
    std::string appName)
    : _pathService(pathService), _fileSystemService(fileSystemService), _appName(std::move(appName)) {}

void LinuxExtensionService::SetPermissionPrompt(PermissionPromptCallback callback) {
    _permissionPrompt = std::move(callback);
}

Status LinuxExtensionService::EnsureServiceReady() const {
    if (_pathService == nullptr || _fileSystemService == nullptr) {
        return Status{StatusCode::kUnavailable, "Extension service dependencies are not available"};
    }
    if (_appName.empty()) {
        return MakeInvalidStatus("Extension service app name must not be empty");
    }
    return Status::Ok();
}

std::string LinuxExtensionService::ExtensionsRootPath() const {
    const auto dataPath = _pathService->GetAppPath(PathScope::kData, _appName);
    if (!dataPath.ok()) {
        return {};
    }
    return *dataPath.value + "/extensions";
}

std::string LinuxExtensionService::ExtensionInstallPath(const std::string& extensionId) const {
    return ExtensionsRootPath() + "/" + extensionId;
}

std::string LinuxExtensionService::ExtensionStateFilePath() const {
    const auto configPath = _pathService->GetAppPath(PathScope::kConfig, _appName);
    if (!configPath.ok()) {
        return {};
    }
    return *configPath.value + "/extensions-state.json";
}

std::string LinuxExtensionService::ExtensionPermissionFilePath() const {
    const auto configPath = _pathService->GetAppPath(PathScope::kConfig, _appName);
    if (!configPath.ok()) {
        return {};
    }
    return *configPath.value + "/extensions-permissions.json";
}

Status LinuxExtensionService::EnsureBaseDirectories() {
    const Status readyStatus = EnsureServiceReady();
    if (!readyStatus.ok()) {
        return readyStatus;
    }

    const auto dataPath = _pathService->GetAppPath(PathScope::kData, _appName);
    const auto configPath = _pathService->GetAppPath(PathScope::kConfig, _appName);
    if (!dataPath.ok() || !configPath.ok()) {
        return Status{
            StatusCode::kUnavailable,
            "Failed resolving extension data/config paths"};
    }

    Status status = _fileSystemService->CreateDirectories(*dataPath.value);
    if (!status.ok()) {
        return status;
    }
    status = _fileSystemService->CreateDirectories(*configPath.value);
    if (!status.ok()) {
        return status;
    }
    status = _fileSystemService->CreateDirectories(ExtensionsRootPath());
    if (!status.ok()) {
        return status;
    }

    const auto stateExists = _fileSystemService->Exists(ExtensionStateFilePath());
    if (!stateExists.ok()) {
        return stateExists.status;
    }
    if (!(*stateExists.value)) {
        status = WriteTextFile(
            *_fileSystemService,
            ExtensionStateFilePath(),
            "{\n  \"schemaVersion\": 1,\n  \"extensions\": {}\n}\n");
        if (!status.ok()) {
            return status;
        }
    }

    const auto permissionExists = _fileSystemService->Exists(ExtensionPermissionFilePath());
    if (!permissionExists.ok()) {
        return permissionExists.status;
    }
    if (!(*permissionExists.value)) {
        status = WriteTextFile(
            *_fileSystemService,
            ExtensionPermissionFilePath(),
            "{\n  \"schemaVersion\": 1,\n  \"extensions\": {}\n}\n");
        if (!status.ok()) {
            return status;
        }
    }

    return Status::Ok();
}

Status LinuxExtensionService::Initialize() {
    return EnsureBaseDirectories();
}

StatusOr<ExtensionManifest> LinuxExtensionService::LoadManifestFromDirectory(
    const std::string& directoryUtf8) const {
    const Status readyStatus = EnsureServiceReady();
    if (!readyStatus.ok()) {
        return StatusOr<ExtensionManifest>::FromStatus(readyStatus);
    }
    if (directoryUtf8.empty()) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("Manifest directory path must not be empty"));
    }

    const std::filesystem::path directory = std::filesystem::path(directoryUtf8);
    const std::filesystem::path manifestPath = directory / "extension.json";

    const auto manifestJson = ReadJsonFile(*_fileSystemService, manifestPath.string());
    if (!manifestJson.ok()) {
        return StatusOr<ExtensionManifest>::FromStatus(manifestJson.status);
    }
    if (!manifestJson.value->is_object()) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("extension.json root must be an object"));
    }

    const Json& root = *manifestJson.value;
    const auto schemaVersionIt = root.find("schemaVersion");
    const auto idIt = root.find("id");
    const auto nameIt = root.find("name");
    const auto versionIt = root.find("version");
    const auto typeIt = root.find("type");
    const auto apiVersionIt = root.find("apiVersion");
    if (schemaVersionIt == root.end() || !schemaVersionIt->is_number_integer() ||
        idIt == root.end() || !idIt->is_string() ||
        nameIt == root.end() || !nameIt->is_string() ||
        versionIt == root.end() || !versionIt->is_string() ||
        typeIt == root.end() || !typeIt->is_string() ||
        apiVersionIt == root.end() || !apiVersionIt->is_string()) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("extension.json missing required fields"));
    }

    ExtensionManifest manifest;
    manifest.schemaVersion = schemaVersionIt->get<int>();
    manifest.id = idIt->get<std::string>();
    manifest.name = nameIt->get<std::string>();
    manifest.version = versionIt->get<std::string>();
    manifest.apiVersion = apiVersionIt->get<std::string>();
    manifest.permissions = JsonStringArray(root, "permissions");
    manifest.categories = JsonStringArray(root, "categories");

    if (const auto descriptionIt = root.find("description"); descriptionIt != root.end() && descriptionIt->is_string()) {
        manifest.description = descriptionIt->get<std::string>();
    }
    if (const auto authorIt = root.find("author"); authorIt != root.end() && authorIt->is_string()) {
        manifest.author = authorIt->get<std::string>();
    }
    if (const auto homepageIt = root.find("homepage"); homepageIt != root.end() && homepageIt->is_string()) {
        manifest.homepage = homepageIt->get<std::string>();
    }
    if (const auto entrypointIt = root.find("entrypoint"); entrypointIt != root.end() && entrypointIt->is_string()) {
        manifest.entrypoint = entrypointIt->get<std::string>();
    }

    const auto parsedType = ExtensionTypeFromString(typeIt->get<std::string>());
    if (!parsedType.ok()) {
        return StatusOr<ExtensionManifest>::FromStatus(parsedType.status);
    }
    manifest.type = *parsedType.value;

    if (manifest.schemaVersion != 1) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("Unsupported extension schemaVersion: " + std::to_string(manifest.schemaVersion)));
    }
    const Status extensionIdStatus = ValidateExtensionId(manifest.id);
    if (!extensionIdStatus.ok()) {
        return StatusOr<ExtensionManifest>::FromStatus(extensionIdStatus);
    }
    if (manifest.apiVersion.rfind("1.", 0) != 0) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("Unsupported extension apiVersion: " + manifest.apiVersion));
    }
    if ((manifest.type == ExtensionType::kCommandPlugin || manifest.type == ExtensionType::kToolIntegration) &&
        manifest.entrypoint.empty()) {
        return StatusOr<ExtensionManifest>::FromStatus(
            MakeInvalidStatus("Executable extension types require entrypoint"));
    }

    return StatusOr<ExtensionManifest>{Status::Ok(), manifest};
}

Status LinuxExtensionService::CopyDirectoryRecursively(
    const std::string& sourceDirectoryUtf8,
    const std::string& destinationDirectoryUtf8) const {
    std::error_code ec;
    const std::filesystem::path source = std::filesystem::path(sourceDirectoryUtf8);
    const std::filesystem::path destination = std::filesystem::path(destinationDirectoryUtf8);

    if (!std::filesystem::exists(source, ec) || ec) {
        return Status{StatusCode::kNotFound, "Source directory does not exist: " + sourceDirectoryUtf8};
    }
    std::filesystem::create_directories(destination, ec);
    if (ec) {
        return Status{StatusCode::kIoError, "Failed creating destination directory: " + ec.message()};
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(source, ec)) {
        if (ec) {
            return Status{StatusCode::kIoError, "Failed traversing source directory: " + ec.message()};
        }
        const std::filesystem::path relative = std::filesystem::relative(entry.path(), source, ec);
        if (ec) {
            return Status{StatusCode::kIoError, "Failed computing relative path: " + ec.message()};
        }
        const std::filesystem::path target = destination / relative;
        if (entry.is_directory(ec)) {
            ec.clear();
            std::filesystem::create_directories(target, ec);
            if (ec) {
                return Status{StatusCode::kIoError, "Failed creating directory: " + target.string()};
            }
            continue;
        }
        if (entry.is_regular_file(ec)) {
            ec.clear();
            std::filesystem::create_directories(target.parent_path(), ec);
            ec.clear();
            std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) {
                return Status{StatusCode::kIoError, "Failed copying file: " + entry.path().string()};
            }
        }
    }
    return Status::Ok();
}

Status LinuxExtensionService::RemoveDirectoryRecursively(const std::string& directoryUtf8) const {
    if (directoryUtf8.empty()) {
        return MakeInvalidStatus("Directory path must not be empty");
    }
    std::error_code ec;
    const std::filesystem::path directory = std::filesystem::path(directoryUtf8);
    if (!std::filesystem::exists(directory, ec)) {
        return Status::Ok();
    }
    ec.clear();
    std::filesystem::remove_all(directory, ec);
    if (ec) {
        return Status{StatusCode::kIoError, "Failed removing directory " + directoryUtf8 + ": " + ec.message()};
    }
    return Status::Ok();
}

StatusOr<LinuxExtensionService::ExtensionStateEntry> LinuxExtensionService::ReadStateEntry(
    const std::string& extensionId) const {
    const auto stateJson = ReadJsonFile(*_fileSystemService, ExtensionStateFilePath());
    if (!stateJson.ok()) {
        return StatusOr<ExtensionStateEntry>::FromStatus(stateJson.status);
    }
    const Json& root = *stateJson.value;
    const auto extensionsIt = root.find("extensions");
    if (extensionsIt == root.end() || !extensionsIt->is_object()) {
        return StatusOr<ExtensionStateEntry>::FromStatus(
            MakeInvalidStatus("extensions-state.json missing object field: extensions"));
    }
    const auto entryIt = extensionsIt->find(extensionId);
    if (entryIt == extensionsIt->end() || !entryIt->is_object()) {
        return StatusOr<ExtensionStateEntry>::FromStatus(
            Status{StatusCode::kNotFound, "No state entry for extension: " + extensionId});
    }

    ExtensionStateEntry entry;
    if (const auto enabledIt = entryIt->find("enabled"); enabledIt != entryIt->end() && enabledIt->is_boolean()) {
        entry.enabled = enabledIt->get<bool>();
    }
    if (const auto versionIt = entryIt->find("version"); versionIt != entryIt->end() && versionIt->is_string()) {
        entry.version = versionIt->get<std::string>();
    }
    if (const auto installPathIt = entryIt->find("installPath"); installPathIt != entryIt->end() && installPathIt->is_string()) {
        entry.installPath = installPathIt->get<std::string>();
    }
    return StatusOr<ExtensionStateEntry>{Status::Ok(), entry};
}

Status LinuxExtensionService::WriteStateEntry(const std::string& extensionId, const ExtensionStateEntry& stateEntry) {
    const auto stateJson = ReadJsonFile(*_fileSystemService, ExtensionStateFilePath());
    if (!stateJson.ok()) {
        return stateJson.status;
    }

    Json root = *stateJson.value;
    if (!root.contains("extensions") || !root["extensions"].is_object()) {
        root["extensions"] = Json::object();
    }

    Json entry;
    entry["enabled"] = stateEntry.enabled;
    entry["version"] = stateEntry.version;
    entry["installPath"] = stateEntry.installPath;
    root["extensions"][extensionId] = std::move(entry);

    return WriteTextFile(*_fileSystemService, ExtensionStateFilePath(), root.dump(2) + "\n");
}

Status LinuxExtensionService::RemoveStateEntry(const std::string& extensionId) {
    const auto stateJson = ReadJsonFile(*_fileSystemService, ExtensionStateFilePath());
    if (!stateJson.ok()) {
        return stateJson.status;
    }
    Json root = *stateJson.value;
    if (!root.contains("extensions") || !root["extensions"].is_object()) {
        root["extensions"] = Json::object();
    }
    root["extensions"].erase(extensionId);
    return WriteTextFile(*_fileSystemService, ExtensionStateFilePath(), root.dump(2) + "\n");
}

StatusOr<std::set<std::string>> LinuxExtensionService::ReadPersistedPermissions(
    const std::string& extensionId) const {
    const auto permissionJson = ReadJsonFile(*_fileSystemService, ExtensionPermissionFilePath());
    if (!permissionJson.ok()) {
        return StatusOr<std::set<std::string>>::FromStatus(permissionJson.status);
    }

    std::set<std::string> granted;
    const Json& root = *permissionJson.value;
    const auto extensionsIt = root.find("extensions");
    if (extensionsIt == root.end() || !extensionsIt->is_object()) {
        return StatusOr<std::set<std::string>>{Status::Ok(), granted};
    }

    const auto extensionEntryIt = extensionsIt->find(extensionId);
    if (extensionEntryIt == extensionsIt->end() || !extensionEntryIt->is_object()) {
        return StatusOr<std::set<std::string>>{Status::Ok(), granted};
    }

    const auto permissionsIt = extensionEntryIt->find("permissions");
    if (permissionsIt == extensionEntryIt->end() || !permissionsIt->is_object()) {
        return StatusOr<std::set<std::string>>{Status::Ok(), granted};
    }

    for (auto it = permissionsIt->begin(); it != permissionsIt->end(); ++it) {
        if (!it.value().is_object()) {
            continue;
        }
        const auto modeIt = it.value().find("mode");
        if (modeIt == it.value().end() || !modeIt->is_string()) {
            continue;
        }
        if (modeIt->get<std::string>() == "always") {
            granted.insert(it.key());
        }
    }
    return StatusOr<std::set<std::string>>{Status::Ok(), granted};
}

Status LinuxExtensionService::WritePersistedPermission(
    const std::string& extensionId,
    const std::string& permission,
    PermissionGrantMode grantMode) {
    const auto permissionJson = ReadJsonFile(*_fileSystemService, ExtensionPermissionFilePath());
    if (!permissionJson.ok()) {
        return permissionJson.status;
    }
    Json root = *permissionJson.value;
    if (!root.contains("extensions") || !root["extensions"].is_object()) {
        root["extensions"] = Json::object();
    }
    if (!root["extensions"].contains(extensionId) || !root["extensions"][extensionId].is_object()) {
        root["extensions"][extensionId] = Json::object();
    }
    if (!root["extensions"][extensionId].contains("permissions") ||
        !root["extensions"][extensionId]["permissions"].is_object()) {
        root["extensions"][extensionId]["permissions"] = Json::object();
    }

    Json entry;
    entry["mode"] = PermissionGrantModeToString(grantMode);
    root["extensions"][extensionId]["permissions"][permission] = std::move(entry);

    return WriteTextFile(*_fileSystemService, ExtensionPermissionFilePath(), root.dump(2) + "\n");
}

Status LinuxExtensionService::RemovePersistedPermissions(const std::string& extensionId) {
    const auto permissionJson = ReadJsonFile(*_fileSystemService, ExtensionPermissionFilePath());
    if (!permissionJson.ok()) {
        return permissionJson.status;
    }
    Json root = *permissionJson.value;
    if (!root.contains("extensions") || !root["extensions"].is_object()) {
        root["extensions"] = Json::object();
    }
    root["extensions"].erase(extensionId);
    return WriteTextFile(*_fileSystemService, ExtensionPermissionFilePath(), root.dump(2) + "\n");
}

bool LinuxExtensionService::RequiresInteractivePermissionPrompt(const std::string& permission) {
    if (permission == "network.client" || permission == "process.spawn") {
        return true;
    }
    if (permission.rfind("filesystem.read:", 0) == 0 || permission.rfind("filesystem.write:", 0) == 0) {
        return true;
    }
    return false;
}

StatusOr<std::vector<InstalledExtension>> LinuxExtensionService::DiscoverInstalled() const {
    const Status readyStatus = EnsureServiceReady();
    if (!readyStatus.ok()) {
        return StatusOr<std::vector<InstalledExtension>>::FromStatus(readyStatus);
    }

    const auto exists = _fileSystemService->Exists(ExtensionsRootPath());
    if (!exists.ok()) {
        return StatusOr<std::vector<InstalledExtension>>::FromStatus(exists.status);
    }
    if (!(*exists.value)) {
        return StatusOr<std::vector<InstalledExtension>>{Status::Ok(), {}};
    }

    ListDirectoryOptions options;
    options.recursive = false;
    options.includeDirectories = true;
    const auto entries = _fileSystemService->ListDirectory(ExtensionsRootPath(), options);
    if (!entries.ok()) {
        return StatusOr<std::vector<InstalledExtension>>::FromStatus(entries.status);
    }

    std::vector<InstalledExtension> installed;
    for (const std::string& entry : *entries.value) {
        const auto isDir = _fileSystemService->IsDirectory(entry);
        if (!isDir.ok() || !(*isDir.value)) {
            continue;
        }

        const auto manifest = LoadManifestFromDirectory(entry);
        if (!manifest.ok()) {
            continue;
        }

        InstalledExtension extension;
        extension.manifest = *manifest.value;
        extension.installPath = entry;
        extension.enabled = true;

        const auto state = ReadStateEntry(extension.manifest.id);
        if (state.ok()) {
            extension.enabled = state.value->enabled;
            if (!state.value->installPath.empty()) {
                extension.installPath = state.value->installPath;
            }
        }

        installed.push_back(std::move(extension));
    }

    std::sort(installed.begin(), installed.end(), [](const InstalledExtension& left, const InstalledExtension& right) {
        return left.manifest.id < right.manifest.id;
    });

    return StatusOr<std::vector<InstalledExtension>>{Status::Ok(), installed};
}

Status LinuxExtensionService::InstallFromDirectory(const std::string& sourceDirectoryUtf8) {
    Status status = EnsureBaseDirectories();
    if (!status.ok()) {
        return status;
    }

    const auto manifest = LoadManifestFromDirectory(sourceDirectoryUtf8);
    if (!manifest.ok()) {
        return manifest.status;
    }

    const std::string destinationPath = ExtensionInstallPath(manifest.value->id);
    status = RemoveDirectoryRecursively(destinationPath);
    if (!status.ok()) {
        return status;
    }
    status = CopyDirectoryRecursively(sourceDirectoryUtf8, destinationPath);
    if (!status.ok()) {
        return status;
    }

    const auto rollbackInstall = [this, &destinationPath, &manifest]() {
        static_cast<void>(RemoveDirectoryRecursively(destinationPath));
        static_cast<void>(RemoveStateEntry(manifest.value->id));
        static_cast<void>(RemovePersistedPermissions(manifest.value->id));
        _sessionGrantedPermissions.erase(manifest.value->id);
    };

    if (manifest.value->type == ExtensionType::kLanguagePack) {
        const auto vscodeSnapshot = LoadVscodeLanguagePackFromDirectory(destinationPath);
        if (vscodeSnapshot.ok()) {
            status = WriteVscodeLanguagePackSnapshot(
                destinationPath + "/npp-language-pack-vscode.json",
                *vscodeSnapshot.value);
            if (!status.ok()) {
                rollbackInstall();
                return status;
            }
        }
    }

    for (const std::string& permission : manifest.value->permissions) {
        if (!RequiresInteractivePermissionPrompt(permission)) {
            continue;
        }
        const auto permissionGranted = RequestPermission(manifest.value->id, permission, "install");
        if (!permissionGranted.ok()) {
            rollbackInstall();
            return permissionGranted.status;
        }
    }

    ExtensionStateEntry stateEntry;
    stateEntry.enabled = true;
    stateEntry.version = manifest.value->version;
    stateEntry.installPath = destinationPath;
    status = WriteStateEntry(manifest.value->id, stateEntry);
    if (!status.ok()) {
        rollbackInstall();
        return status;
    }
    return status;
}

Status LinuxExtensionService::EnableExtension(const std::string& extensionId) {
    const auto state = ReadStateEntry(extensionId);
    if (!state.ok()) {
        return state.status;
    }
    ExtensionStateEntry updated = *state.value;
    updated.enabled = true;
    return WriteStateEntry(extensionId, updated);
}

Status LinuxExtensionService::DisableExtension(const std::string& extensionId) {
    const auto state = ReadStateEntry(extensionId);
    if (!state.ok()) {
        return state.status;
    }
    ExtensionStateEntry updated = *state.value;
    updated.enabled = false;
    return WriteStateEntry(extensionId, updated);
}

Status LinuxExtensionService::ResetPermissions(const std::string& extensionId) {
    const Status extensionIdStatus = ValidateExtensionId(extensionId);
    if (!extensionIdStatus.ok()) {
        return extensionIdStatus;
    }

    const auto exists = _fileSystemService->Exists(ExtensionInstallPath(extensionId));
    if (!exists.ok()) {
        return exists.status;
    }
    if (!(*exists.value)) {
        return Status{StatusCode::kNotFound, "Extension not installed: " + extensionId};
    }

    _sessionGrantedPermissions.erase(extensionId);
    return RemovePersistedPermissions(extensionId);
}

Status LinuxExtensionService::RemoveExtension(const std::string& extensionId) {
    const Status extensionIdStatus = ValidateExtensionId(extensionId);
    if (!extensionIdStatus.ok()) {
        return extensionIdStatus;
    }

    const auto state = ReadStateEntry(extensionId);
    if (!state.ok() && state.status.code != StatusCode::kNotFound) {
        return state.status;
    }

    const std::string installPath = state.ok() && !state.value->installPath.empty()
        ? state.value->installPath
        : ExtensionInstallPath(extensionId);
    Status status = RemoveDirectoryRecursively(installPath);
    if (!status.ok()) {
        return status;
    }

    status = RemoveStateEntry(extensionId);
    if (!status.ok()) {
        return status;
    }
    status = RemovePersistedPermissions(extensionId);
    if (!status.ok()) {
        return status;
    }
    _sessionGrantedPermissions.erase(extensionId);
    return Status::Ok();
}

StatusOr<bool> LinuxExtensionService::IsPermissionGranted(
    const std::string& extensionId,
    const std::string& permission) const {
    if (extensionId.empty() || permission.empty()) {
        return StatusOr<bool>::FromStatus(MakeInvalidStatus("extensionId and permission must not be empty"));
    }

    const auto persisted = ReadPersistedPermissions(extensionId);
    if (!persisted.ok()) {
        return StatusOr<bool>::FromStatus(persisted.status);
    }
    if (persisted.value->find(permission) != persisted.value->end()) {
        return StatusOr<bool>{Status::Ok(), true};
    }

    const auto sessionIt = _sessionGrantedPermissions.find(extensionId);
    if (sessionIt != _sessionGrantedPermissions.end() &&
        sessionIt->second.find(permission) != sessionIt->second.end()) {
        return StatusOr<bool>{Status::Ok(), true};
    }

    return StatusOr<bool>{Status::Ok(), false};
}

StatusOr<bool> LinuxExtensionService::RequestPermission(
    const std::string& extensionId,
    const std::string& permission,
    const std::string& reason) {
    if (extensionId.empty() || permission.empty()) {
        return StatusOr<bool>::FromStatus(MakeInvalidStatus("extensionId and permission must not be empty"));
    }

    const auto manifest = LoadManifestFromDirectory(ExtensionInstallPath(extensionId));
    if (!manifest.ok()) {
        return StatusOr<bool>::FromStatus(manifest.status);
    }

    const bool declaredPermission =
        std::find(manifest.value->permissions.begin(), manifest.value->permissions.end(), permission) !=
        manifest.value->permissions.end();
    if (!declaredPermission) {
        return StatusOr<bool>::FromStatus(
            Status{StatusCode::kPermissionDenied, "Extension did not declare permission: " + permission});
    }

    const auto currentGrant = IsPermissionGranted(extensionId, permission);
    if (!currentGrant.ok()) {
        return StatusOr<bool>::FromStatus(currentGrant.status);
    }
    if (*currentGrant.value) {
        return StatusOr<bool>{Status::Ok(), true};
    }

    PermissionGrantMode grantMode = PermissionGrantMode::kDenied;
    if (_permissionPrompt) {
        PermissionPromptRequest request;
        request.extensionId = extensionId;
        request.extensionName = manifest.value->name;
        request.permission = permission;
        request.reason = reason;
        grantMode = _permissionPrompt(request);
    }

    if (grantMode == PermissionGrantMode::kAllowOnce || grantMode == PermissionGrantMode::kAllowSession) {
        _sessionGrantedPermissions[extensionId].insert(permission);
        return StatusOr<bool>{Status::Ok(), true};
    }
    if (grantMode == PermissionGrantMode::kAllowAlways) {
        Status persistStatus = WritePersistedPermission(extensionId, permission, grantMode);
        if (!persistStatus.ok()) {
            return StatusOr<bool>::FromStatus(persistStatus);
        }
        return StatusOr<bool>{Status::Ok(), true};
    }
    return StatusOr<bool>::FromStatus(
        Status{
            StatusCode::kPermissionDenied,
            "Permission denied for extension '" + extensionId + "': " + permission + " (" + reason + ")"});
}

}  // namespace npp::platform
