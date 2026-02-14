#include <iostream>
#include <string>

#include "LanguageAwareFormatter.h"

int main() {
    using npp::ui::FormatDocumentLanguageAware;

    {
        const std::string input =
            "def outer():\n"
            "print('a')\n"
            "if True:\n"
            "print('b')\n"
            "else:\n"
            "print('c')\n";
        const std::string expected =
            "def outer():\n"
            "    print('a')\n"
            "    if True:\n"
            "        print('b')\n"
            "    else:\n"
            "        print('c')\n";

        const auto result = FormatDocumentLanguageAware(input, "python", 4);
        if (!result.supported) {
            std::cerr << "python formatter should be supported\n";
            return 1;
        }
        if (!result.changed) {
            std::cerr << "python formatter should report changes\n";
            return 2;
        }
        if (result.formattedTextUtf8 != expected) {
            std::cerr << "python formatter output mismatch\n";
            return 3;
        }
    }

    {
        const std::string input =
            "\troot:\n"
            "\t\tchild: true\n";
        const std::string expected =
            "    root:\n"
            "        child: true\n";

        const auto result = FormatDocumentLanguageAware(input, "yaml", 4);
        if (!result.supported || !result.changed) {
            std::cerr << "yaml normalizer should be supported and change tab-indented input\n";
            return 4;
        }
        if (result.formattedTextUtf8 != expected) {
            std::cerr << "yaml normalizer output mismatch\n";
            return 5;
        }
    }

    {
        const std::string input = "no-op text\n";
        const auto result = FormatDocumentLanguageAware(input, "sql", 4);
        if (result.supported) {
            std::cerr << "sql should not be supported by built-in formatter\n";
            return 6;
        }
        if (result.changed) {
            std::cerr << "unsupported formatter should not report changes\n";
            return 7;
        }
        if (result.formattedTextUtf8 != input) {
            std::cerr << "unsupported formatter must preserve text\n";
            return 8;
        }
    }

    return 0;
}
