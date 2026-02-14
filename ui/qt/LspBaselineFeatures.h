#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace npp::ui {

struct BaselineDiagnostic {
    int line = 0;
    std::string severity;
    std::string code;
    std::string message;
};

struct BaselineDocumentSymbol {
    int line = 0;
    std::string kind;
    std::string name;
};

struct BaselineCodeAction {
    std::string id;
    int line = 0;
    std::string title;
};

std::string MapLexerToLspLanguageId(const std::string& lexerName);

std::string ExtractWordAt(const std::string& textUtf8, std::size_t caretByteOffset);

int FindBaselineDefinitionLine(const std::string& textUtf8, const std::string& symbol);

int RenameBaselineSymbol(
    std::string* textUtf8,
    const std::string& oldSymbol,
    const std::string& newSymbol,
    std::size_t maxReplacements = 10000);

std::vector<BaselineDiagnostic> CollectBaselineDiagnostics(
    const std::string& textUtf8,
    std::size_t maxIssues = 64);

std::vector<BaselineDocumentSymbol> CollectBaselineDocumentSymbols(
    const std::string& textUtf8,
    std::size_t maxSymbols = 256);

std::vector<BaselineCodeAction> CollectBaselineCodeActions(
    const std::vector<BaselineDiagnostic>& diagnostics,
    std::size_t maxActions = 64);

bool ApplyBaselineCodeAction(
    std::string* textUtf8,
    const std::string& actionId,
    int line,
    int tabWidth);

}  // namespace npp::ui
