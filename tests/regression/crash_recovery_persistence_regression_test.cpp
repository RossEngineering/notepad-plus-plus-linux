#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>

#include "LinuxFileSystemService.h"
#include "LinuxPathService.h"

int main() {
    using namespace npp::platform;

    LinuxPathService pathService;
    LinuxFileSystemService fsService;

    const std::filesystem::path tempRoot =
        std::filesystem::temp_directory_path() /
        ("npp-crash-recovery-regression-" + std::to_string(getpid()));
    std::filesystem::remove_all(tempRoot);
    std::filesystem::create_directories(tempRoot);

    const std::string xdgConfigHome = (tempRoot / "xdg-config").string();
    setenv("XDG_CONFIG_HOME", xdgConfigHome.c_str(), 1);

    const auto configRoot = pathService.GetAppPath(PathScope::kConfig, "notepad-plus-plus-linux");
    if (!configRoot.ok()) {
        std::cerr << "GetAppPath failed: " << configRoot.status.message << "\n";
        return 1;
    }

    const std::string journalPath = *configRoot.value + "/crash-recovery-journal.json";

    WriteFileOptions writeOptions;
    writeOptions.atomic = true;
    writeOptions.createParentDirs = true;

    const std::string initialContent = R"({"schemaVersion":1,"tabs":[{"dirty":true}]})";
    if (!fsService.WriteTextFile(journalPath, initialContent, writeOptions).ok()) {
        std::cerr << "initial atomic write failed\n";
        return 2;
    }

    const auto existsAfterInitialWrite = fsService.Exists(journalPath);
    if (!existsAfterInitialWrite.ok() || !(*existsAfterInitialWrite.value)) {
        std::cerr << "journal should exist after first write\n";
        return 3;
    }

    const auto firstRead = fsService.ReadTextFile(journalPath);
    if (!firstRead.ok() || *firstRead.value != initialContent) {
        std::cerr << "first read mismatch\n";
        return 4;
    }

    const std::string updatedContent =
        R"({"schemaVersion":1,"tabs":[{"dirty":true},{"dirty":false}],"activeIndex":1})";
    if (!fsService.WriteTextFile(journalPath, updatedContent, writeOptions).ok()) {
        std::cerr << "update atomic write failed\n";
        return 5;
    }

    const auto secondRead = fsService.ReadTextFile(journalPath);
    if (!secondRead.ok() || *secondRead.value != updatedContent) {
        std::cerr << "second read mismatch\n";
        return 6;
    }

    ListDirectoryOptions listOptions;
    listOptions.recursive = true;
    listOptions.includeDirectories = true;
    const auto listed = fsService.ListDirectory(xdgConfigHome, listOptions);
    if (!listed.ok()) {
        std::cerr << "ListDirectory failed: " << listed.status.message << "\n";
        return 7;
    }
    for (const std::string &entry : *listed.value) {
        if (entry.find(".tmp.") != std::string::npos) {
            std::cerr << "unexpected temp file residue: " << entry << "\n";
            return 8;
        }
    }

    if (!fsService.RemoveFile(journalPath).ok()) {
        std::cerr << "RemoveFile failed\n";
        return 9;
    }

    const auto existsAfterRemove = fsService.Exists(journalPath);
    if (!existsAfterRemove.ok() || *existsAfterRemove.value) {
        std::cerr << "journal should not exist after remove\n";
        return 10;
    }

    std::filesystem::remove_all(tempRoot);
    return 0;
}
