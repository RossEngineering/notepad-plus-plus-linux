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

std::string LargeSampleDocument() {
    std::string text;
    text.reserve(4U * 1024U * 1024U);
    for (int i = 0; i < 18000; ++i) {
        text += "line ";
        text += std::to_string(i);
        text += ": alpha beta gamma delta ";
        text += (i % 17 == 0) ? "needle_hot" : "filler";
        text += " token_";
        text += std::to_string(i % 100);
        text += " lorem ipsum dolor sit amet\n";
    }
    return text;
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

std::vector<double> RunLargeFileOpenBenchmark(int iterations) {
    using Clock = std::chrono::steady_clock;
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;

    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iterations));

    const std::string text = LargeSampleDocument();
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

std::vector<double> RunSearchBenchmark(int iterations) {
    using Clock = std::chrono::steady_clock;
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::FindOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());
    const std::string largeText = LargeSampleDocument();
    doc.InsertString(0, largeText);

    const std::array<std::string, 4> queries = {
        "needle_hot",
        "token_42",
        "alpha beta",
        "line 1729"
    };

    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(iterations));
    for (int i = 0; i < iterations; ++i) {
        const std::string &query = queries[static_cast<size_t>(i) % queries.size()];
        Sci::Position foundLength = static_cast<Sci::Position>(query.size());
        const auto start = Clock::now();
        const Sci::Position position = doc.FindText(
            0,
            doc.LengthNoExcept(),
            query.c_str(),
            FindOption::None,
            &foundLength);
        const auto stop = Clock::now();
        if (position < 0 || foundLength <= 0) {
            samples.push_back(1e9);
            continue;
        }
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
    int largeFileOpenIterations = 60;
    int searchIterations = 1500;
    int typingIterations = 5000;

    if (argc > 1 && std::string_view(argv[1]) == "--quick") {
        startupIterations = 50;
        largeFileOpenIterations = 15;
        searchIterations = 400;
        typingIterations = 1000;
    }

    const std::vector<double> startupSamples = RunStartupBenchmark(startupIterations);
    const std::vector<double> largeFileOpenSamples = RunLargeFileOpenBenchmark(largeFileOpenIterations);
    const std::vector<double> searchSamples = RunSearchBenchmark(searchIterations);
    const std::vector<double> typingSamples = RunTypingBenchmark(typingIterations);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "startup_mean_us=" << Mean(startupSamples) << "\n";
    std::cout << "startup_p95_us=" << Percentile95(startupSamples) << "\n";
    std::cout << "large_file_open_mean_us=" << Mean(largeFileOpenSamples) << "\n";
    std::cout << "large_file_open_p95_us=" << Percentile95(largeFileOpenSamples) << "\n";
    std::cout << "search_mean_us=" << Mean(searchSamples) << "\n";
    std::cout << "search_p95_us=" << Percentile95(searchSamples) << "\n";
    std::cout << "typing_mean_us=" << Mean(typingSamples) << "\n";
    std::cout << "typing_p95_us=" << Percentile95(typingSamples) << "\n";
    std::cout << "startup_iterations=" << startupIterations << "\n";
    std::cout << "large_file_open_iterations=" << largeFileOpenIterations << "\n";
    std::cout << "search_iterations=" << searchIterations << "\n";
    std::cout << "typing_iterations=" << typingIterations << "\n";

    return 0;
}
