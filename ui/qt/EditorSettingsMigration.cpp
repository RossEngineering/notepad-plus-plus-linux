#include "EditorSettingsMigration.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <vector>

#include "json.hpp"

namespace npp::ui {

namespace {

using Json = nlohmann::json;

std::string TrimAsciiWhitespace(std::string value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::vector<std::string> ParseLanguageCsvList(const std::string& csv) {
    std::set<std::string> dedupe;
    std::vector<std::string> languages;
    size_t cursor = 0;
    while (cursor <= csv.size()) {
        const size_t commaPos = csv.find(',', cursor);
        const size_t endPos = commaPos == std::string::npos ? csv.size() : commaPos;
        std::string token = AsciiLower(TrimAsciiWhitespace(csv.substr(cursor, endPos - cursor)));
        if (!token.empty() && dedupe.insert(token).second) {
            languages.push_back(token);
        }
        if (commaPos == std::string::npos) {
            break;
        }
        cursor = commaPos + 1;
    }
    return languages;
}

bool IsValidFormatterProfile(const std::string& profile) {
    if (profile == "auto" || profile == "builtin" || profile == "extension") {
        return true;
    }
    static constexpr char kPrefix[] = "extension:";
    if (profile.rfind(kPrefix, 0) == 0 && profile.size() > sizeof(kPrefix) - 1) {
        return true;
    }
    return false;
}

std::string NormalizeFormatterProfile(std::string profile) {
    profile = AsciiLower(TrimAsciiWhitespace(profile));
    if (profile.empty()) {
        return "auto";
    }
    if (IsValidFormatterProfile(profile)) {
        return profile;
    }
    return "auto";
}

void MigrateAliasKey(Json& object, const char* canonicalKey, const std::initializer_list<const char*> aliases) {
    if (!object.is_object()) {
        return;
    }
    if (object.contains(canonicalKey)) {
        return;
    }
    for (const char* alias : aliases) {
        auto aliasIt = object.find(alias);
        if (aliasIt != object.end()) {
            object[canonicalKey] = *aliasIt;
            return;
        }
    }
}

void NormalizeFormatOnSaveLanguages(Json& object) {
    if (!object.is_object()) {
        return;
    }

    auto valueIt = object.find("formatOnSaveLanguages");
    if (valueIt == object.end()) {
        return;
    }

    std::set<std::string> dedupe;
    std::vector<std::string> normalized;
    auto appendLanguage = [&](const std::string& candidate) {
        const std::string language = AsciiLower(TrimAsciiWhitespace(candidate));
        if (!language.empty() && dedupe.insert(language).second) {
            normalized.push_back(language);
        }
    };

    if (valueIt->is_string()) {
        const std::vector<std::string> parsed = ParseLanguageCsvList(valueIt->get<std::string>());
        for (const std::string& language : parsed) {
            appendLanguage(language);
        }
    } else if (valueIt->is_array()) {
        for (const Json& languageValue : *valueIt) {
            if (!languageValue.is_string()) {
                continue;
            }
            const std::vector<std::string> parsed = ParseLanguageCsvList(languageValue.get<std::string>());
            for (const std::string& language : parsed) {
                appendLanguage(language);
            }
        }
    }

    std::sort(normalized.begin(), normalized.end());
    Json canonical = Json::array();
    for (const std::string& language : normalized) {
        canonical.push_back(language);
    }
    object["formatOnSaveLanguages"] = std::move(canonical);
}

void NormalizeFormatterOverrides(Json& object) {
    if (!object.is_object()) {
        return;
    }

    auto valueIt = object.find("formatterProfileOverrides");
    if (valueIt == object.end() || !valueIt->is_object()) {
        return;
    }

    Json canonical = Json::object();
    std::vector<std::string> keys;
    keys.reserve(valueIt->size());
    for (auto it = valueIt->begin(); it != valueIt->end(); ++it) {
        keys.push_back(it.key());
    }
    std::sort(keys.begin(), keys.end());

    for (const std::string& rawKey : keys) {
        const Json& profileValue = (*valueIt)[rawKey];
        if (!profileValue.is_string()) {
            continue;
        }
        const std::string language = AsciiLower(TrimAsciiWhitespace(rawKey));
        if (language.empty()) {
            continue;
        }
        const std::string profile = NormalizeFormatterProfile(profileValue.get<std::string>());
        if (!IsValidFormatterProfile(profile)) {
            continue;
        }
        canonical[language] = profile;
    }

    object["formatterProfileOverrides"] = std::move(canonical);
}

}  // namespace

std::string ApplyEditorSettingsMigrations(const std::string& settingsJsonUtf8) {
    const Json parsed = Json::parse(settingsJsonUtf8, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
        return settingsJsonUtf8;
    }

    Json object = parsed;

    MigrateAliasKey(object, "autoDetectLanguage", {"languageAutoDetect", "autoDetectLang"});
    MigrateAliasKey(object, "formatOnSaveEnabled", {"formatOnSave"});
    MigrateAliasKey(object, "splitViewMode", {"splitMode"});
    MigrateAliasKey(object, "minimapEnabled", {"minimap"});
    MigrateAliasKey(object, "restoreSessionOnStartup", {"restoreSession"});
    MigrateAliasKey(object, "autoSaveIntervalSeconds", {"autoSaveSeconds"});
    MigrateAliasKey(object, "extensionStartupBudgetMs", {"extensionStartupBudget"});
    MigrateAliasKey(object, "extensionPerExtensionBudgetMs", {"extensionPerExtensionBudget"});

    object.erase("languageAutoDetect");
    object.erase("autoDetectLang");
    object.erase("formatOnSave");
    object.erase("splitMode");
    object.erase("minimap");
    object.erase("restoreSession");
    object.erase("autoSaveSeconds");
    object.erase("extensionStartupBudget");
    object.erase("extensionPerExtensionBudget");

    NormalizeFormatOnSaveLanguages(object);
    NormalizeFormatterOverrides(object);
    if (object.contains("formatterDefaultProfile") && object["formatterDefaultProfile"].is_string()) {
        object["formatterDefaultProfile"] = NormalizeFormatterProfile(object["formatterDefaultProfile"].get<std::string>());
    }

    return object.dump(2) + "\n";
}

}  // namespace npp::ui
