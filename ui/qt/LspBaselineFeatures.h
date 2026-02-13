#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace npp::ui {

struct BaselineDiagnostic {
    int line = 0;
    std::string message;
};

std::string MapLexerToLspLanguageId(const std::string& lexerName);

std::string ExtractWordAt(const std::string& textUtf8, std::size_t caretByteOffset);

int FindBaselineDefinitionLine(const std::string& textUtf8, const std::string& symbol);

std::vector<BaselineDiagnostic> CollectBaselineDiagnostics(
    const std::string& textUtf8,
    std::size_t maxIssues = 64);

}  // namespace npp::ui
