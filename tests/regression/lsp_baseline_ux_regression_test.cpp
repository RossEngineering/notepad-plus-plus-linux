#include <iostream>
#include <string>

#include "LspBaselineFeatures.h"

int main() {
    using namespace npp::ui;

    if (MapLexerToLspLanguageId("cpp") != "cpp" ||
        MapLexerToLspLanguageId("python") != "python" ||
        MapLexerToLspLanguageId("xml") != "html" ||
        !MapLexerToLspLanguageId("null").empty()) {
        std::cerr << "lexer-to-language-id mapping regression\n";
        return 1;
    }

    const std::string source =
        "int helper(int value) {\n"
        "    return value + 1;\n"
        "}\n"
        "\n"
        "int main() {\n"
        "    return helper(41);\n"
        "}\n";

    const std::size_t helperCall = source.find("helper(41)");
    const std::string extracted = ExtractWordAt(source, helperCall);
    if (extracted != "helper") {
        std::cerr << "failed to extract symbol under caret\n";
        return 2;
    }

    const int definitionLine = FindBaselineDefinitionLine(source, "helper");
    if (definitionLine != 0) {
        std::cerr << "failed to find expected baseline definition line\n";
        return 3;
    }

    const std::string diagnosticSample =
        "line with trailing whitespace  \n"
        "\tline with tab\n"
        + std::string(145, 'x') + "\n";
    const auto diagnostics = CollectBaselineDiagnostics(diagnosticSample);
    if (diagnostics.size() < 3) {
        std::cerr << "expected baseline diagnostics were not produced\n";
        return 4;
    }

    std::cout << "LSP baseline UX regression checks passed\n";
    return 0;
}
