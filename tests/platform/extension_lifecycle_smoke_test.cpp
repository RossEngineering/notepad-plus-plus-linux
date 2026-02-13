#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "LinuxExtensionService.h"
#include "LinuxFileSystemService.h"
#include "LinuxPathService.h"

namespace {

bool WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) {
            return false;
        }
    }
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        return false;
    }
    stream << content;
    return stream.good();
}

void SetXdgPaths(const std::filesystem::path& root) {
    setenv("XDG_DATA_HOME", (root / "data").string().c_str(), 1);
    setenv("XDG_CONFIG_HOME", (root / "config").string().c_str(), 1);
    setenv("XDG_STATE_HOME", (root / "state").string().c_str(), 1);
    setenv("XDG_CACHE_HOME", (root / "cache").string().c_str(), 1);
}

}  // namespace

int main() {
    namespace fs = std::filesystem;
    using namespace npp::platform;

    const fs::path tempRoot =
        fs::temp_directory_path() / ("npp-extension-smoke-" + std::to_string(getpid()));
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);
    if (ec) {
        std::cerr << "failed creating temp root: " << ec.message() << "\n";
        return 1;
    }
    SetXdgPaths(tempRoot);

    LinuxPathService pathService;
    LinuxFileSystemService fsService;
    LinuxExtensionService extensionService(&pathService, &fsService, "npp-extension-smoke");
    extensionService.SetPermissionPrompt([](const PermissionPromptRequest& request) {
        if (request.extensionId == "ross.sample.denied" && request.permission == "process.spawn") {
            return PermissionGrantMode::kDenied;
        }
        if (request.permission == "process.spawn") {
            return PermissionGrantMode::kAllowAlways;
        }
        return PermissionGrantMode::kDenied;
    });

    if (!extensionService.Initialize().ok()) {
        std::cerr << "extension service init failed\n";
        return 2;
    }
    const auto dataRoot = pathService.GetAppPath(PathScope::kData, "npp-extension-smoke");
    if (!dataRoot.ok()) {
        std::cerr << "failed resolving data root\n";
        return 3;
    }

    const fs::path sampleExtensionDir = tempRoot / "source" / "ross.sample.formatter";
    if (!WriteTextFile(
            sampleExtensionDir / "extension.json",
            "{\n"
            "  \"schemaVersion\": 1,\n"
            "  \"id\": \"ross.sample.formatter\",\n"
            "  \"name\": \"Sample Formatter\",\n"
            "  \"version\": \"0.1.0\",\n"
            "  \"type\": \"command-plugin\",\n"
            "  \"apiVersion\": \"1.0\",\n"
            "  \"entrypoint\": \"bin/formatter\",\n"
            "  \"permissions\": [\"workspace.read\", \"process.spawn\"]\n"
            "}\n")) {
        std::cerr << "failed writing sample extension manifest\n";
        return 4;
    }
    if (!WriteTextFile(sampleExtensionDir / "README.md", "# sample\n")) {
        std::cerr << "failed writing sample extension readme\n";
        return 5;
    }

    if (!extensionService.InstallFromDirectory(sampleExtensionDir.string()).ok()) {
        std::cerr << "failed installing sample extension\n";
        return 6;
    }

    const auto discoveredAfterInstall = extensionService.DiscoverInstalled();
    if (!discoveredAfterInstall.ok() || discoveredAfterInstall.value->size() != 1) {
        std::cerr << "discover after install failed\n";
        return 7;
    }
    if (discoveredAfterInstall.value->front().manifest.id != "ross.sample.formatter" ||
        !discoveredAfterInstall.value->front().enabled) {
        std::cerr << "unexpected installed extension metadata\n";
        return 8;
    }

    const auto processPermission = extensionService.IsPermissionGranted(
        "ross.sample.formatter",
        "process.spawn");
    if (!processPermission.ok() || !(*processPermission.value)) {
        std::cerr << "expected process.spawn permission to be granted\n";
        return 9;
    }

    if (!extensionService.ResetPermissions("ross.sample.formatter").ok()) {
        std::cerr << "expected permission reset to succeed\n";
        return 10;
    }
    const auto processPermissionAfterReset = extensionService.IsPermissionGranted(
        "ross.sample.formatter",
        "process.spawn");
    if (!processPermissionAfterReset.ok() || *processPermissionAfterReset.value) {
        std::cerr << "expected process.spawn permission to be cleared after reset\n";
        return 11;
    }
    const auto processPermissionRegrant = extensionService.RequestPermission(
        "ross.sample.formatter",
        "process.spawn",
        "runtime");
    if (!processPermissionRegrant.ok() || !(*processPermissionRegrant.value)) {
        std::cerr << "expected process.spawn permission to be regranted after reset\n";
        return 12;
    }

    const fs::path deniedExtensionDir = tempRoot / "source" / "ross.sample.denied";
    if (!WriteTextFile(
            deniedExtensionDir / "extension.json",
            "{\n"
            "  \"schemaVersion\": 1,\n"
            "  \"id\": \"ross.sample.denied\",\n"
            "  \"name\": \"Denied Extension\",\n"
            "  \"version\": \"0.1.0\",\n"
            "  \"type\": \"command-plugin\",\n"
            "  \"apiVersion\": \"1.0\",\n"
            "  \"entrypoint\": \"bin/denied\",\n"
            "  \"permissions\": [\"process.spawn\"]\n"
            "}\n")) {
        std::cerr << "failed writing denied extension manifest\n";
        return 13;
    }
    const auto deniedInstallStatus = extensionService.InstallFromDirectory(deniedExtensionDir.string());
    if (deniedInstallStatus.ok()) {
        std::cerr << "expected denied extension install to fail\n";
        return 14;
    }
    if (deniedInstallStatus.code != StatusCode::kPermissionDenied) {
        std::cerr << "expected permission denied status when extension permission is denied\n";
        return 15;
    }
    const fs::path deniedInstallPath = fs::path(*dataRoot.value) / "extensions/ross.sample.denied";
    if (fs::exists(deniedInstallPath)) {
        std::cerr << "expected denied extension install rollback to remove copied files\n";
        return 16;
    }

    if (!extensionService.DisableExtension("ross.sample.formatter").ok()) {
        std::cerr << "disable failed\n";
        return 17;
    }
    const auto discoveredAfterDisable = extensionService.DiscoverInstalled();
    if (!discoveredAfterDisable.ok() || discoveredAfterDisable.value->empty() ||
        discoveredAfterDisable.value->front().enabled) {
        std::cerr << "disable state not persisted\n";
        return 18;
    }

    if (!extensionService.EnableExtension("ross.sample.formatter").ok()) {
        std::cerr << "enable failed\n";
        return 19;
    }

    const fs::path languagePackDir = tempRoot / "source" / "ross.language.python";
    if (!WriteTextFile(
            languagePackDir / "extension.json",
            "{\n"
            "  \"schemaVersion\": 1,\n"
            "  \"id\": \"ross.language.python\",\n"
            "  \"name\": \"Python Language Pack\",\n"
            "  \"version\": \"0.1.0\",\n"
            "  \"type\": \"language-pack\",\n"
            "  \"apiVersion\": \"1.0\"\n"
            "}\n")) {
        std::cerr << "failed writing language pack manifest\n";
        return 20;
    }
    if (!WriteTextFile(
            languagePackDir / "package.json",
            "{\n"
            "  \"name\": \"python-language-pack\",\n"
            "  \"contributes\": {\n"
            "    \"languages\": [\n"
            "      {\n"
            "        \"id\": \"python\",\n"
            "        \"extensions\": [\".py\"],\n"
            "        \"configuration\": \"./language-configuration.json\"\n"
            "      }\n"
            "    ],\n"
            "    \"grammars\": [\n"
            "      {\n"
            "        \"language\": \"python\",\n"
            "        \"scopeName\": \"source.python\",\n"
            "        \"path\": \"./syntaxes/python.tmLanguage.json\"\n"
            "      }\n"
            "    ]\n"
            "  }\n"
            "}\n")) {
        std::cerr << "failed writing language pack package.json\n";
        return 21;
    }
    if (!WriteTextFile(languagePackDir / "language-configuration.json", "{ \"comments\": {} }\n")) {
        std::cerr << "failed writing language config\n";
        return 22;
    }
    if (!WriteTextFile(languagePackDir / "syntaxes" / "python.tmLanguage.json", "{ \"scopeName\": \"source.python\" }\n")) {
        std::cerr << "failed writing grammar file\n";
        return 23;
    }

    if (!extensionService.InstallFromDirectory(languagePackDir.string()).ok()) {
        std::cerr << "language pack install failed\n";
        return 24;
    }
    const fs::path generatedSnapshot =
        fs::path(*dataRoot.value) /
        "extensions/ross.language.python/npp-language-pack-vscode.json";
    if (!fs::exists(generatedSnapshot)) {
        std::cerr << "expected generated VS Code compatibility snapshot\n";
        return 25;
    }

    if (!extensionService.RemoveExtension("ross.sample.formatter").ok()) {
        std::cerr << "remove sample extension failed\n";
        return 26;
    }
    if (!extensionService.RemoveExtension("ross.language.python").ok()) {
        std::cerr << "remove language pack extension failed\n";
        return 27;
    }

    const auto discoveredAfterRemove = extensionService.DiscoverInstalled();
    if (!discoveredAfterRemove.ok() || !discoveredAfterRemove.value->empty()) {
        std::cerr << "expected no installed extensions after removals\n";
        return 28;
    }

    fs::remove_all(tempRoot, ec);
    return 0;
}
