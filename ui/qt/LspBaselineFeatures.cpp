#include "LspBaselineFeatures.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>
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

std::string_view TrimLeftAsciiWhitespace(std::string_view text) {
    size_t index = 0;
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index])) != 0) {
        ++index;
    }
    return text.substr(index);
}

std::string_view TrimRightAsciiWhitespace(std::string_view text) {
    size_t end = text.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }
    return text.substr(0, end);
}

std::string ExtractIdentifierAfterKeyword(std::string_view line, std::string_view keyword) {
    if (line.size() <= keyword.size() || line.substr(0, keyword.size()) != keyword) {
        return {};
    }
    size_t cursor = keyword.size();
    while (cursor < line.size() && std::isspace(static_cast<unsigned char>(line[cursor])) != 0) {
        ++cursor;
    }
    const size_t start = cursor;
    while (cursor < line.size() && IsIdentifierByte(static_cast<unsigned char>(line[cursor]))) {
        ++cursor;
    }
    if (cursor <= start) {
        return {};
    }
    return std::string(line.substr(start, cursor - start));
}

struct SplitLinesResult {
    std::vector<std::string> lines;
    bool trailingNewline = false;
};

SplitLinesResult SplitLines(const std::string& textUtf8) {
    SplitLinesResult split;
    if (textUtf8.empty()) {
        return split;
    }

    size_t lineStart = 0;
    for (size_t index = 0; index < textUtf8.size(); ++index) {
        if (textUtf8[index] != '\n') {
            continue;
        }
        split.lines.push_back(textUtf8.substr(lineStart, index - lineStart));
        lineStart = index + 1;
    }

    if (lineStart < textUtf8.size()) {
        split.lines.push_back(textUtf8.substr(lineStart));
    } else {
        split.trailingNewline = true;
    }
    return split;
}

std::string JoinLines(const SplitLinesResult& split) {
    std::string output;
    for (size_t index = 0; index < split.lines.size(); ++index) {
        output += split.lines[index];
        if (index + 1 < split.lines.size()) {
            output.push_back('\n');
        }
    }
    if (split.trailingNewline) {
        output.push_back('\n');
    }
    return output;
}

bool RemoveLineTrailingWhitespace(std::string* line) {
    if (line == nullptr) {
        return false;
    }
    const std::string_view trimmed = TrimRightAsciiWhitespace(*line);
    if (trimmed.size() == line->size()) {
        return false;
    }
    line->assign(trimmed.data(), trimmed.size());
    return true;
}

