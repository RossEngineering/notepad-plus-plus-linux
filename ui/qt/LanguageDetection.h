#pragma once

#include <string>
#include <string_view>

namespace npp::ui {

struct LanguageDetectionResult {
    std::string lexerName;
    double confidence = 0.0;
    std::string reason;
};

LanguageDetectionResult DetectLanguage(std::string_view pathUtf8, std::string_view contentUtf8);

}  // namespace npp::ui
