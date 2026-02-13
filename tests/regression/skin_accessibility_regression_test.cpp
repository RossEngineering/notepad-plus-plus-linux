#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct RgbColor {
    int red = 0;
    int green = 0;
    int blue = 0;
};

struct SkinPalette {
    RgbColor windowBackground;
    RgbColor windowForeground;
    RgbColor menuBackground;
    RgbColor menuForeground;
    RgbColor statusBackground;
    RgbColor statusForeground;
    RgbColor accent;

    RgbColor editorBackground;
    RgbColor editorForeground;
    RgbColor lineNumberBackground;
    RgbColor lineNumberForeground;
    RgbColor selectionBackground;
    RgbColor selectionForeground;

    RgbColor dialogBackground;
    RgbColor dialogForeground;
    RgbColor dialogButtonBackground;
    RgbColor dialogButtonForeground;
};

bool ParseHexColor(std::string_view value, RgbColor *outColor) {
    if (!outColor || value.size() != 7 || value[0] != '#') {
        return false;
    }

    const auto parsePair = [](char first, char second) -> int {
        auto parseHexDigit = [](char digit) -> int {
            if (digit >= '0' && digit <= '9') {
                return digit - '0';
            }
            if (digit >= 'a' && digit <= 'f') {
                return 10 + (digit - 'a');
            }
            if (digit >= 'A' && digit <= 'F') {
                return 10 + (digit - 'A');
            }
            return -1;
        };

        const int high = parseHexDigit(first);
        const int low = parseHexDigit(second);
        if (high < 0 || low < 0) {
            return -1;
        }
        return high * 16 + low;
    };

    const int red = parsePair(value[1], value[2]);
    const int green = parsePair(value[3], value[4]);
    const int blue = parsePair(value[5], value[6]);
    if (red < 0 || green < 0 || blue < 0) {
        return false;
    }

    outColor->red = red;
    outColor->green = green;
    outColor->blue = blue;
    return true;
}