bool ReplaceTabsWithSpaces(std::string* line, int tabWidth) {
    if (line == nullptr || tabWidth <= 0) {
        return false;
    }

    bool changed = false;
    std::string transformed;
    transformed.reserve(line->size() + 8);
    int visualColumn = 0;
    for (char ch : *line) {
        if (ch == '\t') {
            const int spaces = tabWidth - (visualColumn % tabWidth);
            transformed.append(static_cast<size_t>(spaces), ' ');
            visualColumn += spaces;
            changed = true;
            continue;
        }
        transformed.push_back(ch);
        ++visualColumn;
    }
    if (changed) {
        *line = std::move(transformed);
    }
    return changed;
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

int RenameBaselineSymbol(
    std::string* textUtf8,
    const std::string& oldSymbol,
    const std::string& newSymbol,
    std::size_t maxReplacements) {
    if (textUtf8 == nullptr || oldSymbol.empty() || newSymbol.empty() || oldSymbol == newSymbol) {
        return 0;
    }

    std::size_t replacementCount = 0;
    std::size_t cursor = 0;
    while (cursor < textUtf8->size()) {
        const std::size_t found = textUtf8->find(oldSymbol, cursor);
        if (found == std::string::npos) {
            break;
        }

        const bool hasPrefix = found > 0;
        const bool hasSuffix = found + oldSymbol.size() < textUtf8->size();
        const bool prefixIsIdentifier = hasPrefix &&
            IsIdentifierByte(static_cast<unsigned char>((*textUtf8)[found - 1]));
        const bool suffixIsIdentifier = hasSuffix &&
            IsIdentifierByte(static_cast<unsigned char>((*textUtf8)[found + oldSymbol.size()]));
        if (prefixIsIdentifier || suffixIsIdentifier) {
            cursor = found + oldSymbol.size();
            continue;
        }

        textUtf8->replace(found, oldSymbol.size(), newSymbol);
        ++replacementCount;
        if (replacementCount >= maxReplacements) {
            break;
        }
        cursor = found + newSymbol.size();
    }

    return static_cast<int>(replacementCount);
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
            diagnostics.push_back(BaselineDiagnostic{
                lineNumber + 1,
                "warning",
                "trailing-whitespace",
                "trailing whitespace"});
        }
        if (diagnostics.size() >= maxIssues) {
            break;
        }

        if (line.find('\t') != std::string::npos) {
            diagnostics.push_back(BaselineDiagnostic{
                lineNumber + 1,
                "warning",
                "tab-character",
                "tab character detected"});
        }
        if (diagnostics.size() >= maxIssues) {
            break;
        }

        if (line.size() > 140) {
            diagnostics.push_back(BaselineDiagnostic{
                lineNumber + 1,
                "info",
                "line-too-long",
                "line exceeds 140 characters"});
        }

        ++lineNumber;
    }

    return diagnostics;
}

std::vector<BaselineDocumentSymbol> CollectBaselineDocumentSymbols(
    const std::string& textUtf8,
    std::size_t maxSymbols) {
    std::vector<BaselineDocumentSymbol> symbols;
    symbols.reserve(std::min<std::size_t>(maxSymbols, 64));

    std::istringstream lines(textUtf8);
    std::string line;
    int lineNumber = 0;
    while (std::getline(lines, line)) {
        ++lineNumber;
        if (symbols.size() >= maxSymbols) {
            break;
        }

        const std::string_view trimmed = TrimLeftAsciiWhitespace(line);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed[0] == '#') {
            size_t headingLevel = 0;
            while (headingLevel < trimmed.size() && trimmed[headingLevel] == '#') {
                ++headingLevel;
            }
            if (headingLevel > 0 &&
                headingLevel < trimmed.size() &&
                std::isspace(static_cast<unsigned char>(trimmed[headingLevel])) != 0) {
                const std::string_view headingName = TrimLeftAsciiWhitespace(trimmed.substr(headingLevel));
                if (!headingName.empty()) {
                    symbols.push_back(BaselineDocumentSymbol{
                        lineNumber,
                        "heading",
                        std::string(headingName)});
                    continue;
                }
            }
        }

        const std::array<std::pair<std::string_view, std::string_view>, 8> keywords = {
            std::pair<std::string_view, std::string_view>{"def", "function"},
            {"class", "class"},
            {"struct", "struct"},
            {"enum", "enum"},
            {"function", "function"},
            {"namespace", "namespace"},
            {"const", "constant"},
            {"let", "variable"},
        };
        bool matchedKeyword = false;
        for (const auto& keyword : keywords) {
            const std::string probe = std::string(keyword.first) + " ";
            const std::string identifier = ExtractIdentifierAfterKeyword(trimmed, probe);
            if (!identifier.empty()) {
                symbols.push_back(BaselineDocumentSymbol{
                    lineNumber,
                    std::string(keyword.second),
                    identifier});
                matchedKeyword = true;
                break;
            }
        }
        if (matchedKeyword || symbols.size() >= maxSymbols) {
            continue;
        }

        const std::size_t openParen = trimmed.find('(');
        const std::size_t openBrace = trimmed.find('{');
        if (openParen != std::string::npos &&
            openParen > 0 &&
            openBrace != std::string::npos &&
            openBrace > openParen) {
            std::size_t end = openParen;
            while (end > 0 && std::isspace(static_cast<unsigned char>(trimmed[end - 1])) != 0) {
                --end;
            }
            std::size_t start = end;
            while (start > 0 && IsIdentifierByte(static_cast<unsigned char>(trimmed[start - 1]))) {
                --start;
            }
            if (end > start) {
                symbols.push_back(BaselineDocumentSymbol{
                    lineNumber,
                    "function",
                    std::string(trimmed.substr(start, end - start))});
            }
        }
    }

    return symbols;
}

