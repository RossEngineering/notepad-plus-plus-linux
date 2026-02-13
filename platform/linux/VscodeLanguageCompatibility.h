#pragma once

#include <string>
#include <vector>

#include "Types.h"

namespace npp::platform {

struct VscodeLanguageContribution {
    std::string id;
    std::vector<std::string> aliases;
    std::vector<std::string> extensions;
    std::string configurationPath;
    std::string suggestedLexer;
};

struct VscodeGrammarContribution {
    std::string languageId;
    std::string scopeName;
    std::string path;
};

struct VscodeLanguagePackSnapshot {
    std::vector<VscodeLanguageContribution> languages;
    std::vector<VscodeGrammarContribution> grammars;
};

StatusOr<VscodeLanguagePackSnapshot> LoadVscodeLanguagePackFromDirectory(
    const std::string& extensionDirectoryUtf8);

Status WriteVscodeLanguagePackSnapshot(
    const std::string& outputPathUtf8,
    const VscodeLanguagePackSnapshot& snapshot);

}  // namespace npp::platform
