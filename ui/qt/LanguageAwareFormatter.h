#pragma once

#include <string>
#include <string_view>

namespace npp::ui {

struct LanguageAwareFormatResult {
    bool supported = false;
    bool changed = false;
    std::string formatterName;
    std::string formattedTextUtf8;
};

LanguageAwareFormatResult FormatDocumentLanguageAware(
    std::string_view inputUtf8,
    std::string_view lexerName,
    int tabWidth);

}  // namespace npp::ui

