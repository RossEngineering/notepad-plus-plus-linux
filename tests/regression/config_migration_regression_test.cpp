#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "EditorSettingsMigration.h"
#include "json.hpp"

namespace {

using Json = nlohmann::json;

bool Expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "config migration regression failed: " << message << "\n";
        return false;
    }
    return true;
}

std::string ReadFixtureFile(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

}  // namespace

int main() {
    const std::filesystem::path sourceRoot = std::filesystem::path(NPP_SOURCE_DIR);
    const std::vector<std::string> fixtureNames = {
        "v0.9.9-rc3.json",
        "v0.10.0-beta.2.json",
    };

    for (const std::string& fixtureName : fixtureNames) {
        const std::filesystem::path fixturePath = sourceRoot / "tests/fixtures/config-migration" / fixtureName;
        const std::string input = ReadFixtureFile(fixturePath);
        if (!Expect(!input.empty(), "read fixture " + fixtureName)) {
            return 1;
        }

        const std::string migrated = npp::ui::ApplyEditorSettingsMigrations(input);
        if (!Expect(!migrated.empty(), "migrate fixture " + fixtureName)) {
            return 2;
        }

        const Json migratedJson = Json::parse(migrated, nullptr, false);
        if (!Expect(!migratedJson.is_discarded() && migratedJson.is_object(), "parse migrated json " + fixtureName)) {
            return 3;
        }

        const std::string migratedAgain = npp::ui::ApplyEditorSettingsMigrations(migrated);
        if (!Expect(migratedAgain == migrated, "deterministic migration output " + fixtureName)) {
            return 4;
        }

        if (!Expect(migratedJson.contains("autoDetectLanguage"), "canonical autoDetectLanguage key " + fixtureName)) {
            return 5;
        }
        if (!Expect(!migratedJson.contains("languageAutoDetect"), "legacy languageAutoDetect removed " + fixtureName)) {
            return 6;
        }
        if (!Expect(!migratedJson.contains("autoDetectLang"), "legacy autoDetectLang removed " + fixtureName)) {
            return 7;
        }

        if (migratedJson.contains("formatOnSaveLanguages")) {
            if (!Expect(migratedJson["formatOnSaveLanguages"].is_array(), "formatOnSaveLanguages normalized array " + fixtureName)) {
                return 8;
            }
        }

        if (fixtureName == "v0.9.9-rc3.json") {
            if (!Expect(migratedJson.value("autoDetectLanguage", true) == false, "legacy auto detect migrated false")) {
                return 9;
            }
            if (!Expect(migratedJson.value("formatOnSaveEnabled", false) == true, "legacy formatOnSave migrated")) {
                return 10;
            }
            if (!Expect(migratedJson.value("splitViewMode", 0) == 1, "splitMode alias migrated")) {
                return 11;
            }
            if (!Expect(migratedJson.value("minimapEnabled", false) == true, "minimap alias migrated")) {
                return 12;
            }
            if (!Expect(migratedJson.value("autoSaveIntervalSeconds", 0) == 45, "autoSaveSeconds alias migrated")) {
                return 13;
            }
            if (!Expect(
                    migratedJson.value("extensionStartupBudgetMs", 0) == 1800 &&
                        migratedJson.value("extensionPerExtensionBudgetMs", 0) == 350,
                    "extension budget aliases migrated")) {
                return 14;
            }
        }
    }

    return 0;
}
