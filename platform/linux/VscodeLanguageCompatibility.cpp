#include "VscodeLanguageCompatibility.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <utility>

#include "json/json.hpp"

namespace npp::platform {

namespace {

using Json = nlohmann::json;

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

Status MakeInvalidStatus(const std::string& message) {
    return Status{StatusCode::kInvalidArgument, message};
}

StatusOr<std::string> ReadTextFile(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kNotFound, "Unable to open file: " + path.string()});
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return StatusOr<std::string>{Status::Ok(), buffer.str()};
}

StatusOr<Json> ParseJson(const std::string& content, const std::string& sourceLabel) {
    try {
        const Json parsed = Json::parse(content);
        return StatusOr<Json>{Status::Ok(), parsed};
    } catch (const std::exception& ex) {
        return StatusOr<Json>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Failed parsing JSON for " + sourceLabel + ": " + ex.what()});
    }
}

std::string SuggestedLexerForLanguageId(const std::string& languageId) {
    static const std::map<std::string, std::string> kLexerMap = {
        {"bash", "bash"},
        {"c", "cpp"},
        {"cpp", "cpp"},
        {"csharp", "cpp"},
        {"go", "cpp"},
        {"html", "xml"},
        {"java", "cpp"},
        {"javascript", "cpp"},
        {"json", "json"},
        {"markdown", "markdown"},
        {"python", "python"},
        {"rust", "cpp"},
        {"shellscript", "bash"},
        {"sql", "sql"},
        {"typescript", "cpp"},
        {"xml", "xml"},
        {"yaml", "yaml"},
    };
    const auto it = kLexerMap.find(AsciiLower(languageId));
    if (it == kLexerMap.end()) {
        return "null";
    }
    return it->second;
}

bool IsRelativePath(const std::string& pathUtf8) {
    if (pathUtf8.empty()) {
        return false;
    }
    return std::filesystem::path(pathUtf8).is_relative();
}

std::vector<std::string> ReadStringArray(const Json& object, const char* key) {
    std::vector<std::string> out;
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) {
        return out;
    }
    for (const auto& value : *it) {
        if (!value.is_string()) {
            continue;
        }
        out.push_back(value.get<std::string>());
    }
    return out;
}

Status EnsureReferencedPathExists(
    const std::filesystem::path& extensionRoot,
    const std::string& relativePath,
    const std::string& label) {
    if (relativePath.empty()) {
        return MakeInvalidStatus("Missing required relative path for " + label);
    }
    if (!IsRelativePath(relativePath)) {
        return MakeInvalidStatus("Path for " + label + " must be relative: " + relativePath);
    }
    const std::filesystem::path resolved = extensionRoot / relativePath;
    std::error_code ec;
    if (!std::filesystem::exists(resolved, ec) || ec) {
        return Status{StatusCode::kNotFound, "Referenced file not found for " + label + ": " + resolved.string()};
    }
    return Status::Ok();
}

}  // namespace

