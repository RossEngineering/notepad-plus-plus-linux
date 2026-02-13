#include "LspBaselineFeatures.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>

namespace npp::ui {

namespace {

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool IsIdentifierByte(unsigned char ch) {
    return std::isalnum(ch) != 0 || ch == '_' || ch == '-';
}

}  // namespace

std::string MapLexerToLspLanguageId(const std::string& lexerName) {
    if (lexerName == "cpp") {
        return "cpp";
    }
    if (lexerName == "python") {
        return "python";
    }
    if (lexerName == "bash") {
        return "bash";
    }
    if (lexerName == "yaml") {
        return "yaml";
    }
    if (lexerName == "markdown") {
        return "markdown";
    }
    if (lexerName == "sql") {
        return "sql";
    }
    if (lexerName == "xml") {
        return "html";
    }
    if (lexerName == "json") {
        return "json";
    }
    return {};
}

std::string ExtractWordAt(const std::string& textUtf8, std::size_t caretByteOffset) {
    if (textUtf8.empty()) {
        return {};
    }
    if (caretByteOffset > textUtf8.size()) {
        caretByteOffset = textUtf8.size();
    }

    std::size_t start = caretByteOffset;
    while (start > 0 && IsIdentifierByte(static_cast<unsigned char>(textUtf8[start - 1]))) {
        --start;
    }

    std::size_t end = caretByteOffset;
    while (end < textUtf8.size() && IsIdentifierByte(static_cast<unsigned char>(textUtf8[end]))) {
        ++end;
    }

    if (end <= start) {
        return {};
    }
    return textUtf8.substr(start, end - start);
}

int FindBaselineDefinitionLine(const std::string& textUtf8, const std::string& symbol) {
    if (symbol.empty()) {
        return -1;
    }

    const std::string lowerSymbol = AsciiLower(symbol);
    int lineNumber = 0;
    std::istringstream lines(textUtf8);
    std::string line;
    while (std::getline(lines, line)) {
        const std::string lowerLine = AsciiLower(line);
        const std::array<std::string, 8> patterns = {
            "def " + lowerSymbol,
            "class " + lowerSymbol,
            "struct " + lowerSymbol,
            "enum " + lowerSymbol,
            "function " + lowerSymbol,
            "const " + lowerSymbol,
            "let " + lowerSymbol,
            "var " + lowerSymbol,
        };
        for (const auto& pattern : patterns) {
            if (lowerLine.find(pattern) != std::string::npos) {
                return lineNumber;
            }
        }

        const std::string functionPattern = lowerSymbol + "(";
        if (lowerLine.find(functionPattern) != std::string::npos &&
            lowerLine.find(';') == std::string::npos) {
            return lineNumber;
        }

        ++lineNumber;
    }
    return -1;
}

std::vector<BaselineDiagnostic> CollectBaselineDiagnostics(
    const std::string& textUtf8,
    std::size_t maxIssues) {
    std::vector<BaselineDiagnostic> diagnostics;
    diagnostics.reserve(std::min<std::size_t>(maxIssues, 32));

    std::istringstream lines(textUtf8);
    std::string line;
    int lineNumber = 0;
    while (std::getline(lines, line)) {
        if (diagnostics.size() >= maxIssues) {
            break;
        }

        if (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
            diagnostics.push_back(BaselineDiagnostic{lineNumber + 1, "trailing whitespace"});
        }
        if (diagnostics.size() >= maxIssues) {
            break;
        }

        if (line.find('\t') != std::string::npos) {
            diagnostics.push_back(BaselineDiagnostic{lineNumber + 1, "tab character detected"});
        }
        if (diagnostics.size() >= maxIssues) {
            break;
        }

        if (line.size() > 140) {
            diagnostics.push_back(BaselineDiagnostic{lineNumber + 1, "line exceeds 140 characters"});
        }

        ++lineNumber;
    }

    return diagnostics;
}

}  // namespace npp::ui
