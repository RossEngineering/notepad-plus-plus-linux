#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "VscodeLanguageCompatibility.h"

namespace {

struct Case {
    std::string extensionFolder;
    std::vector<std::string> expectedLanguageIds;
    std::vector<std::string> expectedScopeNames;
    std::vector<std::string> expectedLexers;
};

}  // namespace

int main() {
    namespace fs = std::filesystem;
    using namespace npp::platform;

    const fs::path sourceRoot = fs::path(NPP_SOURCE_DIR);
    const fs::path fixtureRoot = sourceRoot / "tests" / "fixtures" / "vscode-language-assets";

    const std::vector<Case> cases = {
        {"ms-python.python", {"python"}, {"source.python"}, {"python"}},
        {"golang.go", {"go"}, {"source.go"}, {"cpp"}},
        {"redhat.vscode-yaml", {"yaml"}, {"source.yaml"}, {"yaml"}},
        {"rust-lang.rust-analyzer", {"rust"}, {"source.rust"}, {"cpp"}},
        {"vscode.markdown-baseline", {"markdown"}, {"text.html.markdown"}, {"markdown"}},
        {"example.multi-grammar", {"html", "xml"}, {"text.html.basic", "text.xml"}, {"xml", "xml"}},
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
        if (testCase.expectedLanguageIds.size() != testCase.expectedLexers.size()) {
            std::cerr << "invalid test setup for " << testCase.extensionFolder << "\n";
            return 3;
        }
        std::set<std::string> actualLanguageIds;
        for (const auto& language : snapshot.value->languages) {
            actualLanguageIds.insert(language.id);
        }
        for (std::size_t i = 0; i < testCase.expectedLanguageIds.size(); ++i) {
            if (actualLanguageIds.find(testCase.expectedLanguageIds[i]) == actualLanguageIds.end()) {
                std::cerr << "missing expected language id for " << testCase.extensionFolder
                          << ": " << testCase.expectedLanguageIds[i] << "\n";
                return 4;
            }

            bool lexerMatch = false;
            for (const auto& language : snapshot.value->languages) {
                if (language.id == testCase.expectedLanguageIds[i] &&
                    language.suggestedLexer == testCase.expectedLexers[i]) {
                    lexerMatch = true;
                    break;
                }
            }
            if (!lexerMatch) {
                std::cerr << "unexpected lexer mapping for " << testCase.extensionFolder
                          << " language=" << testCase.expectedLanguageIds[i] << "\n";
                return 5;
            }
        }

        std::set<std::string> actualScopes;
        for (const auto& grammar : snapshot.value->grammars) {
            actualScopes.insert(grammar.scopeName);
        }
        for (const auto& expectedScope : testCase.expectedScopeNames) {
            if (actualScopes.find(expectedScope) == actualScopes.end()) {
                std::cerr << "missing expected scope for " << testCase.extensionFolder
                          << ": " << expectedScope << "\n";
                return 6;
            }
        }

        if (snapshot.value->languages.size() < testCase.expectedLanguageIds.size()) {
            std::cerr << "language contribution count too small for "
                      << testCase.extensionFolder << "\n";
            return 7;
        }
        if (snapshot.value->grammars.size() < testCase.expectedScopeNames.size()) {
            std::cerr << "grammar contribution count too small for "
                      << testCase.extensionFolder << "\n";
            return 8;
        }

        const fs::path outputPath = extensionPath / "npp-language-pack-vscode.test-output.json";
        const Status writeStatus = WriteVscodeLanguagePackSnapshot(outputPath.string(), *snapshot.value);
        if (!writeStatus.ok() || !fs::exists(outputPath)) {
            std::cerr << "failed writing compatibility snapshot output for "
                      << testCase.extensionFolder << "\n";
            return 9;
        }
        fs::remove(outputPath);
    }

    std::cout << "Validated VS Code language-asset compatibility fixtures: "
              << cases.size() << "\n";
    return 0;
}