StatusOr<VscodeLanguagePackSnapshot> LoadVscodeLanguagePackFromDirectory(
    const std::string& extensionDirectoryUtf8) {
    if (extensionDirectoryUtf8.empty()) {
        return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
            MakeInvalidStatus("Extension directory path must not be empty"));
    }

    const std::filesystem::path extensionRoot = std::filesystem::path(extensionDirectoryUtf8);
    const std::filesystem::path packagePath = extensionRoot / "package.json";

    const auto packageText = ReadTextFile(packagePath);
    if (!packageText.ok()) {
        return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(packageText.status);
    }

    const auto packageJson = ParseJson(*packageText.value, packagePath.string());
    if (!packageJson.ok()) {
        return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(packageJson.status);
    }
    if (!packageJson.value->is_object()) {
        return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
            MakeInvalidStatus("package.json root must be an object"));
    }

    const auto contributionsIt = packageJson.value->find("contributes");
    if (contributionsIt == packageJson.value->end() || !contributionsIt->is_object()) {
        return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
            MakeInvalidStatus("package.json missing object key: contributes"));
    }

    const Json& contributions = *contributionsIt;
    VscodeLanguagePackSnapshot snapshot;

    const auto languagesIt = contributions.find("languages");
    if (languagesIt != contributions.end()) {
        if (!languagesIt->is_array()) {
            return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                MakeInvalidStatus("contributes.languages must be an array"));
        }
        for (const auto& language : *languagesIt) {
            if (!language.is_object()) {
                return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                    MakeInvalidStatus("language contribution entries must be objects"));
            }
            const auto idIt = language.find("id");
            if (idIt == language.end() || !idIt->is_string()) {
                return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                    MakeInvalidStatus("language contribution missing string id"));
            }

            VscodeLanguageContribution contribution;
            contribution.id = idIt->get<std::string>();
            contribution.aliases = ReadStringArray(language, "aliases");
            contribution.extensions = ReadStringArray(language, "extensions");
            contribution.suggestedLexer = SuggestedLexerForLanguageId(contribution.id);

            const auto configIt = language.find("configuration");
            if (configIt != language.end() && configIt->is_string()) {
                contribution.configurationPath = configIt->get<std::string>();
                const Status configStatus = EnsureReferencedPathExists(
                    extensionRoot,
                    contribution.configurationPath,
                    "contributes.languages.configuration");
                if (!configStatus.ok()) {
                    return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(configStatus);
                }
            }

            snapshot.languages.push_back(std::move(contribution));
        }
    }

    const auto grammarsIt = contributions.find("grammars");
    if (grammarsIt != contributions.end()) {
        if (!grammarsIt->is_array()) {
            return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                MakeInvalidStatus("contributes.grammars must be an array"));
        }
        for (const auto& grammar : *grammarsIt) {
            if (!grammar.is_object()) {
                return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                    MakeInvalidStatus("grammar contribution entries must be objects"));
            }
            const auto languageIt = grammar.find("language");
            const auto scopeNameIt = grammar.find("scopeName");
            const auto pathIt = grammar.find("path");
            if (languageIt == grammar.end() || !languageIt->is_string() ||
                scopeNameIt == grammar.end() || !scopeNameIt->is_string() ||
                pathIt == grammar.end() || !pathIt->is_string()) {
                return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(
                    MakeInvalidStatus("grammar contribution requires string fields: language, scopeName, path"));
            }

            VscodeGrammarContribution contribution;
            contribution.languageId = languageIt->get<std::string>();
            contribution.scopeName = scopeNameIt->get<std::string>();
            contribution.path = pathIt->get<std::string>();

            const Status grammarStatus = EnsureReferencedPathExists(
                extensionRoot,
                contribution.path,
                "contributes.grammars.path");
            if (!grammarStatus.ok()) {
                return StatusOr<VscodeLanguagePackSnapshot>::FromStatus(grammarStatus);
            }

            snapshot.grammars.push_back(std::move(contribution));
        }
    }

    return StatusOr<VscodeLanguagePackSnapshot>{Status::Ok(), snapshot};
}

Status WriteVscodeLanguagePackSnapshot(
    const std::string& outputPathUtf8,
    const VscodeLanguagePackSnapshot& snapshot) {
    if (outputPathUtf8.empty()) {
        return MakeInvalidStatus("Output path must not be empty");
    }

    Json root;
    root["schemaVersion"] = 1;
    root["sourceFormat"] = "vscode-language-assets";
    root["languages"] = Json::array();
    root["grammars"] = Json::array();

    for (const auto& language : snapshot.languages) {
        Json item;
        item["id"] = language.id;
        item["aliases"] = language.aliases;
        item["extensions"] = language.extensions;
        item["configurationPath"] = language.configurationPath;
        item["suggestedLexer"] = language.suggestedLexer;
        root["languages"].push_back(std::move(item));
    }

    for (const auto& grammar : snapshot.grammars) {
        Json item;
        item["languageId"] = grammar.languageId;
        item["scopeName"] = grammar.scopeName;
        item["path"] = grammar.path;
        root["grammars"].push_back(std::move(item));
    }

    const std::filesystem::path outputPath = std::filesystem::path(outputPathUtf8);
    std::error_code ec;
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path(), ec);
        if (ec) {
            return Status{StatusCode::kIoError, "Failed creating output parent directory: " + ec.message()};
        }
    }

    std::ofstream stream(outputPath, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        return Status{StatusCode::kIoError, "Failed opening output path: " + outputPath.string()};
    }
    stream << root.dump(2) << '\n';
    if (!stream.good()) {
        return Status{StatusCode::kIoError, "Failed writing output path: " + outputPath.string()};
    }

    return Status::Ok();
}

}  // namespace npp::platform
