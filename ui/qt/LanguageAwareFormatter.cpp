#include "LanguageAwareFormatter.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace npp::ui {

namespace {

std::string_view TrimLeftAsciiWhitespace(std::string_view text) {
    size_t index = 0;
    while (index < text.size() && (text[index] == ' ' || text[index] == '\t')) {
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

bool StartsWithKeywordToken(std::string_view text, std::string_view keyword) {
    if (text.size() < keyword.size()) {
        return false;
    }
    if (text.substr(0, keyword.size()) != keyword) {
        return false;
    }
    if (text.size() == keyword.size()) {
        return true;
    }
    const char next = text[keyword.size()];
    return next == ':' || std::isspace(static_cast<unsigned char>(next)) != 0;
}

bool IsPythonDedentKeyword(std::string_view trimmedLine) {
    return StartsWithKeywordToken(trimmedLine, "elif")
        || StartsWithKeywordToken(trimmedLine, "else")
        || StartsWithKeywordToken(trimmedLine, "except")
        || StartsWithKeywordToken(trimmedLine, "finally");
}

int LeadingIndentColumns(std::string_view line, int tabWidth) {
    int columns = 0;
    for (char ch : line) {
        if (ch == ' ') {
            ++columns;
            continue;
        }
        if (ch == '\t') {
            const int tabAdvance = tabWidth - (columns % tabWidth);
            columns += tabAdvance;
            continue;
        }
        break;
    }
    return columns;
}

struct SplitLinesResult {
    std::vector<std::string> lines;
    bool hasTrailingNewline = false;
};

SplitLinesResult SplitLines(std::string_view text) {
    SplitLinesResult result;
    size_t lineStart = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\n' && text[i] != '\r') {
            continue;
        }
        result.lines.emplace_back(text.substr(lineStart, i - lineStart));
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            ++i;
        }
        lineStart = i + 1;
    }
    if (lineStart < text.size()) {
        result.lines.emplace_back(text.substr(lineStart));
    } else {
        result.hasTrailingNewline = !text.empty();
        if (result.lines.empty()) {
            result.lines.emplace_back();
        }
    }
    return result;
}

std::string JoinLines(const std::vector<std::string>& lines, bool trailingNewline) {
    std::string output;
    for (size_t i = 0; i < lines.size(); ++i) {
        output += lines[i];
        if (i + 1 < lines.size()) {
            output.push_back('\n');
        }
    }
    if (trailingNewline) {
        output.push_back('\n');
    }
    return output;
}

struct PythonLineAnalysis {
    int parenDepthAfter = 0;
    bool opensBlock = false;
    bool explicitContinuation = false;
};

PythonLineAnalysis AnalyzePythonLine(std::string_view trimmedLine, int parenDepthBefore) {
    PythonLineAnalysis analysis;
    analysis.parenDepthAfter = parenDepthBefore;

    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escaping = false;
    char lastSignificant = '\0';

    for (size_t i = 0; i < trimmedLine.size(); ++i) {
        const char ch = trimmedLine[i];
        if (escaping) {
            escaping = false;
            continue;
        }

        if (inSingleQuote) {
            if (ch == '\\') {
                escaping = true;
            } else if (ch == '\'') {
                inSingleQuote = false;
            }
            continue;
        }
        if (inDoubleQuote) {
            if (ch == '\\') {
                escaping = true;
            } else if (ch == '"') {
                inDoubleQuote = false;
            }
            continue;
        }

        if (ch == '#') {
            break;
        }
        if (ch == '\'') {
            inSingleQuote = true;
            continue;
        }
        if (ch == '"') {
            inDoubleQuote = true;
            continue;
        }

        if (ch == '(' || ch == '[' || ch == '{') {
            ++analysis.parenDepthAfter;
        } else if (ch == ')' || ch == ']' || ch == '}') {
            analysis.parenDepthAfter = std::max(0, analysis.parenDepthAfter - 1);
        }

        if (std::isspace(static_cast<unsigned char>(ch)) == 0) {
            lastSignificant = ch;
        }
    }

    analysis.explicitContinuation =
        parenDepthBefore == 0 && analysis.parenDepthAfter == 0 && lastSignificant == '\\';
    analysis.opensBlock =
        parenDepthBefore == 0 && analysis.parenDepthAfter == 0 && lastSignificant == ':';
    return analysis;
}

std::string FormatPython(std::string_view inputUtf8, int tabWidth) {
    SplitLinesResult split = SplitLines(inputUtf8);
    std::vector<std::string> output;
    output.reserve(split.lines.size());

    int indentLevel = 0;
    int parenDepth = 0;
    bool explicitContinuation = false;

    for (const std::string& rawLine : split.lines) {
        const std::string_view line = rawLine;
        const std::string_view trimmedLine = TrimLeftAsciiWhitespace(line);
        if (trimmedLine.empty()) {
            output.emplace_back();
            continue;
        }

        const bool continuationContext = (parenDepth > 0) || explicitContinuation;
        int leadingColumns = LeadingIndentColumns(line, tabWidth);

        if (!continuationContext) {
            if (IsPythonDedentKeyword(trimmedLine) && indentLevel > 0) {
                --indentLevel;
            }
            leadingColumns = indentLevel * tabWidth;
        }

        std::string normalizedLine(static_cast<size_t>(std::max(0, leadingColumns)), ' ');
        normalizedLine += std::string(trimmedLine);
        output.push_back(std::move(normalizedLine));

        const PythonLineAnalysis analysis = AnalyzePythonLine(trimmedLine, parenDepth);
        parenDepth = analysis.parenDepthAfter;
        explicitContinuation = analysis.explicitContinuation;
        if (!continuationContext && analysis.opensBlock) {
            ++indentLevel;
        }
    }

    return JoinLines(output, split.hasTrailingNewline);
}

std::string NormalizeLeadingTabs(std::string_view inputUtf8, int tabWidth) {
    SplitLinesResult split = SplitLines(inputUtf8);
    std::vector<std::string> output;
    output.reserve(split.lines.size());

    for (const std::string& rawLine : split.lines) {
        const std::string_view line = rawLine;
        const std::string_view trimmedLine = TrimLeftAsciiWhitespace(line);
        const int leadingColumns = LeadingIndentColumns(line, tabWidth);
        std::string normalizedLine(static_cast<size_t>(std::max(0, leadingColumns)), ' ');
        normalizedLine += std::string(trimmedLine);
        output.push_back(std::move(normalizedLine));
    }

    return JoinLines(output, split.hasTrailingNewline);
}

}  // namespace

LanguageAwareFormatResult FormatDocumentLanguageAware(
    std::string_view inputUtf8,
    std::string_view lexerName,
    int tabWidth) {
    const int normalizedTabWidth = std::clamp(tabWidth, 1, 12);
    LanguageAwareFormatResult result;
    result.formattedTextUtf8 = std::string(inputUtf8);

    if (lexerName == "python") {
        result.supported = true;
        result.formatterName = "Built-in Python formatter";
        result.formattedTextUtf8 = FormatPython(inputUtf8, normalizedTabWidth);
    } else if (lexerName == "yaml" || lexerName == "bash") {
        result.supported = true;
        result.formatterName = "Built-in indentation normalizer";
        result.formattedTextUtf8 = NormalizeLeadingTabs(inputUtf8, normalizedTabWidth);
    } else {
        result.supported = false;
        result.formatterName = "No built-in formatter";
        result.formattedTextUtf8 = std::string(inputUtf8);
    }

    result.changed = result.formattedTextUtf8 != std::string(inputUtf8);
    return result;
}

}  // namespace npp::ui

