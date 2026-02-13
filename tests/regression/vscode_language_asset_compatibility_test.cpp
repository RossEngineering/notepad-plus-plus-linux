#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "VscodeLanguageCompatibility.h"

namespace {

struct Case {
    std::string extensionFolder;
    std::string expectedLanguageId;
    std::string expectedScopeName;
    std::string expectedLexer;
};

}  // namespace

int main() {
    namespace fs = std::filesystem;
    using namespace npp::platform;

    const fs::path sourceRoot = fs::path(NPP_SOURCE_DIR);
    const fs::path fixtureRoot = sourceRoot / "tests" / "fixtures" / "vscode-language-assets";

    const std::vector<Case> cases = {
        {"ms-python.python", "python", "source.python", "python"},
        {"golang.go", "go", "source.go", "cpp"},
        {"redhat.vscode-yaml", "yaml", "source.yaml", "yaml"},
    };

    for (const Case& testCase : cases) {
        const fs::path extensionPath = fixtureRoot / testCase.extensionFolder;
        const auto snapshot = LoadVscodeLanguagePackFromDirectory(extensionPath.string());
        if (!snapshot.ok()) {
            std::cerr << "failed parsing VS Code extension fixture " << testCase.extensionFolder
                      << ": " << snapshot.status.message << "\n";
            return 1;
        }
        if (snapshot.value->languages.empty() || snapshot.value->grammars.empty()) {
            std::cerr << "empty language/grammar contributions for " << testCase.extensionFolder << "\n";
            return 2;
        }
        if (snapshot.value->languages.front().id != testCase.expectedLanguageId) {
            std::cerr << "unexpected language id for " << testCase.extensionFolder
                      << ": " << snapshot.value->languages.front().id << "\n";
            return 3;
        }
        if (snapshot.value->languages.front().suggestedLexer != testCase.expectedLexer) {
            std::cerr << "unexpected lexer mapping for " << testCase.extensionFolder
                      << ": " << snapshot.value->languages.front().suggestedLexer << "\n";
            return 4;
        }
        if (snapshot.value->grammars.front().scopeName != testCase.expectedScopeName) {
            std::cerr << "unexpected scope name for " << testCase.extensionFolder
                      << ": " << snapshot.value->grammars.front().scopeName << "\n";
            return 5;
        }

        const fs::path outputPath = extensionPath / "npp-language-pack-vscode.test-output.json";
        const Status writeStatus = WriteVscodeLanguagePackSnapshot(outputPath.string(), *snapshot.value);
        if (!writeStatus.ok() || !fs::exists(outputPath)) {
            std::cerr << "failed writing compatibility snapshot output for "
                      << testCase.extensionFolder << "\n";
            return 6;
        }
        fs::remove(outputPath);
    }

    std::cout << "Validated VS Code language-asset compatibility fixtures: "
              << cases.size() << "\n";
    return 0;
}