std::vector<BaselineCodeAction> CollectBaselineCodeActions(
    const std::vector<BaselineDiagnostic>& diagnostics,
    std::size_t maxActions) {
    std::vector<BaselineCodeAction> actions;
    actions.reserve(std::min<std::size_t>(maxActions, 32));

    bool hasTrailingWhitespace = false;
    bool hasTabs = false;
    std::set<int> trailingWhitespaceLines;
    std::set<int> tabLines;
    for (const BaselineDiagnostic& diagnostic : diagnostics) {
        if (diagnostic.code == "trailing-whitespace") {
            hasTrailingWhitespace = true;
            trailingWhitespaceLines.insert(diagnostic.line);
        } else if (diagnostic.code == "tab-character") {
            hasTabs = true;
            tabLines.insert(diagnostic.line);
        }
    }

    if (hasTrailingWhitespace) {
        actions.push_back(BaselineCodeAction{
            "fix.trim-trailing-whitespace.document",
            0,
            "Trim trailing whitespace in document"});
    }
    if (hasTabs && actions.size() < maxActions) {
        actions.push_back(BaselineCodeAction{
            "fix.tabs-to-spaces.document",
            0,
            "Convert tabs to spaces in document"});
    }

    for (int line : trailingWhitespaceLines) {
        if (actions.size() >= maxActions) {
            break;
        }
        actions.push_back(BaselineCodeAction{
            "fix.trim-trailing-whitespace.line",
            line,
            "Trim trailing whitespace on line " + std::to_string(line)});
    }
    for (int line : tabLines) {
        if (actions.size() >= maxActions) {
            break;
        }
        actions.push_back(BaselineCodeAction{
            "fix.tabs-to-spaces.line",
            line,
            "Convert tabs to spaces on line " + std::to_string(line)});
    }

    return actions;
}

bool ApplyBaselineCodeAction(
    std::string* textUtf8,
    const std::string& actionId,
    int line,
    int tabWidth) {
    if (textUtf8 == nullptr || actionId.empty()) {
        return false;
    }

    const int normalizedTabWidth = std::clamp(tabWidth, 1, 12);
    SplitLinesResult split = SplitLines(*textUtf8);
    if (split.lines.empty()) {
        return false;
    }

    bool changed = false;
    if (actionId == "fix.trim-trailing-whitespace.document") {
        for (std::string& textLine : split.lines) {
            changed = RemoveLineTrailingWhitespace(&textLine) || changed;
        }
    } else if (actionId == "fix.tabs-to-spaces.document") {
        for (std::string& textLine : split.lines) {
            changed = ReplaceTabsWithSpaces(&textLine, normalizedTabWidth) || changed;
        }
    } else if (actionId == "fix.trim-trailing-whitespace.line") {
        if (line <= 0 || line > static_cast<int>(split.lines.size())) {
            return false;
        }
        changed = RemoveLineTrailingWhitespace(&split.lines[static_cast<size_t>(line - 1)]);
    } else if (actionId == "fix.tabs-to-spaces.line") {
        if (line <= 0 || line > static_cast<int>(split.lines.size())) {
            return false;
        }
        changed = ReplaceTabsWithSpaces(&split.lines[static_cast<size_t>(line - 1)], normalizedTabWidth);
    } else {
        return false;
    }

    if (!changed) {
        return false;
    }

    *textUtf8 = JoinLines(split);
    return true;
}

}  // namespace npp::ui
