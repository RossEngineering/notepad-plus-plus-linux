#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "ScintillaTypes.h"
#include "ILoader.h"
#include "ILexer.h"

#include "Debugging.h"

#include "CharacterCategoryMap.h"
#include "Position.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"

#include "Lexilla.h"

namespace {

struct LexerReleaser {
    void operator()(Scintilla::ILexer5 *lexer) const noexcept {
        if (lexer != nullptr) {
            lexer->Release();
        }
    }
};

double Mean(const std::vector<double> &values) {
    if (values.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (const double value : values) {
        sum += value;
    }
    return sum / static_cast<double>(values.size());
}

double Percentile95(std::vector<double> values) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const size_t index = static_cast<size_t>(0.95 * static_cast<double>(values.size() - 1));
    return values[index];
}

std::string SampleDocument() {
    return
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "int main() {\n"
        "    for (int i = 0; i < 1024; ++i) {\n"
        "        add(i, i * 2);\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
}

std::vector<double> RunStartupBenchmark(int iterations) {
    using Clock = std::chrono::steady_clock;
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;

    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iterations));

    const std::string text = SampleDocument();
    for (int i = 0; i < iterations; ++i) {
        const auto start = Clock::now();

        Document doc(DocumentOption::Default);
        doc.SetDBCSCodePage(CpUtf8);
        doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());
        doc.InsertString(0, text);

        std::unique_ptr<Scintilla::ILexer5, LexerReleaser> lexer(CreateLexer("cpp"));
        if (lexer != nullptr) {
            const Sci::Position length = doc.LengthNoExcept();
            lexer->Lex(0, length, 0, &doc);
        }

        const auto stop = Clock::now();
        const std::chrono::duration<double, std::micro> elapsed = stop - start;
        samples.push_back(elapsed.count());
    }
    return samples;
}

std::vector<double> RunTypingBenchmark(int iterations) {
    using Clock = std::chrono::steady_clock;
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());
    const std::string seedText = SampleDocument();
    doc.InsertString(0, seedText);

    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iterations));

    for (int i = 0; i < iterations; ++i) {
        const auto start = Clock::now();
        doc.InsertString(doc.LengthNoExcept(), "x", 1);
        const auto stop = Clock::now();
        const std::chrono::duration<double, std::micro> elapsed = stop - start;
        samples.push_back(elapsed.count());
    }
    return samples;
}

}  // namespace

int main(int argc, char **argv) {
    int startupIterations = 200;
    int typingIterations = 5000;

    if (argc > 1 && std::string_view(argv[1]) == "--quick") {
        startupIterations = 50;
        typingIterations = 1000;
    }

    const std::vector<double> startupSamples = RunStartupBenchmark(startupIterations);
    const std::vector<double> typingSamples = RunTypingBenchmark(typingIterations);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "startup_mean_us=" << Mean(startupSamples) << "\n";
    std::cout << "startup_p95_us=" << Percentile95(startupSamples) << "\n";
    std::cout << "typing_mean_us=" << Mean(typingSamples) << "\n";
    std::cout << "typing_p95_us=" << Percentile95(typingSamples) << "\n";
    std::cout << "startup_iterations=" << startupIterations << "\n";
    std::cout << "typing_iterations=" << typingIterations << "\n";

    return 0;
}