std::optional<std::string> ReadTextFile(const std::filesystem::path &path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

size_t FindMatchingBrace(const std::string &text, size_t openBraceIndex) {
    int depth = 0;
    bool inString = false;
    bool escape = false;
    for (size_t i = openBraceIndex; i < text.size(); ++i) {
        const char ch = text[i];
        if (escape) {
            escape = false;
            continue;
        }
        if (ch == '\\' && inString) {
            escape = true;
            continue;
        }
        if (ch == '"') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }
        if (ch == '{') {
            ++depth;
            continue;
        }
        if (ch == '}') {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

std::optional<std::string> ExtractJsonObject(std::string_view json, std::string_view key) {
    const std::string keyToken = "\"" + std::string(key) + "\"";
    const size_t keyPos = json.find(keyToken);
    if (keyPos == std::string::npos) {
        return std::nullopt;
    }

    const size_t objectStart = json.find('{', keyPos);
    if (objectStart == std::string::npos) {
        return std::nullopt;
    }
    const size_t objectEnd = FindMatchingBrace(std::string(json), objectStart);
    if (objectEnd == std::string::npos) {
        return std::nullopt;
    }
    return std::string(json.substr(objectStart, objectEnd - objectStart + 1));
}

std::optional<std::string> ExtractJsonStringValue(std::string_view jsonObject, std::string_view key) {
    const std::string keyToken = "\"" + std::string(key) + "\"";
    const size_t keyPos = jsonObject.find(keyToken);
    if (keyPos == std::string::npos) {
        return std::nullopt;
    }
    const size_t colonPos = jsonObject.find(':', keyPos + keyToken.size());
    if (colonPos == std::string::npos) {
        return std::nullopt;
    }
    const size_t quoteOpen = jsonObject.find('"', colonPos + 1);
    if (quoteOpen == std::string::npos) {
        return std::nullopt;
    }
    const size_t quoteClose = jsonObject.find('"', quoteOpen + 1);
    if (quoteClose == std::string::npos) {
        return std::nullopt;
    }
    return std::string(jsonObject.substr(quoteOpen + 1, quoteClose - quoteOpen - 1));
}

std::optional<RgbColor> ExtractColor(std::string_view jsonObject, std::string_view key) {
    const std::optional<std::string> value = ExtractJsonStringValue(jsonObject, key);
    if (!value.has_value()) {
        return std::nullopt;
    }
    RgbColor color;
    if (!ParseHexColor(*value, &color)) {
        return std::nullopt;
    }
    return color;
}

bool ParseSkinPalette(const std::string &json, SkinPalette *paletteOut, std::vector<std::string> *errors) {
    if (!paletteOut || !errors) {
        return false;
    }

    const std::optional<std::string> appChrome = ExtractJsonObject(json, "appChrome");
    const std::optional<std::string> editor = ExtractJsonObject(json, "editor");
    const std::optional<std::string> dialogs = ExtractJsonObject(json, "dialogs");
    if (!appChrome || !editor || !dialogs) {
        errors->push_back("Missing one or more required top-level skin sections.");
        return false;
    }

    const auto load = [&errors](std::string_view section, std::string_view key, RgbColor *field) -> bool {
        const std::optional<RgbColor> color = ExtractColor(section, key);
        if (!color.has_value()) {
            errors->push_back("Missing or invalid color key: " + std::string(key));
            return false;
        }
        *field = *color;
        return true;
    };

    bool ok = true;
    ok = load(*appChrome, "windowBackground", &paletteOut->windowBackground) && ok;
    ok = load(*appChrome, "windowForeground", &paletteOut->windowForeground) && ok;
    ok = load(*appChrome, "menuBackground", &paletteOut->menuBackground) && ok;
    ok = load(*appChrome, "menuForeground", &paletteOut->menuForeground) && ok;
    ok = load(*appChrome, "statusBackground", &paletteOut->statusBackground) && ok;
    ok = load(*appChrome, "statusForeground", &paletteOut->statusForeground) && ok;
    ok = load(*appChrome, "accent", &paletteOut->accent) && ok;

    ok = load(*editor, "background", &paletteOut->editorBackground) && ok;
    ok = load(*editor, "foreground", &paletteOut->editorForeground) && ok;
    ok = load(*editor, "lineNumberBackground", &paletteOut->lineNumberBackground) && ok;
    ok = load(*editor, "lineNumberForeground", &paletteOut->lineNumberForeground) && ok;
    ok = load(*editor, "selectionBackground", &paletteOut->selectionBackground) && ok;
    ok = load(*editor, "selectionForeground", &paletteOut->selectionForeground) && ok;

    ok = load(*dialogs, "background", &paletteOut->dialogBackground) && ok;
    ok = load(*dialogs, "foreground", &paletteOut->dialogForeground) && ok;
    ok = load(*dialogs, "buttonBackground", &paletteOut->dialogButtonBackground) && ok;
    ok = load(*dialogs, "buttonForeground", &paletteOut->dialogButtonForeground) && ok;
    return ok;
}

double ToLinear(double value) {
    if (value <= 0.04045) {
        return value / 12.92;
    }
    return std::pow((value + 0.055) / 1.055, 2.4);
}

double RelativeLuminance(const RgbColor &color) {
    const double red = ToLinear(static_cast<double>(color.red) / 255.0);
    const double green = ToLinear(static_cast<double>(color.green) / 255.0);
    const double blue = ToLinear(static_cast<double>(color.blue) / 255.0);
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue;
}

double ContrastRatio(const RgbColor &first, const RgbColor &second) {
    const double firstLum = RelativeLuminance(first);
    const double secondLum = RelativeLuminance(second);
    const double lighter = std::max(firstLum, secondLum);
    const double darker = std::min(firstLum, secondLum);
    return (lighter + 0.05) / (darker + 0.05);
}

void RequireContrast(
    std::string_view skinName,
    std::string_view pairName,
    const RgbColor &foreground,
    const RgbColor &background,
    double minimumRatio,
    std::vector<std::string> *errors) {
    const double ratio = ContrastRatio(foreground, background);
    if (ratio + 1e-9 < minimumRatio) {
        std::ostringstream stream;
        stream << skinName << ": contrast check failed for " << pairName
               << " (ratio=" << ratio << ", minimum=" << minimumRatio << ")";
        errors->push_back(stream.str());
    }
}

void ValidateSkin(const std::string &skinName, const SkinPalette &palette, std::vector<std::string> *errors) {
    constexpr double kNormalTextContrast = 4.5;
    constexpr double kSecondaryTextContrast = 3.0;
    constexpr double kFocusIndicatorContrast = 3.0;

    RequireContrast(
        skinName,
        "windowForeground/windowBackground",
        palette.windowForeground,
        palette.windowBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "menuForeground/menuBackground",
        palette.menuForeground,
        palette.menuBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "statusForeground/statusBackground",
        palette.statusForeground,
        palette.statusBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "editorForeground/editorBackground",
        palette.editorForeground,
        palette.editorBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "selectionForeground/selectionBackground",
        palette.selectionForeground,
        palette.selectionBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "lineNumberForeground/lineNumberBackground",
        palette.lineNumberForeground,
        palette.lineNumberBackground,
        kSecondaryTextContrast,
        errors);
    RequireContrast(
        skinName,
        "dialogForeground/dialogBackground",
        palette.dialogForeground,
        palette.dialogBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "dialogButtonForeground/dialogButtonBackground",
        palette.dialogButtonForeground,
        palette.dialogButtonBackground,
        kNormalTextContrast,
        errors);
    RequireContrast(
        skinName,
        "accent/dialogBackground (focus indicator)",
        palette.accent,
        palette.dialogBackground,
        kFocusIndicatorContrast,
        errors);
    RequireContrast(
        skinName,
        "accent/editorBackground (focus indicator)",
        palette.accent,
        palette.editorBackground,
        kFocusIndicatorContrast,
        errors);
}

}  // namespace

int main() {
    const std::filesystem::path sourceRoot = std::filesystem::path(NPP_SOURCE_DIR);
    const std::vector<std::filesystem::path> skinFiles = {
        sourceRoot / "packaging/linux/skins/light.json",
        sourceRoot / "packaging/linux/skins/dark.json",
        sourceRoot / "packaging/linux/skins/high-contrast.json",
    };

    std::vector<std::string> errors;
    for (const auto &skinFile : skinFiles) {
        const std::optional<std::string> json = ReadTextFile(skinFile);
        if (!json.has_value()) {
            errors.push_back("Failed to read skin file: " + skinFile.string());
            continue;
        }

        SkinPalette palette;
        std::vector<std::string> parseErrors;
        if (!ParseSkinPalette(*json, &palette, &parseErrors)) {
            for (const std::string &parseError : parseErrors) {
                errors.push_back(skinFile.filename().string() + ": " + parseError);
            }
            continue;
        }
        ValidateSkin(skinFile.filename().string(), palette, &errors);
    }

    if (!errors.empty()) {
        for (const std::string &error : errors) {
            std::cerr << "[skin-accessibility] " << error << '\n';
        }
        return 1;
    }

    std::cout << "Skin accessibility regression checks passed.\n";
    return 0;
}
