#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "LanguageDetection.h"

namespace {

struct Case {
    std::string fixturePath;
    std::string virtualPath;
    std::string expectedLexer;
    double minConfidence;
};

std::string ReadTextFile(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool ApproxLess(double left, double right) {
    return left + 1e-9 < right;
}

}  // namespace

int main() {
    const std::filesystem::path sourceRoot = std::filesystem::path(NPP_SOURCE_DIR);
    const std::filesystem::path fixtureRoot =
        sourceRoot / "tests" / "fixtures" / "language-detection";

    const std::vector<Case> cases = {
        {"sample_markdown.md", "notes/sample_markdown.md", "markdown", 0.95},
        {"sample_markdown_no_ext.txt", "notes/release_notes", "markdown", 0.70},
        {"sample_html.html", "web/sample_html.html", "xml", 0.95},
        {"sample_html_no_ext.txt", "web/index_template", "xml", 0.80},
        {"sample_python.py", "scripts/sample_python.py", "python", 0.95},
        {"sample_python_shebang", "scripts/run", "python", 0.90},
        {"sample_shell_shebang", "scripts/deploy", "bash", 0.90},
        {"sample_cpp.cpp", "src/sample_cpp.cpp", "cpp", 0.95},
        {"sample_json_no_ext.txt", "data/config", "json", 0.80},
    };

    size_t total = 0;
    size_t correct = 0;
    size_t falsePositive = 0;
    size_t lowConfidenceFailures = 0;

    for (const Case &entry : cases) {
        const std::filesystem::path fixturePath = fixtureRoot / entry.fixturePath;
        const std::string text = ReadTextFile(fixturePath);
        if (text.empty()) {
            std::cerr << "failed reading fixture: " << fixturePath.string() << "\n";
            return 2;
        }

        const npp::ui::LanguageDetectionResult result = npp::ui::DetectLanguage(entry.virtualPath, text);
        ++total;
        if (result.lexerName == entry.expectedLexer) {
            ++correct;
        } else if (result.lexerName != "null") {
            ++falsePositive;
            std::cerr << "wrong lexer for " << entry.fixturePath << ": expected " << entry.expectedLexer
                      << " got " << result.lexerName << " via " << result.reason << "\n";
        }
        if (ApproxLess(result.confidence, entry.minConfidence)) {
            ++lowConfidenceFailures;
            std::cerr << "confidence too low for " << entry.fixturePath
                      << ": got " << result.confidence << " expected >= " << entry.minConfidence << "\n";
        }
    }

    const double accuracy = total > 0 ? static_cast<double>(correct) / static_cast<double>(total) : 0.0;
    const double falsePositiveRate =
        total > 0 ? static_cast<double>(falsePositive) / static_cast<double>(total) : 0.0;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "language_detection_total=" << total << "\n";
    std::cout << "language_detection_correct=" << correct << "\n";
    std::cout << "language_detection_accuracy=" << accuracy << "\n";
    std::cout << "language_detection_false_positive_rate=" << falsePositiveRate << "\n";
    std::cout << "language_detection_low_confidence_failures=" << lowConfidenceFailures << "\n";

    constexpr double kRequiredAccuracy = 0.95;
    constexpr double kMaxFalsePositiveRate = 0.05;
    if (accuracy < kRequiredAccuracy || falsePositiveRate > kMaxFalsePositiveRate || lowConfidenceFailures > 0) {
        std::cerr << "language detection metrics threshold failed\n";
        return 1;
    }
    return 0;
}
