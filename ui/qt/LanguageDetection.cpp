#include "LanguageDetection.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string TrimLeft(std::string_view text) {
    size_t index = 0;
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index])) != 0) {
        ++index;
    }
    return std::string(text.substr(index));
}

std::string FirstLine(std::string_view text) {
    const size_t newlinePos = text.find('\n');
    if (newlinePos == std::string_view::npos) {
        return std::string(text);
    }
    return std::string(text.substr(0, newlinePos));
}

std::string ExtensionLexer(std::string_view pathUtf8) {
    if (pathUtf8.empty()) {
        return {};
    }
    const std::string extension = AsciiLower(std::filesystem::path(std::string(pathUtf8)).extension().string());
    if (extension == ".c" || extension == ".cc" || extension == ".cpp" ||
        extension == ".cxx" || extension == ".h" || extension == ".hpp" ||
        extension == ".hh" || extension == ".hxx" || extension == ".java") {
        return "cpp";
    }
    if (extension == ".py" || extension == ".pyw") {
        return "python";
    }
    if (extension == ".js" || extension == ".jsx" || extension == ".ts" || extension == ".tsx") {
        return "cpp";
    }
    if (extension == ".json") {
        return "json";
    }
    if (extension == ".xml" || extension == ".xsd" || extension == ".xsl" ||
        extension == ".html" || extension == ".htm") {
        return "xml";
    }
    if (extension == ".md" || extension == ".markdown") {
        return "markdown";
    }
    if (extension == ".sh" || extension == ".bash" || extension == ".zsh") {
        return "bash";
    }
    if (extension == ".yaml" || extension == ".yml") {
        return "yaml";
    }
    if (extension == ".sql") {
        return "sql";
    }
    if (extension == ".toml" || extension == ".ini" || extension == ".cfg" || extension == ".conf") {
        return "props";
    }
    if (extension == ".make" || extension == ".mk") {
        return "makefile";
    }
    return {};
}

npp::ui::LanguageDetectionResult DetectShebang(std::string_view contentUtf8) {
    const std::string first = AsciiLower(FirstLine(contentUtf8));
    if (first.rfind("#!", 0) != 0) {
        return {};
    }
    if (first.find("python") != std::string::npos) {
        return {"python", 0.92, "shebang"};
    }
    if (first.find("bash") != std::string::npos ||
        first.find("/sh") != std::string::npos ||
        first.find("zsh") != std::string::npos) {
        return {"bash", 0.92, "shebang"};
    }
    return {"cpp", 0.70, "shebang-generic-script"};
}

npp::ui::LanguageDetectionResult DetectModeline(std::string_view contentUtf8) {
    const std::string sample = AsciiLower(std::string(contentUtf8.substr(0, std::min<size_t>(contentUtf8.size(), 4096))));
    const std::vector<std::pair<std::string, std::string>> modelines = {
        {"ft=python", "python"},
        {"filetype=python", "python"},
        {"mode: python", "python"},
        {"ft=sh", "bash"},
        {"filetype=sh", "bash"},
        {"mode: shell", "bash"},
        {"ft=markdown", "markdown"},
        {"filetype=markdown", "markdown"},
        {"ft=html", "xml"},
        {"filetype=html", "xml"},
        {"ft=cpp", "cpp"},
        {"filetype=cpp", "cpp"},
    };
    for (const auto &[needle, lexer] : modelines) {
        if (sample.find(needle) != std::string::npos) {
            return {lexer, 0.88, "modeline"};
        }
    }
    return {};
}

size_t CountNeedle(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) {
        return 0;
    }
    size_t count = 0;
    size_t pos = haystack.find(needle);
    while (pos != std::string_view::npos) {
        ++count;
        pos = haystack.find(needle, pos + needle.size());
    }
    return count;
}

npp::ui::LanguageDetectionResult DetectFromContent(std::string_view contentUtf8) {
    if (contentUtf8.empty()) {
        return {};
    }

    const std::string lower = AsciiLower(std::string(contentUtf8.substr(0, std::min<size_t>(contentUtf8.size(), 8192))));
    const std::string trimmed = AsciiLower(TrimLeft(lower));

    if (trimmed.rfind("<!doctype html", 0) == 0 ||
        CountNeedle(lower, "<html") > 0 ||
        (CountNeedle(lower, "<head") > 0 && CountNeedle(lower, "<body") > 0)) {
        return {"xml", 0.86, "html-content"};
    }

    if ((trimmed.rfind("{", 0) == 0 || trimmed.rfind("[", 0) == 0) &&
        CountNeedle(lower, "\":") > 0) {
        return {"json", 0.83, "json-shape"};
    }

    size_t markdownSignals = 0;
    markdownSignals += CountNeedle(lower, "\n# ");
    markdownSignals += CountNeedle(lower, "\n## ");
    markdownSignals += CountNeedle(lower, "\n- ");
    markdownSignals += CountNeedle(lower, "\n* ");
    markdownSignals += CountNeedle(lower, "\n```");
    markdownSignals += CountNeedle(lower, "](");
    if (markdownSignals >= 3) {
        return {"markdown", 0.78, "markdown-signals"};
    }

    if (CountNeedle(lower, "def ") > 0 &&
        (CountNeedle(lower, "import ") > 0 || CountNeedle(lower, "__name__") > 0)) {
        return {"python", 0.74, "python-signals"};
    }

    if (CountNeedle(lower, "#include") > 0 ||
        (CountNeedle(lower, "int main(") > 0 && CountNeedle(lower, "{") > 0)) {
        return {"cpp", 0.72, "cpp-signals"};
    }

    if (CountNeedle(lower, "select ") > 0 && CountNeedle(lower, " from ") > 0) {
        return {"sql", 0.70, "sql-signals"};
    }

    return {};
}

}  // namespace

namespace npp::ui {

LanguageDetectionResult DetectLanguage(std::string_view pathUtf8, std::string_view contentUtf8) {
    const std::string fromExtension = ExtensionLexer(pathUtf8);
    if (!fromExtension.empty()) {
        return {fromExtension, 0.98, "extension"};
    }

    const LanguageDetectionResult fromShebang = DetectShebang(contentUtf8);
    if (fromShebang.confidence > 0.0) {
        return fromShebang;
    }

    const LanguageDetectionResult fromModeline = DetectModeline(contentUtf8);
    if (fromModeline.confidence > 0.0) {
        return fromModeline;
    }

    const LanguageDetectionResult fromContent = DetectFromContent(contentUtf8);
    if (fromContent.confidence > 0.0) {
        return fromContent;
    }

    return {"null", 0.0, "fallback"};
}

}  // namespace npp::ui
