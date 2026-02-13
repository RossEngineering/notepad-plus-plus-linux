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

bool WriteFile(const std::filesystem::path& path, const std::string& content) {
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
        if (request.permission == "process.spawn") {
            return PermissionGrantMode::kAllowAlways;
        }
        return PermissionGrantMode::kDenied;
    });

    if (!extensionService.Initialize().ok()) {
        std::cerr << "extension service init failed\n";
        return 2;
    }

    const fs::path sampleExtensionDir = tempRoot / "source" / "ross.sample.formatter";
    if (!WriteFile(
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
        return 3;
    }
    if (!WriteFile(sampleExtensionDir / "README.md", "# sample\n")) {
        std::cerr << "failed writing sample extension readme\n";
        return 4;
    }

    if (!extensionService.InstallFromDirectory(sampleExtensionDir.string()).ok()) {
        std::cerr << "failed installing sample extension\n";
        return 5;
    }

    const auto discoveredAfterInstall = extensionService.DiscoverInstalled();
    if (!discoveredAfterInstall.ok() || discoveredAfterInstall.value->size() != 1) {
        std::cerr << "discover after install failed\n";
        return 6;
    }
    if (discoveredAfterInstall.value->front().manifest.id != "ross.sample.formatter" ||
        !discoveredAfterInstall.value->front().enabled) {
        std::cerr << "unexpected installed extension metadata\n";
        return 7;
    }

    const auto processPermission = extensionService.IsPermissionGranted(
        "ross.sample.formatter",
        "process.spawn");
    if (!processPermission.ok() || !(*processPermission.value)) {
        std::cerr << "expected process.spawn permission to be granted\n";
        return 8;
    }

    if (!extensionService.DisableExtension("ross.sample.formatter").ok()) {
        std::cerr << "disable failed\n";
        return 9;
    }
    const auto discoveredAfterDisable = extensionService.DiscoverInstalled();
    if (!discoveredAfterDisable.ok() || discoveredAfterDisable.value->empty() ||
        discoveredAfterDisable.value->front().enabled) {
        std::cerr << "disable state not persisted\n";
        return 10;
    }

    if (!extensionService.EnableExtension("ross.sample.formatter").ok()) {
        std::cerr << "enable failed\n";
        return 11;
    }

    const fs::path languagePackDir = tempRoot / "source" / "ross.language.python";
    if (!WriteFile(
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
        return 12;
    }
    if (!WriteFile(
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
        return 13;
    }
    if (!WriteFile(languagePackDir / "language-configuration.json", "{ \"comments\": {} }\n")) {
        std::cerr << "failed writing language config\n";
        return 14;
    }
    if (!WriteFile(languagePackDir / "syntaxes" / "python.tmLanguage.json", "{ \"scopeName\": \"source.python\" }\n")) {
        std::cerr << "failed writing grammar file\n";
        return 15;
    }

    if (!extensionService.InstallFromDirectory(languagePackDir.string()).ok()) {
        std::cerr << "language pack install failed\n";
        return 16;
    }

    const auto dataRoot = pathService.GetAppPath(PathScope::kData, "npp-extension-smoke");
    if (!dataRoot.ok()) {
        std::cerr << "failed resolving data root\n";
        return 17;
    }
    const fs::path generatedSnapshot =
        fs::path(*dataRoot.value) /
        "extensions/ross.language.python/npp-language-pack-vscode.json";
    if (!fs::exists(generatedSnapshot)) {
        std::cerr << "expected generated VS Code compatibility snapshot\n";
        return 18;
    }

    if (!extensionService.RemoveExtension("ross.sample.formatter").ok()) {
        std::cerr << "remove sample extension failed\n";
        return 19;
    }
    if (!extensionService.RemoveExtension("ross.language.python").ok()) {
        std::cerr << "remove language pack extension failed\n";
        return 20;
    }

    const auto discoveredAfterRemove = extensionService.DiscoverInstalled();
    if (!discoveredAfterRemove.ok() || !discoveredAfterRemove.value->empty()) {
        std::cerr << "expected no installed extensions after removals\n";
        return 21;
    }

    fs::remove_all(tempRoot, ec);
    return 0;
}
