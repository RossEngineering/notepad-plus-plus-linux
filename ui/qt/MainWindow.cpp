#include "MainWindow.h"
#include "LanguageAwareFormatter.h"
#include "LanguageDetection.h"
#include "LexerStyleConfig.h"
#include "LspBaselineFeatures.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDateTime>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QKeySequence>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QSysInfo>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "Scintilla.h"
#include "ScintillaEditBase.h"
#include "SciLexer.h"

#if defined(NPP_HAVE_LEXILLA) && NPP_HAVE_LEXILLA
#include "ILexer.h"
#include "Lexilla.h"
#endif

namespace {

QString FileNameFromPath(const std::string &pathUtf8) {
    if (pathUtf8.empty()) {
        return QStringLiteral("Untitled");
    }
    return QString::fromStdString(std::filesystem::path(pathUtf8).filename().string());
}

sptr_t LineNumberMarginWidth(ScintillaEditBase *editor) {
    return editor->send(
        SCI_TEXTWIDTH,
        STYLE_LINENUMBER,
        reinterpret_cast<sptr_t>("_99999"));
}

const std::map<std::string, QKeySequence> &DefaultShortcutMap() {
    static const std::map<std::string, QKeySequence> kShortcuts = {
        {"file.new", QKeySequence(QStringLiteral("Ctrl+N"))},
        {"file.open", QKeySequence(QStringLiteral("Ctrl+O"))},
        {"file.reload", QKeySequence(QStringLiteral("Ctrl+Shift+R"))},
        {"file.save", QKeySequence(QStringLiteral("Ctrl+S"))},
        {"file.saveAs", QKeySequence(QStringLiteral("Ctrl+Shift+S"))},
        {"tab.close", QKeySequence(QStringLiteral("Ctrl+W"))},
        {"file.quit", QKeySequence(QStringLiteral("Ctrl+Q"))},
        {"edit.find", QKeySequence(QStringLiteral("Ctrl+F"))},
        {"search.findInFiles", QKeySequence(QStringLiteral("Ctrl+Shift+F"))},
        {"edit.replace", QKeySequence(QStringLiteral("Ctrl+H"))},
        {"edit.gotoLine", QKeySequence(QStringLiteral("Ctrl+G"))},
        {"edit.multiCursor.addAbove", QKeySequence(QStringLiteral("Ctrl+Alt+Up"))},
        {"edit.multiCursor.addBelow", QKeySequence(QStringLiteral("Ctrl+Alt+Down"))},
        {"edit.multiCursor.addNextMatch", QKeySequence(QStringLiteral("Ctrl+D"))},
        {"edit.multiCursor.selectAllMatches", QKeySequence(QStringLiteral("Ctrl+Shift+L"))},
        {"edit.formatDocument", QKeySequence(QStringLiteral("Ctrl+Shift+I"))},
        {"edit.preferences", QKeySequence(QStringLiteral("Ctrl+Comma"))},
        {"language.autoDetect", QKeySequence(QStringLiteral("Ctrl+Alt+L"))},
        {"language.lockCurrent", QKeySequence(QStringLiteral("Ctrl+Alt+Shift+L"))},
        {"language.lsp.start", QKeySequence(QStringLiteral("Ctrl+Alt+Enter"))},
        {"language.lsp.stop", QKeySequence(QStringLiteral("Ctrl+Alt+Backspace"))},
        {"language.lsp.hover", QKeySequence(QStringLiteral("Ctrl+K"))},
        {"language.lsp.gotoDefinition", QKeySequence(QStringLiteral("F12"))},
        {"language.lsp.diagnostics", QKeySequence(QStringLiteral("Alt+F8"))},
        {"language.lsp.symbols", QKeySequence(QStringLiteral("Ctrl+Shift+O"))},
        {"language.lsp.rename", QKeySequence(QStringLiteral("F2"))},
        {"language.lsp.codeActions", QKeySequence(QStringLiteral("Ctrl+."))},
        {"tools.commandPalette", QKeySequence(QStringLiteral("Ctrl+Shift+P"))},
        {"tools.runCommand", QKeySequence(QStringLiteral("F5"))},
        {"tools.shortcuts.open", QKeySequence(QStringLiteral("Ctrl+Alt+K"))},
        {"tools.shortcuts.reload", QKeySequence(QStringLiteral("Ctrl+Alt+R"))},
        {"view.split.none", QKeySequence(QStringLiteral("Ctrl+Alt+0"))},
        {"view.split.vertical", QKeySequence(QStringLiteral("Ctrl+Alt+1"))},
        {"view.split.horizontal", QKeySequence(QStringLiteral("Ctrl+Alt+2"))},
        {"view.minimap.toggle", QKeySequence(QStringLiteral("Ctrl+Alt+M"))},
        {"help.wiki", QKeySequence(QStringLiteral("F1"))},
    };
    return kShortcuts;
}

const QString &RepositoryBaseUrl() {
    static const QString kUrl = QStringLiteral("https://github.com/RossEngineering/notepad-plus-plus-linux");
    return kUrl;
}

QString RepositoryUrl(const QString &suffix) {
    return RepositoryBaseUrl() + suffix;
}

bool IsValidUtf8(std::string_view bytes) {
    size_t index = 0;
    while (index < bytes.size()) {
        const unsigned char lead = static_cast<unsigned char>(bytes[index]);
        if ((lead & 0x80U) == 0U) {
            ++index;
            continue;
        }

        int continuationCount = 0;
        if ((lead & 0xE0U) == 0xC0U) {
            continuationCount = 1;
            if (lead < 0xC2U) {
                return false;
            }
        } else if ((lead & 0xF0U) == 0xE0U) {
            continuationCount = 2;
        } else if ((lead & 0xF8U) == 0xF0U) {
            continuationCount = 3;
            if (lead > 0xF4U) {
                return false;
            }
        } else {
            return false;
        }

        if (index + static_cast<size_t>(continuationCount) >= bytes.size()) {
            return false;
        }

        for (int offset = 1; offset <= continuationCount; ++offset) {
            const unsigned char tail = static_cast<unsigned char>(bytes[index + static_cast<size_t>(offset)]);
            if ((tail & 0xC0U) != 0x80U) {
                return false;
            }
        }

        index += static_cast<size_t>(continuationCount) + 1U;
    }
    return true;
}

QString DecodeUtf16(const std::string &bytes, bool littleEndian) {
    const size_t pairCount = bytes.size() / 2U;
    std::u16string codeUnits;
    codeUnits.reserve(pairCount);
    for (size_t i = 0; i + 1U < bytes.size(); i += 2U) {
        const unsigned char b0 = static_cast<unsigned char>(bytes[i]);
        const unsigned char b1 = static_cast<unsigned char>(bytes[i + 1U]);
        const char16_t value = littleEndian
            ? static_cast<char16_t>(b0 | (static_cast<char16_t>(b1) << 8))
            : static_cast<char16_t>((static_cast<char16_t>(b0) << 8) | b1);
        codeUnits.push_back(value);
    }
    return QString::fromUtf16(codeUnits.data(), static_cast<int>(codeUnits.size()));
}

std::string AsciiLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

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

std::vector<std::string> ParseLanguageCsvList(const std::string &csv) {
    std::vector<std::string> languages;
    std::set<std::string> dedupe;
    size_t cursor = 0;
    while (cursor <= csv.size()) {
        const size_t commaPos = csv.find(',', cursor);
        const size_t endPos = commaPos == std::string::npos ? csv.size() : commaPos;
        std::string token = TrimAsciiWhitespace(csv.substr(cursor, endPos - cursor));
        token = AsciiLower(token);
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

QString LanguageCsvListString(const std::vector<std::string> &languages) {
    QStringList parts;
    for (const std::string &language : languages) {
        if (!language.empty()) {
            parts.append(QString::fromStdString(language));
        }
    }
    return parts.join(QStringLiteral(", "));
}

std::string NormalizeFormatterProfile(std::string profile) {
    profile = AsciiLower(TrimAsciiWhitespace(profile));
    if (profile.empty()) {
        return "auto";
    }
    if (profile == "auto" || profile == "builtin" || profile == "extension") {
        return profile;
    }
    if (profile.rfind("extension:", 0) == 0 && profile.size() > std::string("extension:").size()) {
        return profile;
    }
    return "auto";
}

std::string NormalizeUpdateChannel(std::string channel) {
    channel = AsciiLower(TrimAsciiWhitespace(channel));
    if (channel == "candidate") {
        return "candidate";
    }
    return "stable";
}

QString UpdateChannelLabel(const std::string &channel) {
    return NormalizeUpdateChannel(channel) == "candidate"
        ? QStringLiteral("Candidate")
        : QStringLiteral("Stable");
}

QString FormatterProfileToLabel(const std::string &profile) {
    if (profile == "builtin") {
        return QStringLiteral("Built-in only");
    }
    if (profile == "extension") {
        return QStringLiteral("Extension only");
    }
    if (profile.rfind("extension:", 0) == 0) {
        return QStringLiteral("Specific extension (%1)")
            .arg(QString::fromStdString(profile.substr(std::string("extension:").size())));
    }
    return QStringLiteral("Auto");
}

bool IsValidFormatterProfile(std::string_view profile) {
    return profile == "auto" ||
        profile == "builtin" ||
        profile == "extension" ||
        (profile.rfind("extension:", 0) == 0 && profile.size() > std::string_view("extension:").size());
}

bool ParseFormatterOverrideCsv(
    const std::string &csv,
    std::map<std::string, std::string> *overrides,
    std::string *errorMessage) {
    if (overrides == nullptr) {
        return false;
    }
    overrides->clear();

    size_t cursor = 0;
    while (cursor <= csv.size()) {
        const size_t commaPos = csv.find(',', cursor);
        const size_t endPos = commaPos == std::string::npos ? csv.size() : commaPos;
        const std::string token = TrimAsciiWhitespace(csv.substr(cursor, endPos - cursor));
        if (!token.empty()) {
            const size_t equalsPos = token.find('=');
            if (equalsPos == std::string::npos) {
                if (errorMessage) {
                    *errorMessage = "Invalid formatter override entry: '" + token + "' (expected language=profile)";
                }
                overrides->clear();
                return false;
            }

            const std::string language = AsciiLower(TrimAsciiWhitespace(token.substr(0, equalsPos)));
            const std::string profile = AsciiLower(TrimAsciiWhitespace(token.substr(equalsPos + 1)));
            if (language.empty()) {
                if (errorMessage) {
                    *errorMessage = "Invalid formatter override entry: empty language key";
                }
                overrides->clear();
                return false;
            }
            if (!IsValidFormatterProfile(profile)) {
                if (errorMessage) {
                    *errorMessage =
                        "Invalid formatter profile '" + profile +
                        "'. Supported values: auto, builtin, extension, extension:<id>";
                }
                overrides->clear();
                return false;
            }
            overrides->insert_or_assign(language, profile);
        }

        if (commaPos == std::string::npos) {
            break;
        }
        cursor = commaPos + 1;
    }

    return true;
}

QString FormatterOverrideCsvString(const std::map<std::string, std::string> &overrides) {
    QStringList entries;
    for (const auto &[language, profile] : overrides) {
        entries.append(QStringLiteral("%1=%2").arg(QString::fromStdString(language), QString::fromStdString(profile)));
    }
    return entries.join(QStringLiteral(", "));
}

bool IsAsciiIdentifierToken(const std::string &token) {
    if (token.empty()) {
        return false;
    }
    for (char ch : token) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch) != 0 || ch == '_' || ch == '-') {
            continue;
        }
        return false;
    }
    return true;
}

std::string PercentString(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(0) << (value * 100.0);
    return stream.str();
}

std::string PreferredExtensionForLexer(const std::string &lexerName) {
    if (lexerName == "markdown") {
        return ".md";
    }
    if (lexerName == "xml") {
        return ".html";
    }
    if (lexerName == "cpp") {
        return ".cpp";
    }
    if (lexerName == "python") {
        return ".py";
    }
    if (lexerName == "bash") {
        return ".sh";
    }
    if (lexerName == "json") {
        return ".json";
    }
    if (lexerName == "yaml") {
        return ".yaml";
    }
    if (lexerName == "sql") {
        return ".sql";
    }
    return ".txt";
}

std::string LanguageActionIdForLexer(const std::string &lexerName) {
    if (lexerName == "markdown") {
        return "language.set.markdown";
    }
    if (lexerName == "xml") {
        return "language.set.html";
    }
    if (lexerName == "cpp") {
        return "language.set.cpp";
    }
    if (lexerName == "python") {
        return "language.set.python";
    }
    if (lexerName == "bash") {
        return "language.set.bash";
    }
    if (lexerName == "yaml") {
        return "language.set.yaml";
    }
    if (lexerName == "sql") {
        return "language.set.sql";
    }
    return "language.set.plain";
}

std::string SkinActionIdForSkinId(const std::string &skinId) {
    if (skinId == "builtin.dark") {
        return "view.skin.dark";
    }
    if (skinId == "builtin.high_contrast") {
        return "view.skin.highContrast";
    }
    return "view.skin.light";
}

std::string ReplaceAll(std::string text, const std::string& needle, const std::string& replacement) {
    if (needle.empty()) {
        return text;
    }
    size_t position = 0;
    while ((position = text.find(needle, position)) != std::string::npos) {
        text.replace(position, needle.size(), replacement);
        position += replacement.size();
    }
    return text;
}

QStringList BuildFormatterArguments(
    const std::vector<std::string>& argTemplates,
    const std::string& filePathUtf8,
    const std::string& languageId,
    int tabWidth) {
    QStringList args;
    for (const std::string& argTemplate : argTemplates) {
        std::string expanded = argTemplate;
        expanded = ReplaceAll(expanded, "${filePath}", filePathUtf8);
        expanded = ReplaceAll(expanded, "${languageId}", languageId);
        expanded = ReplaceAll(expanded, "${tabWidth}", std::to_string(tabWidth));
        args.append(QString::fromStdString(expanded));
    }
    return args;
}

std::vector<std::string> FormatterLanguageCandidates(
    const std::string& lexerName,
    const std::string& languageId) {
    std::set<std::string> candidates;
    if (!lexerName.empty()) {
        candidates.insert(AsciiLower(lexerName));
    }
    if (!languageId.empty()) {
        candidates.insert(AsciiLower(languageId));
    }
    if (lexerName == "null") {
        candidates.insert("plaintext");
        candidates.insert("plain_text");
        candidates.insert("text");
    }
    if (lexerName == "xml") {
        candidates.insert("html");
        candidates.insert("xml");
    }
    if (lexerName == "cpp") {
        candidates.insert("c");
        candidates.insert("c++");
        candidates.insert("cpp");
    }
    if (lexerName == "bash") {
        candidates.insert("shell");
        candidates.insert("sh");
    }
    return std::vector<std::string>(candidates.begin(), candidates.end());
}

bool FormatterContributionMatchesLanguage(
    const npp::platform::ExtensionManifest::FormatterContribution& contribution,
    const std::string& lexerName,
    const std::string& languageId) {
    const std::vector<std::string> candidates = FormatterLanguageCandidates(lexerName, languageId);
    for (const std::string& language : contribution.languages) {
        const std::string normalized = AsciiLower(language);
        if (std::find(candidates.begin(), candidates.end(), normalized) != candidates.end()) {
            return true;
        }
    }
    return false;
}

bool IsIdentifierChar(char ch) {
    const unsigned char uch = static_cast<unsigned char>(ch);
    return std::isalnum(uch) != 0 || ch == '-' || ch == '_' || ch == ':';
}

int ParseThemeColor(const QJsonObject &obj, const char *key, int fallback) {
    const QJsonValue value = obj.value(QString::fromLatin1(key));
    if (!value.isString()) {
        return fallback;
    }
    const QColor color(value.toString());
    if (!color.isValid()) {
        return fallback;
    }
    return color.blue() << 16 | color.green() << 8 | color.red();
}

int ParseThemeColorFromSection(
    const QJsonObject &root,
    const char *sectionKey,
    const char *key,
    int fallback) {
    const QJsonValue sectionValue = root.value(QString::fromLatin1(sectionKey));
    if (!sectionValue.isObject()) {
        return fallback;
    }
    return ParseThemeColor(sectionValue.toObject(), key, fallback);
}

QString ThemeColorHex(int bgr) {
    const int red = bgr & 0xFF;
    const int green = (bgr >> 8) & 0xFF;
    const int blue = (bgr >> 16) & 0xFF;
    return QStringLiteral("#%1%2%3")
        .arg(red, 2, 16, QLatin1Char('0'))
        .arg(green, 2, 16, QLatin1Char('0'))
        .arg(blue, 2, 16, QLatin1Char('0'))
        .toUpper();
}

int BlendThemeColor(int firstBgr, int secondBgr, double secondWeight) {
    const double clampedWeight = std::clamp(secondWeight, 0.0, 1.0);
    const double firstWeight = 1.0 - clampedWeight;

    const int firstRed = firstBgr & 0xFF;
    const int firstGreen = (firstBgr >> 8) & 0xFF;
    const int firstBlue = (firstBgr >> 16) & 0xFF;

    const int secondRed = secondBgr & 0xFF;
    const int secondGreen = (secondBgr >> 8) & 0xFF;
    const int secondBlue = (secondBgr >> 16) & 0xFF;

    const int mixedRed = static_cast<int>(firstRed * firstWeight + secondRed * clampedWeight + 0.5);
    const int mixedGreen = static_cast<int>(firstGreen * firstWeight + secondGreen * clampedWeight + 0.5);
    const int mixedBlue = static_cast<int>(firstBlue * firstWeight + secondBlue * clampedWeight + 0.5);

    return mixedBlue << 16 | mixedGreen << 8 | mixedRed;
}

struct ExtensionMarketplaceEntry {
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::string homepage;
    std::string sourceDirectoryUtf8;
};

struct ExtensionDirectoryFootprint {
    std::uint64_t totalBytes = 0;
    std::size_t fileCount = 0;
};

std::vector<std::string> SplitVersionTokens(const std::string &version) {
    std::vector<std::string> tokens;
    size_t start = 0;
    for (size_t index = 0; index <= version.size(); ++index) {
        const bool separator = index == version.size() ||
            version[index] == '.' || version[index] == '-' ||
            version[index] == '_' || version[index] == '+';
        if (!separator) {
            continue;
        }
        if (index > start) {
            tokens.push_back(version.substr(start, index - start));
        }
        start = index + 1;
    }
    return tokens;
}

bool IsAsciiNumber(const std::string &token) {
    if (token.empty()) {
        return false;
    }
    return std::all_of(token.begin(), token.end(), [](unsigned char c) {
        return std::isdigit(c) != 0;
    });
}

int CompareLooseVersion(const std::string &left, const std::string &right) {
    if (left == right) {
        return 0;
    }

    const std::vector<std::string> leftTokens = SplitVersionTokens(left);
    const std::vector<std::string> rightTokens = SplitVersionTokens(right);
    const size_t maxSize = std::max(leftTokens.size(), rightTokens.size());
    for (size_t index = 0; index < maxSize; ++index) {
        const std::string leftToken = index < leftTokens.size() ? leftTokens[index] : "0";
        const std::string rightToken = index < rightTokens.size() ? rightTokens[index] : "0";
        if (leftToken == rightToken) {
            continue;
        }

        const bool leftNumeric = IsAsciiNumber(leftToken);
        const bool rightNumeric = IsAsciiNumber(rightToken);
        if (leftNumeric && rightNumeric) {
            long long leftValue = 0;
            long long rightValue = 0;
            try {
                leftValue = std::stoll(leftToken);
                rightValue = std::stoll(rightToken);
            } catch (...) {
                // Fallback to lexical comparison when values overflow.
                const int lexical = leftToken.compare(rightToken);
                return lexical < 0 ? -1 : 1;
            }
            if (leftValue < rightValue) {
                return -1;
            }
            if (leftValue > rightValue) {
                return 1;
            }
            continue;
        }

        if (leftNumeric != rightNumeric) {
            // Prefer numeric tokens over textual qualifiers (for example 1.2.0 > 1.2.0-rc.1).
            return leftNumeric ? 1 : -1;
        }

        const int lexical = leftToken.compare(rightToken);
        return lexical < 0 ? -1 : 1;
    }

    return 0;
}

QString FormatBytesBinary(std::uint64_t bytes) {
    static const char *kUnits[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    double value = static_cast<double>(bytes);
    size_t unitIndex = 0;
    while (value >= 1024.0 && unitIndex + 1 < std::size(kUnits)) {
        value /= 1024.0;
        ++unitIndex;
    }
    const int precision = unitIndex == 0 ? 0 : 1;
    return QStringLiteral("%1 %2")
        .arg(QString::number(value, 'f', precision))
        .arg(QString::fromLatin1(kUnits[unitIndex]));
}

std::optional<ExtensionDirectoryFootprint> ComputeDirectoryFootprint(const std::string &rootUtf8) {
    std::error_code ec;
    const std::filesystem::path rootPath(rootUtf8);
    if (rootUtf8.empty() || !std::filesystem::exists(rootPath, ec) || ec) {
        return std::nullopt;
    }

    ExtensionDirectoryFootprint footprint;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(rootPath, ec)) {
        if (ec) {
            return std::nullopt;
        }
        if (!entry.is_regular_file(ec) || ec) {
            ec.clear();
            continue;
        }
        ++footprint.fileCount;
        const auto size = entry.file_size(ec);
        if (!ec) {
            footprint.totalBytes += static_cast<std::uint64_t>(size);
        }
        ec.clear();
    }
    return footprint;
}

std::vector<ExtensionMarketplaceEntry> ParseMarketplaceIndex(
    const std::string &indexJsonUtf8,
    const std::string &indexPathUtf8,
    QString *errorOut) {
    if (errorOut) {
        errorOut->clear();
    }

    QJsonParseError parseError{};
    const QByteArray bytes(indexJsonUtf8.data(), static_cast<int>(indexJsonUtf8.size()));
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut) {
            *errorOut = QObject::tr("Invalid marketplace index JSON: %1")
                            .arg(parseError.errorString());
        }
        return {};
    }

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("schemaVersion")).toInt(1) != 1) {
        if (errorOut) {
            *errorOut = QObject::tr("Unsupported marketplace schemaVersion in %1")
                            .arg(QString::fromStdString(indexPathUtf8));
        }
        return {};
    }

    const QJsonValue extensionsValue = root.value(QStringLiteral("extensions"));
    if (!extensionsValue.isArray()) {
        if (errorOut) {
            *errorOut = QObject::tr("Marketplace index is missing an extensions array.");
        }
        return {};
    }

    const std::filesystem::path indexDir = std::filesystem::path(indexPathUtf8).parent_path();
    std::vector<ExtensionMarketplaceEntry> entries;
    for (const QJsonValue &entryValue : extensionsValue.toArray()) {
        if (!entryValue.isObject()) {
            continue;
        }
        const QJsonObject entryObject = entryValue.toObject();
        const auto toStdStringUtf8 = [](const QString &value) {
            const QByteArray bytes = value.toUtf8();
            return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
        };

        const std::string id = AsciiLower(TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("id")).toString())));
        const std::string version = TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("version")).toString()));
        const std::string sourceDirectoryRaw = TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("sourceDirectory")).toString()));
        if (id.empty() || version.empty() || sourceDirectoryRaw.empty()) {
            continue;
        }

        std::filesystem::path sourcePath(sourceDirectoryRaw);
        if (sourcePath.is_relative()) {
            sourcePath = indexDir / sourcePath;
        }

        ExtensionMarketplaceEntry entry;
        entry.id = id;
        entry.version = version;
        entry.name = TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("name")).toString()));
        if (entry.name.empty()) {
            entry.name = entry.id;
        }
        entry.description = TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("description")).toString()));
        entry.homepage = TrimAsciiWhitespace(
            toStdStringUtf8(entryObject.value(QStringLiteral("homepage")).toString()));
        entry.sourceDirectoryUtf8 = sourcePath.lexically_normal().string();
        entries.push_back(std::move(entry));
    }

    std::sort(entries.begin(), entries.end(), [](const ExtensionMarketplaceEntry &left, const ExtensionMarketplaceEntry &right) {
        return left.id < right.id;
    });
    return entries;
}

std::vector<npp::platform::LspServerConfig> DefaultLspServerConfigs() {
    using npp::platform::LspServerConfig;
    return {
        LspServerConfig{"cpp", "clangd", {"--background-index"}, {}},
        LspServerConfig{"python", "pylsp", {}, {}},
        LspServerConfig{"bash", "bash-language-server", {"start"}, {}},
        LspServerConfig{"yaml", "yaml-language-server", {"--stdio"}, {}},
        LspServerConfig{"markdown", "marksman", {"server"}, {}},
        LspServerConfig{"sql", "sqls", {}, {}},
        LspServerConfig{"json", "vscode-json-language-server", {"--stdio"}, {}},
        LspServerConfig{"html", "vscode-html-language-server", {"--stdio"}, {}},
    };
}

}  // namespace

MainWindow::MainWindow(bool safeModeNoExtensions, QWidget *parent)
    : QMainWindow(parent),
      _diagnosticsService(_pathService, _fileSystemService),
      _safeModeNoExtensions(safeModeNoExtensions),
      _extensionService(&_pathService, &_fileSystemService, kAppName),
      _lspService(&_processService) {
    BuildUi();
    InitializeLspServers();
    LoadEditorSettings();
    ApplySplitViewModeFromSettings();
    ApplyMinimapStateFromSettings();
    StartCrashRecoveryTimer();
    StartAutoSaveTimer();
    _extensionService.SetPermissionPrompt([this](const npp::platform::PermissionPromptRequest& request) {
        const QStringList options = {
            tr("Deny"),
            tr("Allow Once"),
            tr("Allow Session"),
            tr("Allow Always"),
        };
        bool ok = false;
        const QString selected = QInputDialog::getItem(
            this,
            tr("Extension Permission Request"),
            tr("Extension: %1\nPermission: %2\nReason: %3")
                .arg(ToQString(request.extensionName.empty() ? request.extensionId : request.extensionName))
                .arg(ToQString(request.permission))
                .arg(ToQString(request.reason)),
            options,
            0,
            false,
            &ok);
        if (!ok || selected == options.at(0)) {
            return npp::platform::PermissionGrantMode::kDenied;
        }
        if (selected == options.at(1)) {
            return npp::platform::PermissionGrantMode::kAllowOnce;
        }
        if (selected == options.at(2)) {
            return npp::platform::PermissionGrantMode::kAllowSession;
        }
        return npp::platform::PermissionGrantMode::kAllowAlways;
    });
    if (_safeModeNoExtensions) {
        for (const std::string actionId : {
                 "tools.extensions.install",
                 "tools.extensions.manage",
                 "tools.extensions.marketplace"}) {
            const auto it = _actionsById.find(actionId);
            if (it != _actionsById.end() && it->second) {
                it->second->setEnabled(false);
            }
        }
        statusBar()->showMessage(tr("Safe mode active: extensions are disabled"), 5000);
    } else {
        InitializeExtensionsWithGuardrails();
        NotifyExtensionUpdatesIfAvailable();
    }
    EnsureBuiltInSkins();
    EnsureThemeFile();
    LoadTheme();
    EnsureShortcutConfigFile();
    LoadShortcutOverrides();
    ApplyShortcuts();
    const bool recoveredFromCrash = RestoreCrashRecoveryJournal();
    const bool restoredSession = !recoveredFromCrash &&
        _editorSettings.restoreSessionOnStartup &&
        RestoreSession();
    if (!recoveredFromCrash && !restoredSession) {
        NewTab();
    }
    UpdateWindowTitle();
    UpdateCursorStatus();
    UpdateLanguageActionState();
    UpdateSkinActionState();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    _closingApplication = true;
    for (int index = _tabs->count() - 1; index >= 0; --index) {
        if (!CloseTabAt(index)) {
            _closingApplication = false;
            event->ignore();
            return;
        }
    }

    if (_crashRecoveryTimer) {
        _crashRecoveryTimer->stop();
    }
    if (_autoSaveTimer) {
        _autoSaveTimer->stop();
    }
    SaveSession();
    ClearCrashRecoveryJournal();
    _lspService.StopAllSessions();
    _lspSessionByEditor.clear();
    _closingApplication = false;
    event->accept();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == _minimapEditor && event != nullptr) {
        const bool isMousePress = event->type() == QEvent::MouseButtonPress;
        const bool isMouseDrag = event->type() == QEvent::MouseMove;
        if (isMousePress || isMouseDrag) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (isMouseDrag && (mouseEvent->buttons() & Qt::LeftButton) == 0) {
                return QMainWindow::eventFilter(watched, event);
            }

            ScintillaEditBase *editor = CurrentEditor();
            if (editor && _editorSettings.minimapEnabled) {
                const QPoint position = mouseEvent->position().toPoint();
                const sptr_t documentPos = _minimapEditor->send(
                    SCI_POSITIONFROMPOINTCLOSE,
                    position.x(),
                    position.y());
                if (documentPos >= 0) {
                    const int targetLine = static_cast<int>(_minimapEditor->send(SCI_LINEFROMPOSITION, documentPos));
                    const int visibleLines = std::max(1, static_cast<int>(editor->send(SCI_LINESONSCREEN)));
                    const int targetFirstVisibleLine = std::max(0, targetLine - (visibleLines / 2));
                    editor->send(SCI_SETFIRSTVISIBLELINE, targetFirstVisibleLine);
                    editor->send(SCI_GOTOLINE, targetLine);
                    UpdateMinimapViewportHighlight();
                    return true;
                }
            }
        }
    }

    if (!_closingApplication &&
        _editorSettings.autoSaveOnFocusLost &&
        event != nullptr &&
        event->type() == QEvent::FocusOut) {
        auto *editor = qobject_cast<ScintillaEditBase *>(watched);
        if (editor && _editorStates.find(editor) != _editorStates.end()) {
            std::string savedPathUtf8;
            if (AutoSaveEditorIfNeeded(editor, &savedPathUtf8)) {
                statusBar()->showMessage(
                    tr("Auto-saved %1 (focus lost).").arg(FileNameFromPath(savedPathUtf8)),
                    1500);
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::BuildUi() {
    resize(1100, 760);
    BuildTabs();
    BuildActions();
    BuildMenus();
    BuildStatusBar();
}

void MainWindow::BuildMenus() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(_actionsById.at("file.new"));
    fileMenu->addAction(_actionsById.at("file.open"));
    fileMenu->addAction(_actionsById.at("file.reload"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.save"));
    fileMenu->addAction(_actionsById.at("file.saveAs"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("tab.close"));
    fileMenu->addSeparator();
    fileMenu->addAction(_actionsById.at("file.quit"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(_actionsById.at("edit.find"));
    editMenu->addAction(_actionsById.at("search.findInFiles"));
    editMenu->addAction(_actionsById.at("edit.replace"));
    editMenu->addAction(_actionsById.at("edit.gotoLine"));
    QMenu *multiCursorMenu = editMenu->addMenu(tr("Multi-Cursor"));
    multiCursorMenu->addAction(_actionsById.at("edit.multiCursor.addAbove"));
    multiCursorMenu->addAction(_actionsById.at("edit.multiCursor.addBelow"));
    multiCursorMenu->addSeparator();
    multiCursorMenu->addAction(_actionsById.at("edit.multiCursor.addNextMatch"));
    multiCursorMenu->addAction(_actionsById.at("edit.multiCursor.selectAllMatches"));
    editMenu->addAction(_actionsById.at("edit.formatDocument"));
    editMenu->addSeparator();
    editMenu->addAction(_actionsById.at("edit.preferences"));

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(_actionsById.at("tools.commandPalette"));
    toolsMenu->addAction(_actionsById.at("tools.runCommand"));
    toolsMenu->addSeparator();
    toolsMenu->addAction(_actionsById.at("tools.extensions.install"));
    toolsMenu->addAction(_actionsById.at("tools.extensions.manage"));
    toolsMenu->addAction(_actionsById.at("tools.extensions.marketplace"));
    toolsMenu->addSeparator();
    toolsMenu->addAction(_actionsById.at("tools.safeMode.restart"));
    toolsMenu->addSeparator();
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.open"));
    toolsMenu->addAction(_actionsById.at("tools.shortcuts.reload"));

    QMenu *languageMenu = menuBar()->addMenu(tr("&Language"));
    languageMenu->addAction(_actionsById.at("language.autoDetect"));
    languageMenu->addAction(_actionsById.at("language.lockCurrent"));
    languageMenu->addSeparator();
    languageMenu->addAction(_actionsById.at("language.set.plain"));
    languageMenu->addAction(_actionsById.at("language.set.markdown"));
    languageMenu->addAction(_actionsById.at("language.set.html"));
    languageMenu->addAction(_actionsById.at("language.set.cpp"));
    languageMenu->addAction(_actionsById.at("language.set.python"));
    languageMenu->addAction(_actionsById.at("language.set.bash"));
    languageMenu->addAction(_actionsById.at("language.set.yaml"));
    languageMenu->addAction(_actionsById.at("language.set.sql"));
    languageMenu->addSeparator();
    languageMenu->addAction(_actionsById.at("language.lsp.start"));
    languageMenu->addAction(_actionsById.at("language.lsp.stop"));
    languageMenu->addAction(_actionsById.at("language.lsp.hover"));
    languageMenu->addAction(_actionsById.at("language.lsp.gotoDefinition"));
    languageMenu->addAction(_actionsById.at("language.lsp.diagnostics"));
    languageMenu->addAction(_actionsById.at("language.lsp.symbols"));
    languageMenu->addAction(_actionsById.at("language.lsp.rename"));
    languageMenu->addAction(_actionsById.at("language.lsp.codeActions"));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu *skinsMenu = viewMenu->addMenu(tr("Skins"));
    skinsMenu->addAction(_actionsById.at("view.skin.light"));
    skinsMenu->addAction(_actionsById.at("view.skin.dark"));
    skinsMenu->addAction(_actionsById.at("view.skin.highContrast"));
    viewMenu->addSeparator();
    QMenu *splitMenu = viewMenu->addMenu(tr("Split Editor"));
    splitMenu->addAction(_actionsById.at("view.split.none"));
    splitMenu->addAction(_actionsById.at("view.split.vertical"));
    splitMenu->addAction(_actionsById.at("view.split.horizontal"));
    viewMenu->addAction(_actionsById.at("view.minimap.toggle"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(_actionsById.at("help.docs"));
    helpMenu->addAction(_actionsById.at("help.wiki"));
    helpMenu->addAction(_actionsById.at("help.checkUpdates"));
    helpMenu->addSeparator();
    helpMenu->addAction(_actionsById.at("help.reportBug"));
    helpMenu->addAction(_actionsById.at("help.requestFeature"));
    helpMenu->addSeparator();
    helpMenu->addAction(_actionsById.at("help.about"));
}

void MainWindow::BuildStatusBar() {
    _cursorStatusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(_cursorStatusLabel);
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::BuildActions() {
    const auto registerAction = [this](const std::string &id, const QString &label, auto handler) {
        QAction *action = new QAction(label, this);
        connect(action, &QAction::triggered, this, handler);
        _actionsById.insert_or_assign(id, action);
    };

    registerAction("file.new", tr("New Tab"), [this]() { NewTab(); });
    registerAction("file.open", tr("Open..."), [this]() { OpenFile(); });
    registerAction("file.reload", tr("Reload"), [this]() { ReloadCurrentFile(); });
    registerAction("file.save", tr("Save"), [this]() { SaveCurrentFile(); });
    registerAction("file.saveAs", tr("Save As..."), [this]() { SaveCurrentFileAs(); });
    registerAction("tab.close", tr("Close Tab"), [this]() { CloseTab(); });
    registerAction("file.quit", tr("Quit"), [this]() { close(); });

    registerAction("edit.find", tr("Find..."), [this]() { OnFind(); });
    registerAction("search.findInFiles", tr("Find in Files..."), [this]() { OnFindInFiles(); });
    registerAction("edit.replace", tr("Replace..."), [this]() { OnReplace(); });
    registerAction("edit.gotoLine", tr("Go To Line..."), [this]() { OnGoToLine(); });
    registerAction("edit.multiCursor.addAbove", tr("Add Caret Above"), [this]() { OnMultiCursorAddCaretAbove(); });
    registerAction("edit.multiCursor.addBelow", tr("Add Caret Below"), [this]() { OnMultiCursorAddCaretBelow(); });
    registerAction("edit.multiCursor.addNextMatch", tr("Add Next Match"), [this]() { OnMultiCursorAddNextMatch(); });
    registerAction("edit.multiCursor.selectAllMatches", tr("Select All Matches"), [this]() { OnMultiCursorSelectAllMatches(); });
    registerAction("edit.formatDocument", tr("Format Document"), [this]() { OnFormatDocument(); });
    registerAction("edit.preferences", tr("Preferences..."), [this]() { OnPreferences(); });
    registerAction("tools.commandPalette", tr("Command Palette..."), [this]() { OnCommandPalette(); });
    registerAction("tools.runCommand", tr("Run Command..."), [this]() { OnRunCommand(); });
    registerAction(
        "tools.extensions.install",
        tr("Install Extension from Folder..."),
        [this]() { OnInstallExtensionFromDirectory(); });
    registerAction(
        "tools.extensions.manage",
        tr("Manage Extensions..."),
        [this]() { OnManageExtensions(); });
    registerAction(
        "tools.extensions.marketplace",
        tr("Extension Marketplace (Local Index)..."),
        [this]() { OnOpenExtensionMarketplace(); });
    registerAction(
        "tools.safeMode.restart",
        _safeModeNoExtensions
            ? tr("Relaunch in Normal Mode")
            : tr("Relaunch in Safe Mode (No Extensions)"),
        [this]() { OnRestartSafeMode(); });
    registerAction("language.autoDetect", tr("Auto Detect Language"), [this]() { OnAutoDetectLanguage(); });
    registerAction("language.lockCurrent", tr("Lock Current Language"), [this]() { OnToggleLexerLock(); });
    _actionsById.at("language.lockCurrent")->setCheckable(true);
    registerAction("language.set.plain", tr("Plain Text"), [this]() { SetCurrentEditorManualLexer("null"); });
    registerAction("language.set.markdown", tr("Markdown"), [this]() { SetCurrentEditorManualLexer("markdown"); });
    registerAction("language.set.html", tr("HTML/XML"), [this]() { SetCurrentEditorManualLexer("xml"); });
    registerAction("language.set.cpp", tr("C/C++"), [this]() { SetCurrentEditorManualLexer("cpp"); });
    registerAction("language.set.python", tr("Python"), [this]() { SetCurrentEditorManualLexer("python"); });
    registerAction("language.set.bash", tr("Bash/Shell"), [this]() { SetCurrentEditorManualLexer("bash"); });
    registerAction("language.set.yaml", tr("YAML"), [this]() { SetCurrentEditorManualLexer("yaml"); });
    registerAction("language.set.sql", tr("SQL"), [this]() { SetCurrentEditorManualLexer("sql"); });
    registerAction("language.lsp.start", tr("Start Language Server"), [this]() { OnLspStartSession(); });
    registerAction("language.lsp.stop", tr("Stop Language Server"), [this]() { OnLspStopSession(); });
    registerAction("language.lsp.hover", tr("Hover (Baseline)"), [this]() { OnLspShowHover(); });
    registerAction("language.lsp.gotoDefinition", tr("Go To Definition (Baseline)"), [this]() { OnLspGoToDefinition(); });
    registerAction("language.lsp.diagnostics", tr("Diagnostics (Baseline)"), [this]() { OnLspShowDiagnostics(); });
    registerAction("language.lsp.symbols", tr("Document Symbols (Baseline)"), [this]() { OnLspShowDocumentSymbols(); });
    registerAction("language.lsp.rename", tr("Rename Symbol (Baseline)"), [this]() { OnLspRenameSymbol(); });
    registerAction("language.lsp.codeActions", tr("Code Actions (Baseline)"), [this]() { OnLspCodeActions(); });
    _actionsById.at("language.set.plain")->setCheckable(true);
    _actionsById.at("language.set.markdown")->setCheckable(true);
    _actionsById.at("language.set.html")->setCheckable(true);
    _actionsById.at("language.set.cpp")->setCheckable(true);
    _actionsById.at("language.set.python")->setCheckable(true);
    _actionsById.at("language.set.bash")->setCheckable(true);
    _actionsById.at("language.set.yaml")->setCheckable(true);
    _actionsById.at("language.set.sql")->setCheckable(true);

    registerAction("view.skin.light", tr("Light"), [this]() { OnSetSkin("builtin.light"); });
    registerAction("view.skin.dark", tr("Dark"), [this]() { OnSetSkin("builtin.dark"); });
    registerAction("view.skin.highContrast", tr("High Contrast"), [this]() { OnSetSkin("builtin.high_contrast"); });
    registerAction("view.split.none", tr("No Split"), [this]() { OnDisableSplitView(); });
    registerAction("view.split.vertical", tr("Vertical Split"), [this]() { OnEnableSplitVertical(); });
    registerAction("view.split.horizontal", tr("Horizontal Split"), [this]() { OnEnableSplitHorizontal(); });
    registerAction("view.minimap.toggle", tr("Show Minimap"), [this]() { OnToggleMinimap(); });
    _actionsById.at("view.skin.light")->setCheckable(true);
    _actionsById.at("view.skin.dark")->setCheckable(true);
    _actionsById.at("view.skin.highContrast")->setCheckable(true);
    _actionsById.at("view.split.none")->setCheckable(true);
    _actionsById.at("view.split.vertical")->setCheckable(true);
    _actionsById.at("view.split.horizontal")->setCheckable(true);
    _actionsById.at("view.minimap.toggle")->setCheckable(true);
    _actionsById.at("view.split.none")->setChecked(true);

    registerAction(
        "tools.shortcuts.open",
        tr("Open Shortcut Overrides"),
        [this]() { OpenShortcutConfigFile(); });
    registerAction(
        "tools.shortcuts.reload",
        tr("Reload Shortcut Overrides"),
        [this]() { ReloadShortcuts(); });

    registerAction("help.docs", tr("Open Help Docs"), [this]() { OnOpenHelpDocs(); });
    registerAction("help.wiki", tr("Open Project Wiki"), [this]() { OnOpenHelpWiki(); });
    registerAction("help.checkUpdates", tr("Check for Updates"), [this]() { OnCheckForUpdates(); });
    registerAction("help.reportBug", tr("Report Bug..."), [this]() { OnReportBug(); });
    registerAction("help.requestFeature", tr("Request Feature..."), [this]() { OnRequestFeature(); });
    registerAction("help.about", tr("About Notepad++ Linux"), [this]() { OnAboutDialog(); });
}

void MainWindow::BuildTabs() {
    _centralHost = new QWidget(this);
    auto *hostLayout = new QVBoxLayout(_centralHost);
    hostLayout->setContentsMargins(0, 0, 0, 0);
    hostLayout->setSpacing(0);

    _rootSplitter = new QSplitter(Qt::Horizontal, _centralHost);
    _editorSplit = new QSplitter(Qt::Horizontal, _rootSplitter);

    _tabs = new QTabWidget(_editorSplit);
    _tabs->setTabsClosable(true);
    _tabs->setMovable(true);
    _tabs->setDocumentMode(true);

    _splitEditor = CreateEditor();
    _splitEditor->setParent(_editorSplit);
    _splitEditor->hide();

    _editorSplit->addWidget(_tabs);
    _editorSplit->addWidget(_splitEditor);
    _editorSplit->setStretchFactor(0, 1);
    _editorSplit->setStretchFactor(1, 1);

    _minimapEditor = CreateEditor();
    _minimapEditor->setParent(_rootSplitter);
    _minimapEditor->send(SCI_SETREADONLY, 1);
    _minimapEditor->send(SCI_SETMARGINWIDTHN, 0, 0);
    _minimapEditor->send(SCI_SETCARETSTYLE, CARETSTYLE_INVISIBLE);
    _minimapEditor->send(SCI_SETVSCROLLBAR, 0);
    _minimapEditor->send(SCI_SETHSCROLLBAR, 0);
    _minimapEditor->send(SCI_SETZOOM, -8);
    _minimapEditor->installEventFilter(this);
    _minimapEditor->hide();

    _rootSplitter->addWidget(_editorSplit);
    _rootSplitter->addWidget(_minimapEditor);
    _rootSplitter->setStretchFactor(0, 1);
    _rootSplitter->setStretchFactor(1, 0);

    hostLayout->addWidget(_rootSplitter);

    connect(_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        CloseTabAt(index);
    });

    connect(_tabs, &QTabWidget::currentChanged, this, [this](int) {
        SyncAuxiliaryEditorsToCurrentTab();
        UpdateMinimapViewportHighlight();
        UpdateWindowTitle();
        UpdateCursorStatus();
        UpdateLanguageActionState();
    });

    connect(_splitEditor, &ScintillaEditBase::notifyChange, this, [this]() {
        ScintillaEditBase *editor = CurrentEditor();
        if (!editor) {
            return;
        }
        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end()) {
            return;
        }
        stateIt->second.dirty = true;
        UpdateTabTitle(editor);
        UpdateMinimapViewportHighlight();
    });
    connect(_splitEditor, &ScintillaEditBase::savePointChanged, this, [this](bool dirty) {
        ScintillaEditBase *editor = CurrentEditor();
        if (!editor) {
            return;
        }
        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end()) {
            return;
        }
        stateIt->second.dirty = dirty;
        UpdateTabTitle(editor);
    });
    connect(_splitEditor, &ScintillaEditBase::updateUi, this, [this](Scintilla::Update) {
        UpdateMinimapViewportHighlight();
    });

    setCentralWidget(_centralHost);
}

ScintillaEditBase *MainWindow::CreateEditor() {
    auto *editor = new ScintillaEditBase(this);

    editor->send(SCI_SETCODEPAGE, SC_CP_UTF8);
    editor->send(SCI_SETUNDOCOLLECTION, 1);
    editor->sends(SCI_STYLESETFONT, STYLE_DEFAULT, "Noto Sans Mono");
    editor->send(SCI_STYLESETSIZE, STYLE_DEFAULT, 11);
    editor->send(SCI_STYLECLEARALL);
    editor->send(SCI_SETCARETLINEVISIBLE, 1);
    editor->send(SCI_SETCARETLINEBACK, 0xF6F6F6);
    editor->send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    editor->send(SCI_SETMARGINWIDTHN, 0, LineNumberMarginWidth(editor));
    editor->send(SCI_SETWRAPMODE, SC_WRAP_NONE);
    editor->send(SCI_SETTABWIDTH, 4);
    editor->send(SCI_SETMULTIPLESELECTION, 1);
    editor->send(SCI_SETADDITIONALSELECTIONTYPING, 1);
    editor->send(SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH);
    editor->send(SCI_GRABFOCUS);
    editor->installEventFilter(this);

    connect(editor, &ScintillaEditBase::notifyChange, this, [this, editor]() {
        auto it = _editorStates.find(editor);
        if (it == _editorStates.end()) {
            return;
        }
        it->second.dirty = true;
        if (_editorSettings.autoDetectLanguage &&
            !it->second.lexerManualLock &&
            (it->second.lexerName.empty() || it->second.lexerName == "null")) {
            AutoDetectAndApplyLexer(editor, it->second.filePathUtf8, GetEditorText(editor), "edit");
        }
        UpdateTabTitle(editor);
    });

    connect(editor, &ScintillaEditBase::savePointChanged, this, [this, editor](bool dirty) {
        auto it = _editorStates.find(editor);
        if (it == _editorStates.end()) {
            return;
        }
        it->second.dirty = dirty;
        UpdateTabTitle(editor);
    });

    connect(editor, &ScintillaEditBase::updateUi, this, [this, editor](Scintilla::Update) {
        if (editor == CurrentEditor() &&
            _splitEditor &&
            _editorSettings.splitViewMode != kSplitModeDisabled &&
            _splitEditor->isVisible()) {
            _splitEditor->send(SCI_SETFIRSTVISIBLELINE, editor->send(SCI_GETFIRSTVISIBLELINE));
        }
        if (editor != _minimapEditor) {
            UpdateCursorStatus();
            UpdateMinimapViewportHighlight();
        }
    });

    connect(editor, &ScintillaEditBase::charAdded, this, [this, editor](int ch) {
        MaybeAutoCloseDelimiterPair(editor, ch);
        MaybeAutoCloseHtmlTag(editor, ch);
    });

    ApplyTheme(editor);
    ApplyLexerByName(editor, "null");
    ApplyEditorSettings(editor);

    return editor;
}

ScintillaEditBase *MainWindow::CurrentEditor() const {
    if (!_tabs) {
        return nullptr;
    }
    return qobject_cast<ScintillaEditBase *>(_tabs->currentWidget());
}

ScintillaEditBase *MainWindow::EditorAt(int index) const {
    if (!_tabs) {
        return nullptr;
    }
    return qobject_cast<ScintillaEditBase *>(_tabs->widget(index));
}

void MainWindow::NewTab() {
    auto *editor = CreateEditor();
    EditorState state;
    state.lexerName = "null";
    _editorStates.insert_or_assign(editor, state);
    ApplyLexerByName(editor, state.lexerName);

    const int index = _tabs->addTab(editor, tr("Untitled"));
    _tabs->setCurrentIndex(index);
    editor->setFocus();
    UpdateLanguageActionState();
}

bool MainWindow::OpenPathFromCli(const QString &path) {
    const QString normalized = QDir::fromNativeSeparators(path).trimmed();
    if (normalized.isEmpty()) {
        return false;
    }

    auto *editor = CurrentEditor();
    if (!editor) {
        NewTab();
        editor = CurrentEditor();
    }

    auto &state = _editorStates[editor];
    if (!state.filePathUtf8.empty() || !GetEditorText(editor).empty() || state.dirty) {
        NewTab();
        editor = CurrentEditor();
    }

    if (!editor) {
        return false;
    }
    return LoadFileIntoEditor(editor, ToUtf8(normalized));
}

bool MainWindow::OpenMostRecentFileFromSession() {
    EnsureConfigRoot();
    const std::string sessionPath = SessionFilePath();
    const auto exists = _fileSystemService.Exists(sessionPath);
    if (!exists.ok() || !(*exists.value)) {
        return false;
    }

    const auto sessionJson = _fileSystemService.ReadTextFile(sessionPath);
    if (!sessionJson.ok()) {
        return false;
    }

    QJsonParseError parseError{};
    const QByteArray jsonBytes(sessionJson.value->c_str(), static_cast<int>(sessionJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray files = root.value(QStringLiteral("openFiles")).toArray();
    if (files.isEmpty()) {
        return false;
    }

    const int fileCount = static_cast<int>(files.size());
    const int preferredIndex = std::clamp(root.value(QStringLiteral("activeIndex")).toInt(0), 0, fileCount - 1);
    std::vector<int> candidateIndices;
    candidateIndices.reserve(static_cast<size_t>(files.size()));
    candidateIndices.push_back(preferredIndex);
    for (int i = 0; i < files.size(); ++i) {
        if (i != preferredIndex) {
            candidateIndices.push_back(i);
        }
    }

    for (const int index : candidateIndices) {
        const QJsonValue value = files.at(index);
        if (!value.isString()) {
            continue;
        }
        const QString candidatePath = value.toString().trimmed();
        if (candidatePath.isEmpty()) {
            continue;
        }
        const auto fileExists = _fileSystemService.Exists(ToUtf8(candidatePath));
        if (!fileExists.ok() || !(*fileExists.value)) {
            continue;
        }
        if (OpenPathFromCli(candidatePath)) {
            return true;
        }
    }
    return false;
}

void MainWindow::OpenFile() {
    QString initialDirectory;
    auto *editor = CurrentEditor();
    if (editor) {
        const auto stateIt = _editorStates.find(editor);
        if (stateIt != _editorStates.end() && !stateIt->second.filePathUtf8.empty()) {
            initialDirectory = QFileInfo(ToQString(stateIt->second.filePathUtf8)).absolutePath();
        }
    }
    if (initialDirectory.isEmpty()) {
        initialDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    if (initialDirectory.isEmpty()) {
        initialDirectory = QDir::homePath();
    }

    QFileDialog dialog(this, tr("Open File"), initialDirectory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const QStringList selected = dialog.selectedFiles();
    const QString chosen = selected.isEmpty() ? QString{} : selected.front();
    if (chosen.isEmpty()) {
        return;
    }

    if (OpenPathFromCli(chosen)) {
        statusBar()->showMessage(tr("Opened %1").arg(chosen), 2000);
    }
}

void MainWindow::ReloadCurrentFile() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end() || stateIt->second.filePathUtf8.empty()) {
        QMessageBox::information(this, tr("Reload"), tr("Current tab has no file on disk."));
        return;
    }

    if (stateIt->second.dirty) {
        const auto choice = QMessageBox::question(
            this,
            tr("Reload"),
            tr("Discard unsaved changes and reload from disk?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }

    if (LoadFileIntoEditor(editor, stateIt->second.filePathUtf8)) {
        statusBar()->showMessage(tr("Reloaded %1").arg(ToQString(stateIt->second.filePathUtf8)), 2000);
    }
}

bool MainWindow::SaveCurrentFile() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return false;
    }

    auto &state = _editorStates[editor];
    if (state.filePathUtf8.empty()) {
        return SaveCurrentFileAs();
    }

    if (!SaveEditorToFile(editor, state.filePathUtf8)) {
        return false;
    }

    statusBar()->showMessage(tr("Saved %1").arg(ToQString(state.filePathUtf8)), 2000);
    return true;
}

bool MainWindow::SaveCurrentFileAs() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return false;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }

    const std::string lexerName = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string suggestedExtension = PreferredExtensionForLexer(lexerName);

    QString suggestedPath;
    if (!stateIt->second.filePathUtf8.empty()) {
        suggestedPath = ToQString(stateIt->second.filePathUtf8);
    } else {
        QString defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (defaultDirectory.isEmpty()) {
            defaultDirectory = QDir::homePath();
        }
        const QString defaultFileName = QStringLiteral("untitled%1").arg(ToQString(suggestedExtension));
        suggestedPath = QDir(defaultDirectory).filePath(defaultFileName);
    }

    QFileDialog dialog(this, tr("Save File As"), suggestedPath);
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    const QStringList selected = dialog.selectedFiles();
    QString target = selected.isEmpty() ? QString{} : selected.front();
    if (target.isEmpty()) {
        return false;
    }

    const std::filesystem::path chosenPath(ToUtf8(target));
    if (chosenPath.extension().empty() && !suggestedExtension.empty()) {
        target += ToQString(suggestedExtension);
    }

    if (!SaveEditorToFile(editor, ToUtf8(target))) {
        return false;
    }

    statusBar()->showMessage(tr("Saved %1").arg(target), 2000);
    return true;
}

bool MainWindow::CloseTabAt(int index) {
    ScintillaEditBase *editor = EditorAt(index);
    if (!editor) {
        return false;
    }

    auto stateIt = _editorStates.find(editor);
    if (stateIt != _editorStates.end() && stateIt->second.dirty) {
        const QString label = FileNameFromPath(stateIt->second.filePathUtf8);
        const auto choice = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("Save changes to %1 before closing?").arg(label),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);

        if (choice == QMessageBox::Cancel) {
            return false;
        }
        if (choice == QMessageBox::Yes && !_tabs->widget(index)->isHidden()) {
            _tabs->setCurrentIndex(index);
            if (!SaveCurrentFile()) {
                return false;
            }
        }
    }

    StopLspSessionForEditor(editor);
    _tabs->removeTab(index);
    _editorStates.erase(editor);
    editor->deleteLater();

    if (_tabs->count() == 0 && !_closingApplication) {
        NewTab();
    }

    return true;
}

void MainWindow::CloseTab() {
    CloseTabAt(_tabs->currentIndex());
}

void MainWindow::UpdateTabTitle(ScintillaEditBase *editor) {
    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }

    const QString base = FileNameFromPath(stateIt->second.filePathUtf8);
    const QString title = stateIt->second.dirty ? QStringLiteral("*%1").arg(base) : base;
    const int index = _tabs->indexOf(editor);
    if (index >= 0) {
        _tabs->setTabText(index, title);
    }

    UpdateWindowTitle();
}

void MainWindow::UpdateWindowTitle() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        setWindowTitle(QStringLiteral("Notepad++ Linux"));
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        setWindowTitle(QStringLiteral("Notepad++ Linux"));
        return;
    }

    const QString fileName = FileNameFromPath(stateIt->second.filePathUtf8);
    setWindowTitle(tr("%1 - Notepad++ Linux").arg(fileName));
}

void MainWindow::UpdateCursorStatus() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        _cursorStatusLabel->setText(tr("Ln 1, Col 1"));
        return;
    }

    const auto position = static_cast<int>(editor->send(SCI_GETCURRENTPOS));
    const auto line = static_cast<int>(editor->send(SCI_LINEFROMPOSITION, position));
    const auto column = static_cast<int>(editor->send(SCI_GETCOLUMN, position));

    _cursorStatusLabel->setText(tr("Ln %1, Col %2").arg(line + 1).arg(column + 1));
}

void MainWindow::OnFind() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8.empty() ? GetSelectedText(editor) : _lastFindUtf8));
    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);
    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    if (needleUtf8.empty()) {
        return;
    }

    _lastFindUtf8 = needleUtf8;
    if (!FindNextInEditor(editor, needleUtf8, matchCaseCheck->isChecked())) {
        QMessageBox::information(this, tr("Find"), tr("No matches found."));
    }
}

void MainWindow::OnReplace() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    enum class ReplaceAction {
        kNone = 0,
        kSingle,
        kAll,
    };

    ReplaceAction action = ReplaceAction::kNone;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Replace"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8.empty() ? GetSelectedText(editor) : _lastFindUtf8));
    auto *replacementEdit = new QLineEdit(&dialog);
    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);

    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(tr("Replace with:"), replacementEdit);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    QPushButton *replaceButton = buttons->addButton(tr("Replace"), QDialogButtonBox::AcceptRole);
    QPushButton *replaceAllButton = buttons->addButton(tr("Replace All"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    connect(replaceButton, &QPushButton::clicked, &dialog, [&dialog, &action]() {
        action = ReplaceAction::kSingle;
        dialog.accept();
    });
    connect(replaceAllButton, &QPushButton::clicked, &dialog, [&dialog, &action]() {
        action = ReplaceAction::kAll;
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    if (needleUtf8.empty()) {
        return;
    }

    const std::string replacementUtf8 = ToUtf8(replacementEdit->text());
    const bool matchCase = matchCaseCheck->isChecked();
    _lastFindUtf8 = needleUtf8;

    if (action == ReplaceAction::kSingle) {
        if (!FindNextInEditor(editor, needleUtf8, matchCase)) {
            QMessageBox::information(this, tr("Replace"), tr("No matches found."));
            return;
        }
        editor->sends(SCI_REPLACESEL, 0, replacementUtf8.c_str());
        return;
    }

    const int replacements = ReplaceAllInEditor(editor, needleUtf8, replacementUtf8, matchCase);
    QMessageBox::information(
        this,
        tr("Replace All"),
        tr("Replaced %1 occurrence(s).").arg(replacements));
}

void MainWindow::OnFindInFiles() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find in Files"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *needleEdit = new QLineEdit(&dialog);
    needleEdit->setText(ToQString(_lastFindUtf8));

    auto *folderEdit = new QLineEdit(&dialog);
    QString defaultRoot = QDir::homePath();
    ScintillaEditBase *editor = CurrentEditor();
    if (editor) {
        const auto stateIt = _editorStates.find(editor);
        if (stateIt != _editorStates.end() && !stateIt->second.filePathUtf8.empty()) {
            defaultRoot = QString::fromStdString(
                std::filesystem::path(stateIt->second.filePathUtf8).parent_path().string());
        }
    }
    folderEdit->setText(defaultRoot);

    auto *browseButton = new QPushButton(tr("Browse..."), &dialog);
    auto *folderLayout = new QHBoxLayout();
    folderLayout->addWidget(folderEdit);
    folderLayout->addWidget(browseButton);

    auto *matchCaseCheck = new QCheckBox(tr("Match case"), &dialog);

    form->addRow(tr("Find what:"), needleEdit);
    form->addRow(tr("Folder:"), folderLayout);
    form->addRow(QString(), matchCaseCheck);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(browseButton, &QPushButton::clicked, &dialog, [this, folderEdit]() {
        const QString selected = QFileDialog::getExistingDirectory(
            this,
            tr("Select Search Root"),
            folderEdit->text());
        if (!selected.isEmpty()) {
            folderEdit->setText(selected);
        }
    });
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const std::string needleUtf8 = ToUtf8(needleEdit->text());
    const std::string rootPathUtf8 = ToUtf8(folderEdit->text());
    if (needleUtf8.empty() || rootPathUtf8.empty()) {
        return;
    }

    _lastFindUtf8 = needleUtf8;
    npp::platform::ListDirectoryOptions listOptions;
    listOptions.recursive = true;
    listOptions.includeDirectories = false;

    const auto listed = _fileSystemService.ListDirectory(rootPathUtf8, listOptions);
    if (!listed.ok()) {
        QMessageBox::warning(
            this,
            tr("Find in Files"),
            tr("Unable to enumerate folder:\n%1").arg(ToQString(listed.status.message)));
        return;
    }

    const bool matchCase = matchCaseCheck->isChecked();
    const std::string needleCmp = matchCase ? needleUtf8 : AsciiLower(needleUtf8);

    std::ostringstream report;
    int matchCount = 0;
    constexpr int kMaxReportedMatches = 500;
    for (const std::string &path : *listed.value) {
        const auto contentOr = _fileSystemService.ReadTextFile(path);
        if (!contentOr.ok()) {
            continue;
        }

        TextEncoding fileEncoding = TextEncoding::kUtf8;
        std::string utf8Content;
        if (!DecodeTextForEditor(*contentOr.value, &fileEncoding, &utf8Content)) {
            continue;
        }

        std::istringstream stream(utf8Content);
        std::string line;
        int lineNumber = 0;
        while (std::getline(stream, line)) {
            ++lineNumber;
            const std::string lineCmp = matchCase ? line : AsciiLower(line);
            if (lineCmp.find(needleCmp) == std::string::npos) {
                continue;
            }

            ++matchCount;
            if (matchCount <= kMaxReportedMatches) {
                report << path << ":" << lineNumber << ": " << line << "\n";
            }
        }
    }

    if (matchCount == 0) {
        QMessageBox::information(this, tr("Find in Files"), tr("No matches found."));
        return;
    }

    if (matchCount > kMaxReportedMatches) {
        report << "\n... output truncated after " << kMaxReportedMatches << " matches.\n";
    }

    QDialog resultsDialog(this);
    resultsDialog.resize(900, 600);
    resultsDialog.setWindowTitle(tr("Find in Files Results (%1 matches)").arg(matchCount));
    auto *resultsLayout = new QVBoxLayout(&resultsDialog);
    auto *resultsText = new QTextEdit(&resultsDialog);
    resultsText->setReadOnly(true);
    resultsText->setPlainText(QString::fromUtf8(report.str().c_str()));
    resultsLayout->addWidget(resultsText);
    auto *closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &resultsDialog);
    connect(closeButtons, &QDialogButtonBox::rejected, &resultsDialog, &QDialog::reject);
    connect(closeButtons, &QDialogButtonBox::accepted, &resultsDialog, &QDialog::accept);
    resultsLayout->addWidget(closeButtons);
    resultsDialog.exec();
}

void MainWindow::OnGoToLine() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const int maxLine = static_cast<int>(editor->send(SCI_GETLINECOUNT));
    bool ok = false;
    const int line = QInputDialog::getInt(
        this,
        tr("Go To Line"),
        tr("Line number:"),
        1,
        1,
        std::max(maxLine, 1),
        1,
        &ok);
    if (!ok) {
        return;
    }

    editor->send(SCI_GOTOLINE, static_cast<uptr_t>(line - 1));
    editor->send(SCI_SCROLLCARET);
    UpdateCursorStatus();
}

bool MainWindow::FormatEditorWithAvailableFormatter(
    ScintillaEditBase *editor,
    bool showUserMessages,
    bool *formattedApplied) {
    if (formattedApplied) {
        *formattedApplied = false;
    }
    if (!editor) {
        return false;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }

    const std::string inputText = GetEditorText(editor);
    const std::string lexerName = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string languageId = npp::ui::MapLexerToLspLanguageId(lexerName).empty()
        ? lexerName
        : npp::ui::MapLexerToLspLanguageId(lexerName);
    const std::string formatterProfile = ResolveFormatterProfileForLanguage(lexerName, languageId);

    std::string formatterName;
    std::string outputText;
    bool formatterSupported = false;
    bool extensionFormatterSucceeded = false;
    bool allowExtensionFormatter = true;
    bool allowBuiltInFormatter = true;
    if (_safeModeNoExtensions) {
        allowExtensionFormatter = false;
    }
    std::string requiredExtensionId;
    if (formatterProfile == "builtin") {
        allowExtensionFormatter = false;
    } else if (formatterProfile == "extension") {
        allowBuiltInFormatter = false;
    } else if (formatterProfile.rfind("extension:", 0) == 0) {
        allowBuiltInFormatter = false;
        requiredExtensionId = formatterProfile.substr(std::string("extension:").size());
    }

    if (allowExtensionFormatter) {
        const auto installedExtensions = _extensionService.DiscoverInstalled();
        if (installedExtensions.ok()) {
            for (const npp::platform::InstalledExtension& extension : *installedExtensions.value) {
                if (!requiredExtensionId.empty() && extension.manifest.id != requiredExtensionId) {
                    continue;
                }
                if (!extension.enabled) {
                    continue;
                }
                if (extension.manifest.type == npp::platform::ExtensionType::kLanguagePack) {
                    continue;
                }
                if (extension.manifest.entrypoint.empty()) {
                    continue;
                }
                if (extension.manifest.formatters.empty()) {
                    continue;
                }

                for (const auto& contribution : extension.manifest.formatters) {
                    if (!FormatterContributionMatchesLanguage(contribution, lexerName, languageId)) {
                        continue;
                    }

                    const auto permissionGranted = _extensionService.RequestPermission(
                        extension.manifest.id,
                        "process.spawn",
                        "format document");
                    if (!permissionGranted.ok() || !(*permissionGranted.value)) {
                        continue;
                    }

                    std::filesystem::path formatterEntrypoint(extension.manifest.entrypoint);
                    if (formatterEntrypoint.is_relative()) {
                        formatterEntrypoint = std::filesystem::path(extension.installPath) / formatterEntrypoint;
                    }

                    QProcess process(this);
                    process.setProgram(ToQString(formatterEntrypoint.string()));
                    process.setArguments(BuildFormatterArguments(
                        contribution.args,
                        stateIt->second.filePathUtf8,
                        languageId,
                        _editorSettings.tabWidth));
                    process.setWorkingDirectory(ToQString(extension.installPath));
                    process.start();
                    if (!process.waitForStarted(5000)) {
                        continue;
                    }

                    process.write(inputText.data(), static_cast<qint64>(inputText.size()));
                    process.closeWriteChannel();
                    if (!process.waitForFinished(60000)) {
                        process.kill();
                        process.waitForFinished(1000);
                        continue;
                    }
                    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
                        continue;
                    }

                    const QByteArray stdOut = process.readAllStandardOutput();
                    const std::string formattedUtf8(stdOut.constData(), static_cast<size_t>(stdOut.size()));
                    if (!IsValidUtf8(formattedUtf8)) {
                        continue;
                    }

                    formatterSupported = true;
                    extensionFormatterSucceeded = true;
                    formatterName = extension.manifest.name.empty() ? extension.manifest.id : extension.manifest.name;
                    outputText = NormalizeEol(formattedUtf8, stateIt->second.eolMode);
                    break;
                }

                if (extensionFormatterSucceeded) {
                    break;
                }
            }
        }
    }

    if (!extensionFormatterSucceeded && allowBuiltInFormatter) {
        const npp::ui::LanguageAwareFormatResult builtInResult = npp::ui::FormatDocumentLanguageAware(
            inputText,
            lexerName,
            _editorSettings.tabWidth);
        formatterSupported = builtInResult.supported;
        formatterName = builtInResult.formatterName;
        outputText = NormalizeEol(builtInResult.formattedTextUtf8, stateIt->second.eolMode);
    }

    if (!formatterSupported) {
        if (showUserMessages) {
            statusBar()->showMessage(
                tr("No formatter available for %1 (profile: %2)")
                    .arg(ToQString(lexerName))
                    .arg(FormatterProfileToLabel(formatterProfile)),
                2500);
        }
        return false;
    }

    if (outputText == inputText) {
        if (showUserMessages) {
            statusBar()->showMessage(
                tr("Document already formatted (%1)").arg(ToQString(formatterName)),
                2500);
        }
        return true;
    }

    SetEditorText(editor, outputText);
    SetEditorEolMode(editor, stateIt->second.eolMode);
    editor->send(SCI_COLOURISE, 0, -1);
    if (formattedApplied) {
        *formattedApplied = true;
    }
    if (showUserMessages) {
        statusBar()->showMessage(
            tr("Formatted document with %1").arg(ToQString(formatterName)),
            2500);
    }
    return true;
}

std::string MainWindow::ResolveFormatterProfileForLanguage(
    const std::string &lexerName,
    const std::string &languageId) const {
    const std::vector<std::string> candidates = FormatterLanguageCandidates(lexerName, languageId);
    for (const std::string &candidate : candidates) {
        const auto overrideIt = _editorSettings.formatterProfilesByLanguage.find(candidate);
        if (overrideIt != _editorSettings.formatterProfilesByLanguage.end()) {
            return NormalizeFormatterProfile(overrideIt->second);
        }
    }
    return NormalizeFormatterProfile(_editorSettings.formatterDefaultProfile);
}

bool MainWindow::ShouldFormatOnSaveForLexer(const std::string &lexerName) const {
    if (!_editorSettings.formatOnSaveEnabled) {
        return false;
    }
    if (_editorSettings.formatOnSaveLanguages.empty()) {
        return true;
    }

    const std::string languageId = npp::ui::MapLexerToLspLanguageId(lexerName).empty()
        ? lexerName
        : npp::ui::MapLexerToLspLanguageId(lexerName);
    const std::vector<std::string> candidates = FormatterLanguageCandidates(lexerName, languageId);
    for (const std::string &configuredLanguage : _editorSettings.formatOnSaveLanguages) {
        if (std::find(candidates.begin(), candidates.end(), configuredLanguage) != candidates.end()) {
            return true;
        }
    }
    return false;
}

void MainWindow::OnFormatDocument() {
    FormatEditorWithAvailableFormatter(CurrentEditor(), true, nullptr);
}

void MainWindow::OnPreferences() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Preferences"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *tabWidthSpin = new QSpinBox(&dialog);
    tabWidthSpin->setRange(1, 12);
    tabWidthSpin->setValue(_editorSettings.tabWidth);
    auto *wrapCheck = new QCheckBox(tr("Enable line wrap"), &dialog);
    wrapCheck->setChecked(_editorSettings.wrapEnabled);
    auto *lineNumbersCheck = new QCheckBox(tr("Show line numbers"), &dialog);
    lineNumbersCheck->setChecked(_editorSettings.showLineNumbers);
    auto *autoDetectLanguageCheck = new QCheckBox(tr("Auto-detect language"), &dialog);
    autoDetectLanguageCheck->setChecked(_editorSettings.autoDetectLanguage);
    auto *autoCloseHtmlTagsCheck = new QCheckBox(tr("Auto-close HTML/XML tags"), &dialog);
    autoCloseHtmlTagsCheck->setChecked(_editorSettings.autoCloseHtmlTags);
    auto *autoCloseDelimitersCheck = new QCheckBox(tr("Auto-close paired delimiters"), &dialog);
    autoCloseDelimitersCheck->setChecked(_editorSettings.autoCloseDelimiters);
    auto *updateChannelCombo = new QComboBox(&dialog);
    updateChannelCombo->addItem(tr("Stable"), QStringLiteral("stable"));
    updateChannelCombo->addItem(tr("Candidate"), QStringLiteral("candidate"));
    int updateChannelIndex = updateChannelCombo->findData(ToQString(NormalizeUpdateChannel(_editorSettings.updateChannel)));
    if (updateChannelIndex < 0) {
        updateChannelIndex = 0;
    }
    updateChannelCombo->setCurrentIndex(updateChannelIndex);
    auto *formatOnSaveCheck = new QCheckBox(tr("Format on save"), &dialog);
    formatOnSaveCheck->setChecked(_editorSettings.formatOnSaveEnabled);
    auto *formatOnSaveLanguagesEdit = new QLineEdit(&dialog);
    formatOnSaveLanguagesEdit->setPlaceholderText(tr("all languages"));
    formatOnSaveLanguagesEdit->setText(LanguageCsvListString(_editorSettings.formatOnSaveLanguages));
    formatOnSaveLanguagesEdit->setEnabled(_editorSettings.formatOnSaveEnabled);
    auto *formatterDefaultProfileCombo = new QComboBox(&dialog);
    formatterDefaultProfileCombo->addItem(tr("Auto (extensions then built-in)"), QStringLiteral("auto"));
    formatterDefaultProfileCombo->addItem(tr("Built-in only"), QStringLiteral("builtin"));
    formatterDefaultProfileCombo->addItem(tr("Extension only"), QStringLiteral("extension"));
    const QString currentProfile = ToQString(NormalizeFormatterProfile(_editorSettings.formatterDefaultProfile));
    int formatterProfileIndex = formatterDefaultProfileCombo->findData(currentProfile);
    if (formatterProfileIndex < 0) {
        formatterProfileIndex = 0;
    }
    formatterDefaultProfileCombo->setCurrentIndex(formatterProfileIndex);
    auto *formatterOverridesEdit = new QLineEdit(&dialog);
    formatterOverridesEdit->setPlaceholderText(tr("python=builtin, json=extension:ross.sample.formatter"));
    formatterOverridesEdit->setText(FormatterOverrideCsvString(_editorSettings.formatterProfilesByLanguage));
    auto *restoreSessionOnStartupCheck = new QCheckBox(tr("Restore previous session on startup"), &dialog);
    restoreSessionOnStartupCheck->setChecked(_editorSettings.restoreSessionOnStartup);
    auto *perProjectSessionStorageCheck = new QCheckBox(tr("Use per-project session storage"), &dialog);
    perProjectSessionStorageCheck->setChecked(_editorSettings.usePerProjectSessionStorage);
    perProjectSessionStorageCheck->setEnabled(_editorSettings.restoreSessionOnStartup);
    auto *autoSaveOnFocusLostCheck = new QCheckBox(tr("Auto-save on focus lost"), &dialog);
    autoSaveOnFocusLostCheck->setChecked(_editorSettings.autoSaveOnFocusLost);
    auto *autoSaveOnIntervalCheck = new QCheckBox(tr("Auto-save on interval"), &dialog);
    autoSaveOnIntervalCheck->setChecked(_editorSettings.autoSaveOnInterval);
    auto *autoSaveIntervalSpin = new QSpinBox(&dialog);
    autoSaveIntervalSpin->setRange(5, 600);
    autoSaveIntervalSpin->setValue(_editorSettings.autoSaveIntervalSeconds);
    autoSaveIntervalSpin->setEnabled(_editorSettings.autoSaveOnInterval);
    auto *autoSaveBeforeRunCheck = new QCheckBox(tr("Auto-save before Run Command"), &dialog);
    autoSaveBeforeRunCheck->setChecked(_editorSettings.autoSaveBeforeRun);

    form->addRow(tr("Tab width:"), tabWidthSpin);
    form->addRow(QString(), wrapCheck);
    form->addRow(QString(), lineNumbersCheck);
    form->addRow(QString(), autoDetectLanguageCheck);
    form->addRow(QString(), autoCloseHtmlTagsCheck);
    form->addRow(QString(), autoCloseDelimitersCheck);
    form->addRow(tr("Update channel:"), updateChannelCombo);
    form->addRow(QString(), formatOnSaveCheck);
    form->addRow(tr("Format-on-save languages:"), formatOnSaveLanguagesEdit);
    form->addRow(tr("Default formatter profile:"), formatterDefaultProfileCombo);
    form->addRow(tr("Formatter overrides:"), formatterOverridesEdit);
    form->addRow(QString(), restoreSessionOnStartupCheck);
    form->addRow(QString(), perProjectSessionStorageCheck);
    form->addRow(QString(), autoSaveOnFocusLostCheck);
    form->addRow(QString(), autoSaveOnIntervalCheck);
    form->addRow(tr("Auto-save interval (sec):"), autoSaveIntervalSpin);
    form->addRow(QString(), autoSaveBeforeRunCheck);
    layout->addLayout(form);

    connect(formatOnSaveCheck, &QCheckBox::toggled, formatOnSaveLanguagesEdit, &QLineEdit::setEnabled);
    connect(
        restoreSessionOnStartupCheck,
        &QCheckBox::toggled,
        perProjectSessionStorageCheck,
        &QCheckBox::setEnabled);
    connect(autoSaveOnIntervalCheck, &QCheckBox::toggled, autoSaveIntervalSpin, &QSpinBox::setEnabled);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    std::map<std::string, std::string> formatterOverrides;
    std::string overrideParseError;
    if (!ParseFormatterOverrideCsv(ToUtf8(formatterOverridesEdit->text()), &formatterOverrides, &overrideParseError)) {
        QMessageBox::warning(
            this,
            tr("Preferences"),
            tr("Formatter override parse error:\n%1").arg(ToQString(overrideParseError)));
        return;
    }

    _editorSettings.tabWidth = tabWidthSpin->value();
    _editorSettings.wrapEnabled = wrapCheck->isChecked();
    _editorSettings.showLineNumbers = lineNumbersCheck->isChecked();
    _editorSettings.autoDetectLanguage = autoDetectLanguageCheck->isChecked();
    _editorSettings.autoCloseHtmlTags = autoCloseHtmlTagsCheck->isChecked();
    _editorSettings.autoCloseDelimiters = autoCloseDelimitersCheck->isChecked();
    _editorSettings.updateChannel = NormalizeUpdateChannel(ToUtf8(updateChannelCombo->currentData().toString()));
    _editorSettings.formatOnSaveEnabled = formatOnSaveCheck->isChecked();
    _editorSettings.formatOnSaveLanguages = ParseLanguageCsvList(ToUtf8(formatOnSaveLanguagesEdit->text()));
    _editorSettings.formatterDefaultProfile =
        NormalizeFormatterProfile(ToUtf8(formatterDefaultProfileCombo->currentData().toString()));
    _editorSettings.formatterProfilesByLanguage = std::move(formatterOverrides);
    _editorSettings.restoreSessionOnStartup = restoreSessionOnStartupCheck->isChecked();
    _editorSettings.usePerProjectSessionStorage =
        _editorSettings.restoreSessionOnStartup &&
        perProjectSessionStorageCheck->isChecked();
    _editorSettings.autoSaveOnFocusLost = autoSaveOnFocusLostCheck->isChecked();
    _editorSettings.autoSaveOnInterval = autoSaveOnIntervalCheck->isChecked();
    _editorSettings.autoSaveBeforeRun = autoSaveBeforeRunCheck->isChecked();
    _editorSettings.autoSaveIntervalSeconds = autoSaveIntervalSpin->value();
    ApplyEditorSettingsToAllEditors();
    SaveEditorSettings();
    StartAutoSaveTimer();
    statusBar()->showMessage(tr("Preferences updated"), 1500);
}

void MainWindow::OnRunCommand() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Run Command"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *commandEdit = new QLineEdit(&dialog);
    commandEdit->setText(ToQString(_lastRunCommandUtf8));

    auto *workingDirEdit = new QLineEdit(&dialog);
    if (_lastRunWorkingDirUtf8.empty()) {
        ScintillaEditBase *editor = CurrentEditor();
        if (editor) {
            const auto stateIt = _editorStates.find(editor);
            if (stateIt != _editorStates.end() && !stateIt->second.filePathUtf8.empty()) {
                workingDirEdit->setText(QString::fromStdString(
                    std::filesystem::path(stateIt->second.filePathUtf8).parent_path().string()));
            } else {
                workingDirEdit->setText(QDir::homePath());
            }
        } else {
            workingDirEdit->setText(QDir::homePath());
        }
    } else {
        workingDirEdit->setText(ToQString(_lastRunWorkingDirUtf8));
    }

    auto *browseButton = new QPushButton(tr("Browse..."), &dialog);
    auto *workingDirLayout = new QHBoxLayout();
    workingDirLayout->addWidget(workingDirEdit);
    workingDirLayout->addWidget(browseButton);

    form->addRow(tr("Command:"), commandEdit);
    form->addRow(tr("Working dir:"), workingDirLayout);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(browseButton, &QPushButton::clicked, &dialog, [this, workingDirEdit]() {
        const QString selected = QFileDialog::getExistingDirectory(
            this,
            tr("Select Working Directory"),
            workingDirEdit->text());
        if (!selected.isEmpty()) {
            workingDirEdit->setText(selected);
        }
    });
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString command = commandEdit->text().trimmed();
    const QString workingDir = workingDirEdit->text().trimmed();
    if (command.isEmpty()) {
        return;
    }

    if (_editorSettings.autoSaveBeforeRun) {
        const int savedCount = AutoSaveDirtyEditors();
        if (savedCount > 0) {
            statusBar()->showMessage(
                tr("Auto-saved %1 file(s) before running command.").arg(savedCount),
                1800);
        }
    }

    const QStringList parts = QProcess::splitCommand(command);
    if (parts.isEmpty()) {
        QMessageBox::warning(this, tr("Run Command"), tr("Invalid command line."));
        return;
    }

    npp::platform::ProcessSpec spec;
    spec.program = ToUtf8(parts.at(0));
    for (int i = 1; i < parts.size(); ++i) {
        spec.args.push_back(ToUtf8(parts.at(i)));
    }
    if (!workingDir.isEmpty()) {
        spec.workingDirectoryUtf8 = ToUtf8(workingDir);
    }

    _lastRunCommandUtf8 = ToUtf8(command);
    _lastRunWorkingDirUtf8 = ToUtf8(workingDir);

    const auto result = _processService.Run(spec, std::chrono::milliseconds(60000));
    if (!result.ok()) {
        QMessageBox::warning(
            this,
            tr("Run Command"),
            tr("Command failed to run:\n%1").arg(ToQString(result.status.message)));
        return;
    }

    QMessageBox::information(
        this,
        tr("Run Command"),
        tr("Command finished with exit code %1.").arg(result.value->exitCode));
}

void MainWindow::OnCommandPalette() {
    if (_actionsById.empty()) {
        return;
    }

    struct PaletteEntry {
        std::string id;
        QString label;
        QString shortcut;
        bool enabled = true;
    };

    std::vector<PaletteEntry> entries;
    entries.reserve(_actionsById.size());
    for (const auto &[id, action] : _actionsById) {
        if (!action || id == "tools.commandPalette") {
            continue;
        }
        PaletteEntry entry;
        entry.id = id;
        entry.label = action->text();
        entry.label.remove('&');
        entry.shortcut = action->shortcut().toString(QKeySequence::NativeText);
        entry.enabled = action->isEnabled();
        entries.push_back(std::move(entry));
    }

    if (entries.empty()) {
        return;
    }

    std::sort(entries.begin(), entries.end(), [](const PaletteEntry &left, const PaletteEntry &right) {
        const int labelCompare = QString::compare(left.label, right.label, Qt::CaseInsensitive);
        if (labelCompare != 0) {
            return labelCompare < 0;
        }
        return left.id < right.id;
    });

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Command Palette"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *filterEdit = new QLineEdit(&dialog);
    filterEdit->setPlaceholderText(tr("Type to search commands"));
    filterEdit->setClearButtonEnabled(true);
    layout->addWidget(filterEdit);

    auto *commandList = new QListWidget(&dialog);
    commandList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(commandList);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    const auto refillCommands = [&]() {
        commandList->clear();
        const QString query = filterEdit->text().trimmed();
        for (const auto &entry : entries) {
            const QString searchable = (entry.label + QStringLiteral(" ") +
                                        QString::fromStdString(entry.id) + QStringLiteral(" ") +
                                        entry.shortcut)
                                           .toLower();
            if (!query.isEmpty() && !searchable.contains(query.toLower())) {
                continue;
            }

            const QString itemLabel = entry.shortcut.isEmpty()
                ? entry.label
                : tr("%1 (%2)").arg(entry.label, entry.shortcut);

            auto *item = new QListWidgetItem(itemLabel, commandList);
            item->setData(Qt::UserRole, ToQString(entry.id));
            if (!entry.enabled) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            }
        }

        if (commandList->count() > 0) {
            commandList->setCurrentRow(0);
        }
    };

    connect(filterEdit, &QLineEdit::textChanged, &dialog, refillCommands);
    connect(commandList, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    refillCommands();
    filterEdit->setFocus();

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QListWidgetItem *selectedItem = commandList->currentItem();
    if (!selectedItem) {
        return;
    }

    const std::string actionId = ToUtf8(selectedItem->data(Qt::UserRole).toString());
    const auto actionIt = _actionsById.find(actionId);
    if (actionIt == _actionsById.end() || actionIt->second == nullptr) {
        return;
    }
    if (!actionIt->second->isEnabled()) {
        QMessageBox::information(this, tr("Command Palette"), tr("Selected command is currently unavailable."));
        return;
    }

    actionIt->second->trigger();
}

void MainWindow::OnMultiCursorAddCaretAbove() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const int currentPos = static_cast<int>(editor->send(SCI_GETCURRENTPOS));
    const int currentLine = static_cast<int>(editor->send(SCI_LINEFROMPOSITION, currentPos));
    if (currentLine <= 0) {
        statusBar()->showMessage(tr("No line above current caret."), 1500);
        return;
    }

    const int targetLine = currentLine - 1;
    const int column = static_cast<int>(editor->send(SCI_GETCOLUMN, currentPos));
    const int targetPos = static_cast<int>(editor->send(SCI_FINDCOLUMN, targetLine, column));

    const int selectionCount = static_cast<int>(editor->send(SCI_GETSELECTIONS));
    for (int i = 0; i < selectionCount; ++i) {
        const int caret = static_cast<int>(editor->send(SCI_GETSELECTIONNCARET, i));
        const int anchor = static_cast<int>(editor->send(SCI_GETSELECTIONNANCHOR, i));
        if (caret == targetPos && anchor == targetPos) {
            return;
        }
    }

    editor->send(SCI_ADDSELECTION, targetPos, targetPos);
    statusBar()->showMessage(tr("Added caret above."), 1200);
}

void MainWindow::OnMultiCursorAddCaretBelow() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const int currentPos = static_cast<int>(editor->send(SCI_GETCURRENTPOS));
    const int currentLine = static_cast<int>(editor->send(SCI_LINEFROMPOSITION, currentPos));
    const int lineCount = static_cast<int>(editor->send(SCI_GETLINECOUNT));
    if (currentLine >= lineCount - 1) {
        statusBar()->showMessage(tr("No line below current caret."), 1500);
        return;
    }

    const int targetLine = currentLine + 1;
    const int column = static_cast<int>(editor->send(SCI_GETCOLUMN, currentPos));
    const int targetPos = static_cast<int>(editor->send(SCI_FINDCOLUMN, targetLine, column));

    const int selectionCount = static_cast<int>(editor->send(SCI_GETSELECTIONS));
    for (int i = 0; i < selectionCount; ++i) {
        const int caret = static_cast<int>(editor->send(SCI_GETSELECTIONNCARET, i));
        const int anchor = static_cast<int>(editor->send(SCI_GETSELECTIONNANCHOR, i));
        if (caret == targetPos && anchor == targetPos) {
            return;
        }
    }

    editor->send(SCI_ADDSELECTION, targetPos, targetPos);
    statusBar()->showMessage(tr("Added caret below."), 1200);
}

void MainWindow::OnMultiCursorAddNextMatch() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    if (GetSelectedText(editor).empty()) {
        QMessageBox::information(
            this,
            tr("Multi-Cursor"),
            tr("Select text first, then add the next match."));
        return;
    }

    editor->send(SCI_MULTIPLESELECTADDNEXT);
    statusBar()->showMessage(tr("Added next match."), 1200);
}

void MainWindow::OnMultiCursorSelectAllMatches() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    if (GetSelectedText(editor).empty()) {
        QMessageBox::information(
            this,
            tr("Multi-Cursor"),
            tr("Select text first, then select all matches."));
        return;
    }

    editor->send(SCI_MULTIPLESELECTADDEACH);
    statusBar()->showMessage(tr("Selected all matches."), 1200);
}

void MainWindow::OnOpenHelpDocs() {
    OpenExternalLink(
        RepositoryUrl(QStringLiteral("/blob/master/docs/help-and-support.md")),
        tr("Help docs"));
}

void MainWindow::OnOpenHelpWiki() {
    OpenExternalLink(
        RepositoryUrl(QStringLiteral("/wiki")),
        tr("Project wiki"));
}

void MainWindow::OnReportBug() {
    OpenExternalLink(
        RepositoryUrl(QStringLiteral("/issues/new?template=bug_report.yml")),
        tr("bug report form"));
}

void MainWindow::OnRequestFeature() {
    OpenExternalLink(
        RepositoryUrl(QStringLiteral("/issues/new?template=2-feature-request.yml")),
        tr("feature request form"));
}

void MainWindow::OnCheckForUpdates() {
    const std::string channel = NormalizeUpdateChannel(_editorSettings.updateChannel);
    const QString releaseUrl = channel == "candidate"
        ? RepositoryUrl(QStringLiteral("/releases"))
        : RepositoryUrl(QStringLiteral("/releases/latest"));
    OpenExternalLink(releaseUrl, tr("release updates"));
    statusBar()->showMessage(
        tr("Update channel: %1").arg(UpdateChannelLabel(channel)),
        2000);
}

void MainWindow::OnAboutDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("About Notepad++ Linux"));
    dialog.resize(480, 320);

    auto *layout = new QVBoxLayout(&dialog);

    auto *titleLabel = new QLabel(tr("<h3>Notepad++ Linux</h3>"), &dialog);
    titleLabel->setTextFormat(Qt::RichText);
    layout->addWidget(titleLabel);

    QString version = QCoreApplication::applicationVersion().trimmed();
    if (version.isEmpty()) {
        version = QStringLiteral("dev");
    }

    auto *detailsLabel = new QLabel(
        tr("Version: %1\nQt: %2\nPlatform: %3\nBuilt: %4 %5")
            .arg(version)
            .arg(QString::fromLatin1(qVersion()))
            .arg(QSysInfo::prettyProductName())
            .arg(QStringLiteral(__DATE__))
            .arg(QStringLiteral(__TIME__)),
        &dialog);
    detailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLabel->setWordWrap(true);
    layout->addWidget(detailsLabel);

    auto *linksLabel = new QLabel(
        tr("<p><a href=\"%1\">Help and Support</a><br/>"
           "<a href=\"%2\">Project Wiki</a><br/>"
           "<a href=\"%3\">Report Bug</a> | "
           "<a href=\"%4\">Request Feature</a></p>")
            .arg(RepositoryUrl(QStringLiteral("/blob/master/docs/help-and-support.md")))
            .arg(RepositoryUrl(QStringLiteral("/wiki")))
            .arg(RepositoryUrl(QStringLiteral("/issues/new?template=bug_report.yml")))
            .arg(RepositoryUrl(QStringLiteral("/issues/new?template=2-feature-request.yml"))),
        &dialog);
    linksLabel->setTextFormat(Qt::RichText);
    linksLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    linksLabel->setOpenExternalLinks(true);
    layout->addWidget(linksLabel);

    auto *licenseLabel = new QLabel(
        tr("License: GPL\nProject: https://github.com/RossEngineering/notepad-plus-plus-linux"),
        &dialog);
    licenseLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    licenseLabel->setWordWrap(true);
    layout->addWidget(licenseLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);

    dialog.exec();
}

void MainWindow::OpenExternalLink(const QString &url, const QString &label) {
    if (QDesktopServices::openUrl(QUrl(url))) {
        statusBar()->showMessage(tr("Opened %1").arg(label), 2000);
        return;
    }
    QMessageBox::warning(
        this,
        tr("Open Link"),
        tr("Unable to open link:\n%1").arg(url));
}

void MainWindow::InitializeLspServers() {
    for (const auto& config : DefaultLspServerConfigs()) {
        _lspService.RegisterServer(config);
    }
}

void MainWindow::StopLspSessionForEditor(ScintillaEditBase *editor) {
    if (!editor) {
        return;
    }
    const auto sessionIt = _lspSessionByEditor.find(editor);
    if (sessionIt == _lspSessionByEditor.end()) {
        return;
    }
    _lspService.StopSession(sessionIt->second);
    _lspSessionByEditor.erase(sessionIt);
}

void MainWindow::OnInstallExtensionFromDirectory() {
    if (_safeModeNoExtensions) {
        QMessageBox::information(
            this,
            tr("Install Extension"),
            tr("Extensions are disabled in safe mode. Relaunch in normal mode to install extensions."));
        return;
    }

    const QString sourceDir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Extension Folder"),
        QDir::homePath());
    if (sourceDir.isEmpty()) {
        return;
    }

    const npp::platform::Status installStatus =
        _extensionService.InstallFromDirectory(ToUtf8(sourceDir));
    if (!installStatus.ok()) {
        QMessageBox::warning(
            this,
            tr("Install Extension"),
            tr("Extension install failed:\n%1").arg(ToQString(installStatus.message)));
        return;
    }

    statusBar()->showMessage(tr("Extension installed successfully"), 2000);
}

void MainWindow::OnOpenExtensionMarketplace() {
    if (_safeModeNoExtensions) {
        QMessageBox::information(
            this,
            tr("Extension Marketplace"),
            tr("Extensions are disabled in safe mode. Relaunch in normal mode to use the marketplace."));
        return;
    }

    const std::string indexPath = ResolveLocalExtensionMarketplaceIndexPath();
    if (indexPath.empty()) {
        QMessageBox::information(
            this,
            tr("Extension Marketplace"),
            tr("No local marketplace index found.\n\nExpected one of:\n"
               "  - %1/extensions-marketplace-index.json\n"
               "  - ../share/notepad-plus-plus-linux/extensions/index.json\n"
               "  - /usr/local/share/notepad-plus-plus-linux/extensions/index.json\n"
               "  - /usr/share/notepad-plus-plus-linux/extensions/index.json")
                .arg(ToQString(ConfigRootPath())));
        return;
    }

    const auto indexJson = _fileSystemService.ReadTextFile(indexPath);
    if (!indexJson.ok()) {
        QMessageBox::warning(
            this,
            tr("Extension Marketplace"),
            tr("Unable to read marketplace index:\n%1").arg(ToQString(indexJson.status.message)));
        return;
    }

    QString parseError;
    const std::vector<ExtensionMarketplaceEntry> entries =
        ParseMarketplaceIndex(*indexJson.value, indexPath, &parseError);
    if (entries.empty()) {
        QMessageBox::information(
            this,
            tr("Extension Marketplace"),
            parseError.isEmpty()
                ? tr("Marketplace index is valid but contains no entries.")
                : parseError);
        return;
    }

    auto installed = _extensionService.DiscoverInstalled();
    if (!installed.ok()) {
        QMessageBox::warning(
            this,
            tr("Extension Marketplace"),
            tr("Unable to load installed extensions:\n%1").arg(ToQString(installed.status.message)));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Extension Marketplace (Local Index)"));
    dialog.resize(760, 520);
    auto *layout = new QVBoxLayout(&dialog);

    auto *sourceLabel = new QLabel(
        tr("Index: %1").arg(ToQString(indexPath)),
        &dialog);
    sourceLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    sourceLabel->setWordWrap(true);
    layout->addWidget(sourceLabel);

    auto *entryList = new QListWidget(&dialog);
    layout->addWidget(entryList, 1);

    auto *detailsLabel = new QLabel(&dialog);
    detailsLabel->setWordWrap(true);
    detailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(detailsLabel);

    auto *buttonRow = new QHBoxLayout();
    auto *installOrUpdateButton = new QPushButton(tr("Install/Update Selected"), &dialog);
    auto *updateAllButton = new QPushButton(tr("Update All Installed"), &dialog);
    auto *openIndexButton = new QPushButton(tr("Open Index File"), &dialog);
    auto *closeButton = new QPushButton(tr("Close"), &dialog);
    buttonRow->addWidget(installOrUpdateButton);
    buttonRow->addWidget(updateAllButton);
    buttonRow->addStretch(1);
    buttonRow->addWidget(openIndexButton);
    buttonRow->addWidget(closeButton);
    layout->addLayout(buttonRow);

    auto rebuildInstalledMap = [&installed]() {
        std::map<std::string, npp::platform::InstalledExtension> byId;
        if (installed.ok()) {
            for (const npp::platform::InstalledExtension &extension : *installed.value) {
                byId.insert_or_assign(extension.manifest.id, extension);
            }
        }
        return byId;
    };

    std::map<std::string, npp::platform::InstalledExtension> installedById = rebuildInstalledMap();

    auto renderList = [&]() {
        entryList->clear();
        for (const ExtensionMarketplaceEntry &entry : entries) {
            QString status = tr("available");
            auto installedIt = installedById.find(entry.id);
            if (installedIt != installedById.end()) {
                const std::string installedVersion = installedIt->second.manifest.version;
                const int versionCmp = CompareLooseVersion(entry.version, installedVersion);
                if (versionCmp > 0) {
                    status = tr("update available (%1 -> %2)")
                                 .arg(ToQString(installedVersion))
                                 .arg(ToQString(entry.version));
                } else {
                    status = tr("installed (%1)").arg(ToQString(installedVersion));
                }
            }

            auto *item = new QListWidgetItem(
                QStringLiteral("%1 [%2] - %3")
                    .arg(ToQString(entry.name))
                    .arg(ToQString(entry.id))
                    .arg(status));
            item->setData(Qt::UserRole, ToQString(entry.id));
            entryList->addItem(item);
        }
    };

    auto updateDetails = [&]() {
        const int row = entryList->currentRow();
        if (row < 0 || row >= static_cast<int>(entries.size())) {
            detailsLabel->setText(tr("Select an extension to view details."));
            return;
        }

        const ExtensionMarketplaceEntry &entry = entries[static_cast<size_t>(row)];
        QString installedText = tr("Not installed");
        auto installedIt = installedById.find(entry.id);
        if (installedIt != installedById.end()) {
            const std::string installedVersion = installedIt->second.manifest.version;
            const int versionCmp = CompareLooseVersion(entry.version, installedVersion);
            if (versionCmp > 0) {
                installedText = tr("Installed %1 (update to %2 available)")
                                    .arg(ToQString(installedVersion))
                                    .arg(ToQString(entry.version));
            } else {
                installedText = tr("Installed %1 (up to date)")
                                    .arg(ToQString(installedVersion));
            }
        }

        detailsLabel->setText(
            tr("Name: %1\nID: %2\nMarketplace version: %3\nInstall state: %4\nSource: %5\nDescription: %6")
                .arg(ToQString(entry.name))
                .arg(ToQString(entry.id))
                .arg(ToQString(entry.version))
                .arg(installedText)
                .arg(ToQString(entry.sourceDirectoryUtf8))
                .arg(ToQString(entry.description.empty() ? std::string("n/a") : entry.description)));
    };

    auto installFromEntry = [&](const ExtensionMarketplaceEntry &entry) -> bool {
        const npp::platform::Status installStatus =
            _extensionService.InstallFromDirectory(entry.sourceDirectoryUtf8);
        if (!installStatus.ok()) {
            QMessageBox::warning(
                &dialog,
                tr("Extension Marketplace"),
                tr("Install/update failed for %1:\n%2")
                    .arg(ToQString(entry.id))
                    .arg(ToQString(installStatus.message)));
            return false;
        }
        return true;
    };

    renderList();
    if (entryList->count() > 0) {
        entryList->setCurrentRow(0);
    } else {
        detailsLabel->setText(tr("No entries."));
    }
    updateDetails();

    connect(entryList, &QListWidget::currentRowChanged, &dialog, [&](int) {
        updateDetails();
    });
    connect(installOrUpdateButton, &QPushButton::clicked, &dialog, [&]() {
        const int row = entryList->currentRow();
        if (row < 0 || row >= static_cast<int>(entries.size())) {
            return;
        }
        const ExtensionMarketplaceEntry &entry = entries[static_cast<size_t>(row)];
        if (!installFromEntry(entry)) {
            return;
        }
        installed = _extensionService.DiscoverInstalled();
        if (installed.ok()) {
            installedById = rebuildInstalledMap();
        }
        renderList();
        entryList->setCurrentRow(row);
        updateDetails();
        statusBar()->showMessage(
            tr("Installed/updated extension: %1").arg(ToQString(entry.id)),
            2500);
    });
    connect(updateAllButton, &QPushButton::clicked, &dialog, [&]() {
        std::vector<const ExtensionMarketplaceEntry *> toUpdate;
        for (const ExtensionMarketplaceEntry &entry : entries) {
            const auto installedIt = installedById.find(entry.id);
            if (installedIt == installedById.end()) {
                continue;
            }
            if (CompareLooseVersion(entry.version, installedIt->second.manifest.version) > 0) {
                toUpdate.push_back(&entry);
            }
        }
        if (toUpdate.empty()) {
            QMessageBox::information(
                &dialog,
                tr("Extension Marketplace"),
                tr("No installed extensions have available updates."));
            return;
        }

        int updatedCount = 0;
        for (const ExtensionMarketplaceEntry *entry : toUpdate) {
            if (entry && installFromEntry(*entry)) {
                ++updatedCount;
            }
        }
        installed = _extensionService.DiscoverInstalled();
        if (installed.ok()) {
            installedById = rebuildInstalledMap();
        }
        renderList();
        updateDetails();
        statusBar()->showMessage(
            tr("Updated %1 extension(s) from local marketplace").arg(updatedCount),
            3000);
    });
    connect(openIndexButton, &QPushButton::clicked, &dialog, [&]() {
        const npp::platform::Status openStatus = _processService.OpenPath(indexPath);
        if (!openStatus.ok()) {
            QMessageBox::warning(
                &dialog,
                tr("Extension Marketplace"),
                tr("Unable to open index file:\n%1").arg(ToQString(openStatus.message)));
        }
    });
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::OnRestartSafeMode() {
    QStringList forwardedArguments = QCoreApplication::arguments();
    if (!forwardedArguments.isEmpty()) {
        forwardedArguments.removeFirst();
    }

    forwardedArguments.erase(
        std::remove_if(forwardedArguments.begin(), forwardedArguments.end(), [](const QString &argument) {
            return argument == QStringLiteral("--safe-mode") ||
                argument == QStringLiteral("--safe-mode-no-extensions");
        }),
        forwardedArguments.end());
    if (!_safeModeNoExtensions) {
        forwardedArguments.push_back(QStringLiteral("--safe-mode"));
    }

    const bool started = QProcess::startDetached(
        QCoreApplication::applicationFilePath(),
        forwardedArguments);
    if (!started) {
        QMessageBox::warning(
            this,
            tr("Safe Mode"),
            tr("Unable to relaunch the application."));
        return;
    }
    close();
}

void MainWindow::OnManageExtensions() {
    if (_safeModeNoExtensions) {
        QMessageBox::information(
            this,
            tr("Manage Extensions"),
            tr("Extensions are disabled in safe mode. Relaunch in normal mode to manage extensions."));
        return;
    }

    const auto installed = _extensionService.DiscoverInstalled();
    if (!installed.ok()) {
        QMessageBox::warning(
            this,
            tr("Manage Extensions"),
            tr("Unable to load installed extensions:\n%1").arg(ToQString(installed.status.message)));
        return;
    }
    if (installed.value->empty()) {
        QMessageBox::information(this, tr("Manage Extensions"), tr("No installed extensions found."));
        return;
    }

    QStringList extensionChoices;
    for (const auto& extension : *installed.value) {
        extensionChoices << QStringLiteral("%1 (%2)")
                                .arg(ToQString(extension.manifest.id))
                                .arg(extension.enabled ? tr("enabled") : tr("disabled"));
    }

    bool extensionChoiceOk = false;
    const QString chosen = QInputDialog::getItem(
        this,
        tr("Manage Extensions"),
        tr("Extension:"),
        extensionChoices,
        0,
        false,
        &extensionChoiceOk);
    if (!extensionChoiceOk || chosen.isEmpty()) {
        return;
    }

    const QString extensionId = chosen.left(chosen.indexOf(QStringLiteral(" (")));
    const std::string extensionIdUtf8 = ToUtf8(extensionId);
    auto selectedExtensionIt = std::find_if(
        installed.value->begin(),
        installed.value->end(),
        [&extensionIdUtf8](const npp::platform::InstalledExtension &extension) {
            return extension.manifest.id == extensionIdUtf8;
        });
    if (selectedExtensionIt == installed.value->end()) {
        QMessageBox::warning(
            this,
            tr("Manage Extensions"),
            tr("Selected extension is no longer installed."));
        return;
    }

    const npp::platform::InstalledExtension &selectedExtension = *selectedExtensionIt;
    const std::optional<ExtensionDirectoryFootprint> footprint =
        ComputeDirectoryFootprint(selectedExtension.installPath);
    const double startupEstimateMs = _extensionStartupEstimateMsById.count(selectedExtension.manifest.id) > 0
        ? _extensionStartupEstimateMsById.at(selectedExtension.manifest.id)
        : 0.0;
    const double manifestScanMs = _extensionManifestScanMsById.count(selectedExtension.manifest.id) > 0
        ? _extensionManifestScanMsById.at(selectedExtension.manifest.id)
        : 0.0;

    bool updateAvailable = false;
    ExtensionMarketplaceEntry marketplaceEntry;
    const std::string marketplaceIndexPath = ResolveLocalExtensionMarketplaceIndexPath();
    if (!marketplaceIndexPath.empty()) {
        const auto indexJson = _fileSystemService.ReadTextFile(marketplaceIndexPath);
        if (indexJson.ok()) {
            QString ignoredError;
            const std::vector<ExtensionMarketplaceEntry> entries =
                ParseMarketplaceIndex(*indexJson.value, marketplaceIndexPath, &ignoredError);
            const auto entryIt = std::find_if(entries.begin(), entries.end(), [&selectedExtension](const ExtensionMarketplaceEntry &entry) {
                return entry.id == selectedExtension.manifest.id;
            });
            if (entryIt != entries.end()) {
                marketplaceEntry = *entryIt;
                updateAvailable = CompareLooseVersion(
                    marketplaceEntry.version,
                    selectedExtension.manifest.version) > 0;
            }
        }
    }

    QString extensionDetails = tr(
        "Extension: %1\nID: %2\nVersion: %3\nInstall path: %4\nEnabled: %5\n"
        "Startup impact estimate: %6 ms\nManifest scan: %7 ms\n"
        "Resource usage: %8 in %9 file(s)")
            .arg(ToQString(selectedExtension.manifest.name.empty()
                               ? selectedExtension.manifest.id
                               : selectedExtension.manifest.name))
            .arg(ToQString(selectedExtension.manifest.id))
            .arg(ToQString(selectedExtension.manifest.version))
            .arg(ToQString(selectedExtension.installPath))
            .arg(selectedExtension.enabled ? tr("yes") : tr("no"))
            .arg(QString::number(startupEstimateMs, 'f', 1))
            .arg(QString::number(manifestScanMs, 'f', 1))
            .arg(footprint.has_value() ? FormatBytesBinary(footprint->totalBytes) : tr("unknown"))
            .arg(footprint.has_value() ? static_cast<int>(footprint->fileCount) : 0);
    if (updateAvailable) {
        extensionDetails.append(
            tr("\nUpdate available in local marketplace: %1")
                .arg(ToQString(marketplaceEntry.version)));
    } else {
        extensionDetails.append(tr("\nMarketplace update: none"));
    }

    QMessageBox::information(this, tr("Extension Details"), extensionDetails);

    const QString updateOperationLabel = tr("Update from Marketplace");
    QStringList operations = {tr("Enable"), tr("Disable"), tr("Reset Permissions"), tr("Remove")};
    if (updateAvailable) {
        operations.push_back(updateOperationLabel);
    }
    bool operationOk = false;
    const QString operation = QInputDialog::getItem(
        this,
        tr("Manage Extensions"),
        tr("Operation:"),
        operations,
        0,
        false,
        &operationOk);
    if (!operationOk || operation.isEmpty()) {
        return;
    }

    npp::platform::Status operationStatus = npp::platform::Status::Ok();
    if (operation == operations.at(0)) {
        operationStatus = _extensionService.EnableExtension(extensionIdUtf8);
    } else if (operation == operations.at(1)) {
        operationStatus = _extensionService.DisableExtension(extensionIdUtf8);
    } else if (operation == operations.at(2)) {
        operationStatus = _extensionService.ResetPermissions(extensionIdUtf8);
    } else if (operation == operations.at(3)) {
        operationStatus = _extensionService.RemoveExtension(extensionIdUtf8);
    } else {
        operationStatus = _extensionService.InstallFromDirectory(marketplaceEntry.sourceDirectoryUtf8);
    }

    if (!operationStatus.ok()) {
        QMessageBox::warning(
            this,
            tr("Manage Extensions"),
            tr("Operation failed:\n%1").arg(ToQString(operationStatus.message)));
        return;
    }
    if (operation == operations.at(2)) {
        statusBar()->showMessage(tr("Extension permission decisions reset"), 2000);
    } else if (operation == updateOperationLabel) {
        statusBar()->showMessage(tr("Extension updated from local marketplace"), 2000);
    } else {
        statusBar()->showMessage(tr("Extension operation completed"), 2000);
    }
}

void MainWindow::OnAutoDetectLanguage() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = false;
    AutoDetectAndApplyLexer(editor, stateIt->second.filePathUtf8, GetEditorText(editor), "manual-auto-detect", true);
    UpdateLanguageActionState();
}

void MainWindow::OnToggleLexerLock() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = !stateIt->second.lexerManualLock;
    if (!stateIt->second.lexerManualLock) {
        AutoDetectAndApplyLexer(editor, stateIt->second.filePathUtf8, GetEditorText(editor), "unlock-auto-detect");
    } else {
        stateIt->second.lexerReason = "manual-lock";
        stateIt->second.lexerConfidence = 1.0;
        statusBar()->showMessage(
            tr("Language lock enabled (%1)").arg(ToQString(stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName)),
            2000);
    }
    UpdateLanguageActionState();
}

void MainWindow::OnLspStartSession() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }

    const std::string lexerName = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string languageId = npp::ui::MapLexerToLspLanguageId(lexerName);
    if (languageId.empty()) {
        QMessageBox::information(
            this,
            tr("Language Server"),
            tr("No LSP baseline mapping is available for lexer: %1").arg(ToQString(lexerName)));
        return;
    }

    const auto activeSessionIt = _lspSessionByEditor.find(editor);
    if (activeSessionIt != _lspSessionByEditor.end() &&
        _lspService.IsSessionActive(activeSessionIt->second)) {
        statusBar()->showMessage(tr("Language server is already active"), 2000);
        return;
    }

    npp::platform::LspSessionSpec spec;
    spec.languageId = languageId;
    spec.documentPathUtf8 = stateIt->second.filePathUtf8;
    if (!stateIt->second.filePathUtf8.empty()) {
        spec.workspacePathUtf8 = std::filesystem::path(stateIt->second.filePathUtf8).parent_path().string();
    } else {
        spec.workspacePathUtf8 = QDir::currentPath().toStdString();
    }

    auto sessionOr = _lspService.StartSession(spec);
    if (!sessionOr.ok()) {
        QMessageBox::warning(
            this,
            tr("Language Server"),
            tr("Failed to start language server for %1:\n%2")
                .arg(ToQString(languageId))
                .arg(ToQString(sessionOr.status.message)));
        return;
    }

    _lspSessionByEditor.insert_or_assign(editor, *sessionOr.value);
    statusBar()->showMessage(
        tr("Language server started for %1").arg(ToQString(languageId)),
        2500);
}

void MainWindow::OnLspStopSession() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto sessionIt = _lspSessionByEditor.find(editor);
    if (sessionIt == _lspSessionByEditor.end()) {
        statusBar()->showMessage(tr("No active language server session"), 2000);
        return;
    }

    const npp::platform::Status stopStatus = _lspService.StopSession(sessionIt->second);
    _lspSessionByEditor.erase(sessionIt);
    if (!stopStatus.ok()) {
        QMessageBox::warning(
            this,
            tr("Language Server"),
            tr("Failed to stop language server:\n%1").arg(ToQString(stopStatus.message)));
        return;
    }

    statusBar()->showMessage(tr("Language server stopped"), 2000);
}

void MainWindow::OnLspShowHover() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const std::string textUtf8 = GetEditorText(editor);
    const std::size_t caretPos = static_cast<std::size_t>(editor->send(SCI_GETCURRENTPOS));
    const std::string symbol = npp::ui::ExtractWordAt(textUtf8, caretPos);
    if (symbol.empty()) {
        QMessageBox::information(this, tr("Hover"), tr("No symbol under cursor."));
        return;
    }

    const auto sessionIt = _lspSessionByEditor.find(editor);
    const bool sessionActive = sessionIt != _lspSessionByEditor.end() &&
        _lspService.IsSessionActive(sessionIt->second);

    QMessageBox::information(
        this,
        tr("Hover (Baseline)"),
        tr("Symbol: %1\nLSP session active: %2\n\nBaseline hover is available now; "
           "full protocol hover details are planned in later RC work.")
            .arg(ToQString(symbol))
            .arg(sessionActive ? tr("yes") : tr("no")));
}

void MainWindow::OnLspGoToDefinition() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const std::string textUtf8 = GetEditorText(editor);
    const std::size_t caretPos = static_cast<std::size_t>(editor->send(SCI_GETCURRENTPOS));
    const std::string symbol = npp::ui::ExtractWordAt(textUtf8, caretPos);
    if (symbol.empty()) {
        QMessageBox::information(this, tr("Go To Definition"), tr("No symbol under cursor."));
        return;
    }

    const int line = npp::ui::FindBaselineDefinitionLine(textUtf8, symbol);
    if (line < 0) {
        QMessageBox::information(
            this,
            tr("Go To Definition"),
            tr("Baseline definition lookup did not find '%1' in this file.").arg(ToQString(symbol)));
        return;
    }

    editor->send(SCI_GOTOLINE, static_cast<uptr_t>(line));
    editor->send(SCI_SCROLLCARET);
    UpdateCursorStatus();
    statusBar()->showMessage(
        tr("Moved to baseline definition for %1").arg(ToQString(symbol)),
        2000);
}

void MainWindow::OnLspShowDiagnostics() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto diagnostics = npp::ui::CollectBaselineDiagnostics(GetEditorText(editor));
    if (diagnostics.empty()) {
        QMessageBox::information(this, tr("Diagnostics"), tr("No baseline diagnostics."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Diagnostics Panel (Baseline)"));
    dialog.resize(860, 520);

    auto *layout = new QVBoxLayout(&dialog);
    auto *headerLayout = new QHBoxLayout();
    auto *filterEdit = new QLineEdit(&dialog);
    filterEdit->setPlaceholderText(tr("Filter diagnostics"));
    filterEdit->setClearButtonEnabled(true);
    auto *severityCombo = new QComboBox(&dialog);
    severityCombo->addItem(tr("All severities"), QStringLiteral("all"));
    severityCombo->addItem(tr("Warning"), QStringLiteral("warning"));
    severityCombo->addItem(tr("Info"), QStringLiteral("info"));
    headerLayout->addWidget(filterEdit, 1);
    headerLayout->addWidget(severityCombo);
    layout->addLayout(headerLayout);

    auto *diagnosticList = new QListWidget(&dialog);
    diagnosticList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(diagnosticList, 1);

    auto *summaryLabel = new QLabel(&dialog);
    layout->addWidget(summaryLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QPushButton *goToButton = buttons->addButton(tr("Go To"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    const auto goToDiagnosticLine = [this, editor](int line) {
        if (line <= 0) {
            return;
        }
        editor->send(SCI_GOTOLINE, static_cast<uptr_t>(line - 1));
        editor->send(SCI_SCROLLCARET);
        UpdateCursorStatus();
    };

    const auto refillDiagnostics = [&]() {
        diagnosticList->clear();
        const QString query = filterEdit->text().trimmed().toLower();
        const QString severityFilter = severityCombo->currentData().toString();
        int visibleCount = 0;
        for (const auto& diagnostic : diagnostics) {
            const QString severity = ToQString(diagnostic.severity).toLower();
            if (severityFilter != QStringLiteral("all") && severity != severityFilter) {
                continue;
            }

            const QString searchable = QStringLiteral("%1 %2 %3")
                .arg(ToQString(diagnostic.code), ToQString(diagnostic.message), ToQString(diagnostic.severity))
                .toLower();
            if (!query.isEmpty() && !searchable.contains(query)) {
                continue;
            }

            ++visibleCount;
            auto *item = new QListWidgetItem(
                tr("[%1] line %2: %3")
                    .arg(ToQString(diagnostic.severity))
                    .arg(diagnostic.line)
                    .arg(ToQString(diagnostic.message)),
                diagnosticList);
            item->setData(Qt::UserRole, diagnostic.line);
            item->setData(Qt::UserRole + 1, ToQString(diagnostic.code));
        }

        summaryLabel->setText(
            tr("%1 of %2 diagnostics").arg(visibleCount).arg(static_cast<int>(diagnostics.size())));
        if (diagnosticList->count() > 0) {
            diagnosticList->setCurrentRow(0);
            goToButton->setEnabled(true);
        } else {
            goToButton->setEnabled(false);
        }
    };

    connect(filterEdit, &QLineEdit::textChanged, &dialog, refillDiagnostics);
    connect(severityCombo, &QComboBox::currentTextChanged, &dialog, [&](const QString &) {
        refillDiagnostics();
    });
    connect(diagnosticList, &QListWidget::itemDoubleClicked, &dialog, [&](QListWidgetItem *item) {
        goToDiagnosticLine(item == nullptr ? 0 : item->data(Qt::UserRole).toInt());
    });
    connect(goToButton, &QPushButton::clicked, &dialog, [&]() {
        QListWidgetItem *selectedItem = diagnosticList->currentItem();
        goToDiagnosticLine(selectedItem == nullptr ? 0 : selectedItem->data(Qt::UserRole).toInt());
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    refillDiagnostics();
    filterEdit->setFocus();
    dialog.exec();
}

void MainWindow::OnLspShowDocumentSymbols() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto symbols = npp::ui::CollectBaselineDocumentSymbols(GetEditorText(editor));
    if (symbols.empty()) {
        QMessageBox::information(this, tr("Document Symbols"), tr("No baseline symbols found in this document."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Document Symbols (Baseline)"));
    dialog.resize(760, 500);

    auto *layout = new QVBoxLayout(&dialog);
    auto *filterEdit = new QLineEdit(&dialog);
    filterEdit->setPlaceholderText(tr("Filter symbols by name or kind"));
    filterEdit->setClearButtonEnabled(true);
    layout->addWidget(filterEdit);

    auto *symbolList = new QListWidget(&dialog);
    symbolList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(symbolList, 1);

    auto *summaryLabel = new QLabel(&dialog);
    layout->addWidget(summaryLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QPushButton *goToButton = buttons->addButton(tr("Go To"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    const auto goToSymbolLine = [this, editor](int line) {
        if (line <= 0) {
            return;
        }
        editor->send(SCI_GOTOLINE, static_cast<uptr_t>(line - 1));
        editor->send(SCI_SCROLLCARET);
        UpdateCursorStatus();
    };

    const auto refillSymbols = [&]() {
        symbolList->clear();
        const QString query = filterEdit->text().trimmed().toLower();
        int visibleCount = 0;
        for (const auto& symbol : symbols) {
            const QString searchable = QStringLiteral("%1 %2")
                .arg(ToQString(symbol.name), ToQString(symbol.kind))
                .toLower();
            if (!query.isEmpty() && !searchable.contains(query)) {
                continue;
            }

            ++visibleCount;
            auto *item = new QListWidgetItem(
                tr("%1  [%2]  line %3")
                    .arg(ToQString(symbol.name), ToQString(symbol.kind))
                    .arg(symbol.line),
                symbolList);
            item->setData(Qt::UserRole, symbol.line);
        }

        summaryLabel->setText(
            tr("%1 of %2 symbols").arg(visibleCount).arg(static_cast<int>(symbols.size())));
        if (symbolList->count() > 0) {
            symbolList->setCurrentRow(0);
            goToButton->setEnabled(true);
        } else {
            goToButton->setEnabled(false);
        }
    };

    connect(filterEdit, &QLineEdit::textChanged, &dialog, refillSymbols);
    connect(symbolList, &QListWidget::itemDoubleClicked, &dialog, [&](QListWidgetItem *item) {
        goToSymbolLine(item == nullptr ? 0 : item->data(Qt::UserRole).toInt());
    });
    connect(goToButton, &QPushButton::clicked, &dialog, [&]() {
        QListWidgetItem *selectedItem = symbolList->currentItem();
        goToSymbolLine(selectedItem == nullptr ? 0 : selectedItem->data(Qt::UserRole).toInt());
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    refillSymbols();
    filterEdit->setFocus();
    dialog.exec();
}

void MainWindow::OnLspRenameSymbol() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }

    const std::string lexerName = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    if (npp::ui::MapLexerToLspLanguageId(lexerName).empty()) {
        QMessageBox::information(
            this,
            tr("Rename Symbol"),
            tr("Rename baseline is not available for lexer: %1").arg(ToQString(lexerName)));
        return;
    }

    const std::size_t caretPos = static_cast<std::size_t>(editor->send(SCI_GETCURRENTPOS));
    const std::string textUtf8 = GetEditorText(editor);
    const std::string symbol = npp::ui::ExtractWordAt(textUtf8, caretPos);
    if (symbol.empty()) {
        QMessageBox::information(this, tr("Rename Symbol"), tr("No symbol under cursor."));
        return;
    }

    bool renameAccepted = false;
    const QString newName = QInputDialog::getText(
        this,
        tr("Rename Symbol (Baseline)"),
        tr("New name for '%1':").arg(ToQString(symbol)),
        QLineEdit::Normal,
        ToQString(symbol),
        &renameAccepted);
    if (!renameAccepted) {
        return;
    }

    const std::string replacement = ToUtf8(newName);
    if (replacement.empty() || replacement == symbol) {
        return;
    }
    if (!IsAsciiIdentifierToken(replacement)) {
        QMessageBox::warning(
            this,
            tr("Rename Symbol"),
            tr("Invalid symbol name. Use letters, numbers, hyphen, and underscore only."));
        return;
    }

    std::string updatedText = textUtf8;
    const int replacements = npp::ui::RenameBaselineSymbol(&updatedText, symbol, replacement);
    if (replacements <= 0 || updatedText == textUtf8) {
        QMessageBox::information(this, tr("Rename Symbol"), tr("No replaceable symbol occurrences found."));
        return;
    }

    SetEditorText(editor, updatedText);
    editor->send(SCI_COLOURISE, 0, -1);
    const sptr_t currentLength = editor->send(SCI_GETTEXTLENGTH);
    editor->send(SCI_GOTOPOS, std::min<sptr_t>(static_cast<sptr_t>(caretPos), currentLength));

    auto mutableStateIt = _editorStates.find(editor);
    if (mutableStateIt != _editorStates.end()) {
        mutableStateIt->second.dirty = true;
    }
    UpdateTabTitle(editor);
    statusBar()->showMessage(
        tr("Renamed %1 occurrences of %2 to %3")
            .arg(replacements)
            .arg(ToQString(symbol))
            .arg(ToQString(replacement)),
        2500);
}

void MainWindow::OnLspCodeActions() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const auto diagnostics = npp::ui::CollectBaselineDiagnostics(GetEditorText(editor));
    const auto actions = npp::ui::CollectBaselineCodeActions(diagnostics);
    if (actions.empty()) {
        QMessageBox::information(this, tr("Code Actions"), tr("No baseline quick fixes available."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Code Actions (Baseline)"));
    dialog.resize(760, 460);

    auto *layout = new QVBoxLayout(&dialog);
    auto *filterEdit = new QLineEdit(&dialog);
    filterEdit->setPlaceholderText(tr("Filter code actions"));
    filterEdit->setClearButtonEnabled(true);
    layout->addWidget(filterEdit);

    auto *actionList = new QListWidget(&dialog);
    actionList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(actionList, 1);

    auto *summaryLabel = new QLabel(&dialog);
    layout->addWidget(summaryLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    QPushButton *applyButton = buttons->addButton(tr("Apply"), QDialogButtonBox::AcceptRole);
    layout->addWidget(buttons);

    const auto refillActions = [&]() {
        actionList->clear();
        const QString query = filterEdit->text().trimmed().toLower();
        int visibleCount = 0;
        for (const auto& action : actions) {
            const QString title = ToQString(action.title);
            if (!query.isEmpty() && !title.toLower().contains(query)) {
                continue;
            }
            ++visibleCount;
            auto *item = new QListWidgetItem(title, actionList);
            item->setData(Qt::UserRole, ToQString(action.id));
            item->setData(Qt::UserRole + 1, action.line);
        }

        summaryLabel->setText(
            tr("%1 of %2 code actions").arg(visibleCount).arg(static_cast<int>(actions.size())));
        if (actionList->count() > 0) {
            actionList->setCurrentRow(0);
            applyButton->setEnabled(true);
        } else {
            applyButton->setEnabled(false);
        }
    };

    connect(filterEdit, &QLineEdit::textChanged, &dialog, refillActions);
    connect(actionList, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    refillActions();
    filterEdit->setFocus();
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QListWidgetItem *selectedAction = actionList->currentItem();
    if (!selectedAction) {
        return;
    }

    std::string updatedText = GetEditorText(editor);
    const std::string actionId = ToUtf8(selectedAction->data(Qt::UserRole).toString());
    const int line = selectedAction->data(Qt::UserRole + 1).toInt();
    if (!npp::ui::ApplyBaselineCodeAction(&updatedText, actionId, line, _editorSettings.tabWidth)) {
        QMessageBox::information(this, tr("Code Actions"), tr("Selected action produced no changes."));
        return;
    }

    SetEditorText(editor, updatedText);
    editor->send(SCI_COLOURISE, 0, -1);
    auto stateIt = _editorStates.find(editor);
    if (stateIt != _editorStates.end()) {
        stateIt->second.dirty = true;
    }
    UpdateTabTitle(editor);
    statusBar()->showMessage(tr("Applied baseline code action"), 2000);
}

void MainWindow::SetCurrentEditorManualLexer(const std::string &lexerName) {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }
    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    stateIt->second.lexerManualLock = true;
    stateIt->second.lexerName = lexerName;
    stateIt->second.lexerConfidence = 1.0;
    stateIt->second.lexerReason = "manual-selection";
    ApplyLexerByName(editor, lexerName);
    statusBar()->showMessage(
        tr("Language set to %1 (manual lock)").arg(ToQString(lexerName)),
        2000);
    UpdateLanguageActionState();
}

void MainWindow::UpdateLanguageActionState() {
    auto lockIt = _actionsById.find("language.lockCurrent");
    if (lockIt == _actionsById.end() || !lockIt->second) {
        return;
    }

    const std::vector<std::string> languageActionIds = {
        "language.set.plain",
        "language.set.markdown",
        "language.set.html",
        "language.set.cpp",
        "language.set.python",
        "language.set.bash",
        "language.set.yaml",
        "language.set.sql",
    };
    const std::vector<std::string> lspActionIds = {
        "language.lsp.start",
        "language.lsp.stop",
        "language.lsp.hover",
        "language.lsp.gotoDefinition",
        "language.lsp.diagnostics",
        "language.lsp.symbols",
        "language.lsp.rename",
        "language.lsp.codeActions",
    };

    const auto setLanguageActionsEnabled = [this, &languageActionIds](bool enabled) {
        for (const std::string &id : languageActionIds) {
            const auto it = _actionsById.find(id);
            if (it != _actionsById.end() && it->second) {
                it->second->setEnabled(enabled);
            }
        }
    };
    const auto setLspActionsEnabled = [this, &lspActionIds](bool enabled) {
        for (const std::string &id : lspActionIds) {
            const auto it = _actionsById.find(id);
            if (it != _actionsById.end() && it->second) {
                it->second->setEnabled(enabled);
            }
        }
    };

    const auto clearLanguageChecks = [this, &languageActionIds]() {
        for (const std::string &id : languageActionIds) {
            const auto it = _actionsById.find(id);
            if (it != _actionsById.end() && it->second) {
                QSignalBlocker blocker(it->second);
                it->second->setChecked(false);
            }
        }
    };

    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        lockIt->second->setChecked(false);
        lockIt->second->setEnabled(false);
        clearLanguageChecks();
        setLanguageActionsEnabled(false);
        setLspActionsEnabled(false);
        return;
    }
    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        lockIt->second->setChecked(false);
        lockIt->second->setEnabled(false);
        clearLanguageChecks();
        setLanguageActionsEnabled(false);
        setLspActionsEnabled(false);
        return;
    }

    lockIt->second->setEnabled(true);
    lockIt->second->setChecked(stateIt->second.lexerManualLock);
    setLanguageActionsEnabled(true);
    clearLanguageChecks();

    const std::string activeLexer = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    setLspActionsEnabled(!npp::ui::MapLexerToLspLanguageId(activeLexer).empty());
    const std::string activeActionId = LanguageActionIdForLexer(activeLexer);
    const auto activeIt = _actionsById.find(activeActionId);
    if (activeIt != _actionsById.end() && activeIt->second) {
        QSignalBlocker blocker(activeIt->second);
        activeIt->second->setChecked(true);
    }
}

void MainWindow::OnSetSkin(const std::string &skinId) {
    if (skinId != "builtin.light" && skinId != "builtin.dark" && skinId != "builtin.high_contrast") {
        return;
    }

    const std::string skinPath = SkinFilePathForId(skinId);
    if (skinPath.empty()) {
        QMessageBox::warning(
            this,
            tr("Skin"),
            tr("Selected skin file was not found for id: %1").arg(ToQString(skinId)));
        return;
    }

    _editorSettings.skinId = skinId;
    SaveEditorSettings();
    LoadTheme();

    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }
        auto stateIt = _editorStates.find(editor);
        const std::string lexerName = stateIt == _editorStates.end() || stateIt->second.lexerName.empty()
            ? "null"
            : stateIt->second.lexerName;
        ApplyLexerByName(editor, lexerName);
    }
    if (_splitEditor) {
        ApplyTheme(_splitEditor);
    }
    if (_minimapEditor) {
        ApplyTheme(_minimapEditor);
    }
    SyncAuxiliaryEditorsToCurrentTab();
    UpdateMinimapViewportHighlight();
    UpdateSkinActionState();

    if (statusBar()) {
        statusBar()->showMessage(
            tr("Skin applied: %1").arg(ToQString(skinId)),
            2000);
    }
}

void MainWindow::UpdateSkinActionState() {
    const std::vector<std::string> skinActionIds = {
        "view.skin.light",
        "view.skin.dark",
        "view.skin.highContrast",
    };

    for (const std::string &id : skinActionIds) {
        const auto it = _actionsById.find(id);
        if (it == _actionsById.end() || !it->second) {
            continue;
        }
        QSignalBlocker blocker(it->second);
        it->second->setChecked(false);
    }

    const std::string activeActionId = SkinActionIdForSkinId(_editorSettings.skinId);
    const auto activeIt = _actionsById.find(activeActionId);
    if (activeIt != _actionsById.end() && activeIt->second) {
        QSignalBlocker blocker(activeIt->second);
        activeIt->second->setChecked(true);
    }
}

void MainWindow::OnDisableSplitView() {
    _editorSettings.splitViewMode = kSplitModeDisabled;
    SaveEditorSettings();
    ApplySplitViewModeFromSettings();
    statusBar()->showMessage(tr("Split view disabled"), 1500);
}

void MainWindow::OnEnableSplitVertical() {
    _editorSettings.splitViewMode = kSplitModeVertical;
    SaveEditorSettings();
    ApplySplitViewModeFromSettings();
    statusBar()->showMessage(tr("Vertical split enabled"), 1500);
}

void MainWindow::OnEnableSplitHorizontal() {
    _editorSettings.splitViewMode = kSplitModeHorizontal;
    SaveEditorSettings();
    ApplySplitViewModeFromSettings();
    statusBar()->showMessage(tr("Horizontal split enabled"), 1500);
}

void MainWindow::OnToggleMinimap() {
    _editorSettings.minimapEnabled = !_editorSettings.minimapEnabled;
    SaveEditorSettings();
    ApplyMinimapStateFromSettings();
    statusBar()->showMessage(
        _editorSettings.minimapEnabled ? tr("Minimap enabled") : tr("Minimap disabled"),
        1500);
}

void MainWindow::ApplySplitViewModeFromSettings() {
    if (!_editorSplit || !_splitEditor) {
        return;
    }

    const int splitMode = std::clamp(_editorSettings.splitViewMode, kSplitModeDisabled, kSplitModeHorizontal);
    _editorSettings.splitViewMode = splitMode;

    const bool splitEnabled = splitMode != kSplitModeDisabled;
    if (splitMode == kSplitModeHorizontal) {
        _editorSplit->setOrientation(Qt::Vertical);
    } else {
        _editorSplit->setOrientation(Qt::Horizontal);
    }

    if (splitEnabled) {
        _splitEditor->show();
        _editorSplit->setSizes({1, 1});
    } else {
        _splitEditor->hide();
    }

    const auto updateChecked = [this](const std::string &actionId, bool checked) {
        const auto it = _actionsById.find(actionId);
        if (it == _actionsById.end() || !it->second) {
            return;
        }
        QSignalBlocker blocker(it->second);
        it->second->setChecked(checked);
    };
    updateChecked("view.split.none", splitMode == kSplitModeDisabled);
    updateChecked("view.split.vertical", splitMode == kSplitModeVertical);
    updateChecked("view.split.horizontal", splitMode == kSplitModeHorizontal);

    SyncAuxiliaryEditorsToCurrentTab();
}

void MainWindow::ApplyMinimapStateFromSettings() {
    if (!_rootSplitter || !_minimapEditor) {
        return;
    }

    if (_editorSettings.minimapEnabled) {
        _minimapEditor->show();
        _rootSplitter->setSizes({9, 2});
        SyncAuxiliaryEditorsToCurrentTab();
        UpdateMinimapViewportHighlight();
    } else {
        _minimapEditor->hide();
        _minimapEditor->send(SCI_SETINDICATORCURRENT, kMinimapViewportIndicator);
        _minimapEditor->send(SCI_INDICATORCLEARRANGE, 0, _minimapEditor->send(SCI_GETTEXTLENGTH));
    }

    const auto toggleIt = _actionsById.find("view.minimap.toggle");
    if (toggleIt != _actionsById.end() && toggleIt->second) {
        QSignalBlocker blocker(toggleIt->second);
        toggleIt->second->setChecked(_editorSettings.minimapEnabled);
    }
}

void MainWindow::SyncAuxiliaryEditorsToCurrentTab() {
    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const sptr_t documentPointer = editor->send(SCI_GETDOCPOINTER);
    if (documentPointer <= 0) {
        return;
    }

    if (_splitEditor && _editorSettings.splitViewMode != kSplitModeDisabled) {
        _splitEditor->send(SCI_SETDOCPOINTER, 0, documentPointer);
        _splitEditor->send(SCI_GOTOPOS, editor->send(SCI_GETCURRENTPOS));
        _splitEditor->send(SCI_SETFIRSTVISIBLELINE, editor->send(SCI_GETFIRSTVISIBLELINE));
    }

    if (_minimapEditor && _editorSettings.minimapEnabled) {
        _minimapEditor->send(SCI_SETDOCPOINTER, 0, documentPointer);
    }
}

void MainWindow::UpdateMinimapViewportHighlight() {
    if (!_minimapEditor || !_editorSettings.minimapEnabled) {
        return;
    }

    ScintillaEditBase *editor = CurrentEditor();
    if (!editor) {
        return;
    }

    const sptr_t textLength = _minimapEditor->send(SCI_GETTEXTLENGTH);
    _minimapEditor->send(SCI_SETINDICATORCURRENT, kMinimapViewportIndicator);
    _minimapEditor->send(SCI_INDICSETSTYLE, kMinimapViewportIndicator, INDIC_ROUNDBOX);
    _minimapEditor->send(SCI_INDICSETFORE, kMinimapViewportIndicator, _themeSettings.accent);
    _minimapEditor->send(SCI_INDICSETALPHA, kMinimapViewportIndicator, 40);
    _minimapEditor->send(SCI_INDICSETOUTLINEALPHA, kMinimapViewportIndicator, 90);
    _minimapEditor->send(SCI_INDICATORCLEARRANGE, 0, textLength);

    const int lineCount = std::max(1, static_cast<int>(editor->send(SCI_GETLINECOUNT)));
    const int firstVisibleLine = std::max(0, static_cast<int>(editor->send(SCI_GETFIRSTVISIBLELINE)));
    const int linesOnScreen = std::max(1, static_cast<int>(editor->send(SCI_LINESONSCREEN)));
    const int lastVisibleLine = std::min(lineCount - 1, firstVisibleLine + linesOnScreen);

    const sptr_t startPos = _minimapEditor->send(SCI_POSITIONFROMLINE, firstVisibleLine);
    const sptr_t endPos = _minimapEditor->send(SCI_GETLINEENDPOSITION, lastVisibleLine);
    const sptr_t rangeLength = std::max<sptr_t>(1, endPos - startPos + 1);
    _minimapEditor->send(SCI_INDICATORFILLRANGE, startPos, rangeLength);

    const int minimapVisibleLines = std::max(1, static_cast<int>(_minimapEditor->send(SCI_LINESONSCREEN)));
    const int minimapFirstLine = std::clamp(
        firstVisibleLine - (minimapVisibleLines / 2),
        0,
        std::max(0, lineCount - minimapVisibleLines));
    _minimapEditor->send(SCI_SETFIRSTVISIBLELINE, minimapFirstLine);
}

bool MainWindow::FindNextInEditor(
    ScintillaEditBase *editor,
    const std::string &needleUtf8,
    bool matchCase) {
    if (!editor || needleUtf8.empty()) {
        return false;
    }

    const uptr_t flags = matchCase ? SCFIND_MATCHCASE : 0;
    const sptr_t documentLength = editor->send(SCI_GETTEXTLENGTH);
    const sptr_t start = editor->send(SCI_GETSELECTIONEND);
    const sptr_t needleLength = static_cast<sptr_t>(needleUtf8.size());

    editor->send(SCI_SETSEARCHFLAGS, flags);
    editor->send(SCI_SETTARGETRANGE, start, documentLength);
    sptr_t found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());

    if (found < 0 && start > 0) {
        editor->send(SCI_SETTARGETRANGE, 0, start);
        found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());
    }

    if (found < 0) {
        return false;
    }

    const sptr_t targetStart = editor->send(SCI_GETTARGETSTART);
    const sptr_t targetEnd = editor->send(SCI_GETTARGETEND);
    editor->send(SCI_SETSEL, targetStart, targetEnd);
    editor->send(SCI_SCROLLCARET);
    return true;
}

int MainWindow::ReplaceAllInEditor(
    ScintillaEditBase *editor,
    const std::string &needleUtf8,
    const std::string &replacementUtf8,
    bool matchCase) {
    if (!editor || needleUtf8.empty()) {
        return 0;
    }

    const uptr_t flags = matchCase ? SCFIND_MATCHCASE : 0;
    const sptr_t needleLength = static_cast<sptr_t>(needleUtf8.size());
    const sptr_t replacementLength = static_cast<sptr_t>(replacementUtf8.size());

    editor->send(SCI_BEGINUNDOACTION);
    editor->send(SCI_SETSEARCHFLAGS, flags);
    editor->send(SCI_SETTARGETRANGE, 0, editor->send(SCI_GETTEXTLENGTH));

    int replaced = 0;
    while (true) {
        const sptr_t found = editor->sends(SCI_SEARCHINTARGET, needleLength, needleUtf8.c_str());
        if (found < 0) {
            break;
        }

        const sptr_t targetStart = editor->send(SCI_GETTARGETSTART);
        editor->sends(SCI_REPLACETARGET, replacementLength, replacementUtf8.c_str());
        ++replaced;

        const sptr_t nextStart = targetStart + replacementLength;
        const sptr_t docLength = editor->send(SCI_GETTEXTLENGTH);
        editor->send(SCI_SETTARGETRANGE, nextStart, docLength);
    }

    editor->send(SCI_ENDUNDOACTION);
    return replaced;
}

std::string MainWindow::GetEditorText(ScintillaEditBase *editor) const {
    if (!editor) {
        return {};
    }

    const auto textLength = static_cast<size_t>(editor->send(SCI_GETTEXTLENGTH));
    std::string text(textLength + 1, '\0');
    editor->send(SCI_GETTEXT, textLength + 1, reinterpret_cast<sptr_t>(text.data()));
    if (!text.empty()) {
        text.resize(textLength);
    }
    return text;
}

std::string MainWindow::GetSelectedText(ScintillaEditBase *editor) const {
    if (!editor) {
        return {};
    }

    const auto start = static_cast<size_t>(editor->send(SCI_GETSELECTIONSTART));
    const auto end = static_cast<size_t>(editor->send(SCI_GETSELECTIONEND));
    if (end <= start) {
        return {};
    }

    const size_t length = end - start;
    std::string text(length + 1, '\0');
    editor->send(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(text.data()));
    text.resize(length);
    return text;
}

void MainWindow::SetEditorText(ScintillaEditBase *editor, const std::string &textUtf8) {
    if (!editor) {
        return;
    }

    editor->sends(SCI_SETTEXT, 0, textUtf8.c_str());
}

void MainWindow::SetEditorEolMode(ScintillaEditBase *editor, int eolMode) {
    if (!editor) {
        return;
    }
    editor->send(SCI_SETEOLMODE, static_cast<uptr_t>(eolMode));
    editor->send(SCI_CONVERTEOLS, static_cast<uptr_t>(eolMode));
}

int MainWindow::DetectDominantEolMode(const std::string &textUtf8) const {
    size_t crlfCount = 0;
    size_t lfCount = 0;
    size_t crCount = 0;

    for (size_t i = 0; i < textUtf8.size(); ++i) {
        if (textUtf8[i] == '\r') {
            if (i + 1 < textUtf8.size() && textUtf8[i + 1] == '\n') {
                ++crlfCount;
                ++i;
            } else {
                ++crCount;
            }
        } else if (textUtf8[i] == '\n') {
            ++lfCount;
        }
    }

    if (crlfCount >= lfCount && crlfCount >= crCount && crlfCount > 0) {
        return SC_EOL_CRLF;
    }
    if (lfCount >= crCount && lfCount > 0) {
        return SC_EOL_LF;
    }
    if (crCount > 0) {
        return SC_EOL_CR;
    }
    return SC_EOL_LF;
}

void MainWindow::MaybeAutoCloseDelimiterPair(ScintillaEditBase *editor, int ch) {
    if (!editor || _suppressAutoCloseHandler || !_editorSettings.autoCloseDelimiters) {
        return;
    }

    char closeCh = '\0';
    bool isQuote = false;
    switch (ch) {
        case '(':
            closeCh = ')';
            break;
        case '[':
            closeCh = ']';
            break;
        case '{':
            closeCh = '}';
            break;
        case '"':
            closeCh = '"';
            isQuote = true;
            break;
        case '\'':
            closeCh = '\'';
            isQuote = true;
            break;
        default:
            return;
    }

    const sptr_t caretPos = editor->send(SCI_GETCURRENTPOS);
    if (caretPos <= 0) {
        return;
    }

    const char nextChar = static_cast<char>(editor->send(SCI_GETCHARAT, caretPos));
    if (nextChar == closeCh) {
        return;
    }

    char previousChar = '\0';
    if (caretPos >= 2) {
        previousChar = static_cast<char>(editor->send(SCI_GETCHARAT, caretPos - 2));
    }
    if (isQuote) {
        if (previousChar == '\\') {
            return;
        }
        if (ch == '\'' && std::isalnum(static_cast<unsigned char>(previousChar)) != 0) {
            return;
        }
        if (std::isalnum(static_cast<unsigned char>(nextChar)) != 0) {
            return;
        }
    } else if (std::isalnum(static_cast<unsigned char>(nextChar)) != 0) {
        return;
    }

    const std::string closing(1, closeCh);
    _suppressAutoCloseHandler = true;
    editor->send(SCI_INSERTTEXT, caretPos, reinterpret_cast<sptr_t>(closing.c_str()));
    editor->send(SCI_GOTOPOS, caretPos);
    _suppressAutoCloseHandler = false;
}

void MainWindow::MaybeAutoCloseHtmlTag(ScintillaEditBase *editor, int ch) {
    if (!editor || ch != '>') {
        return;
    }
    if (_suppressAutoCloseHandler || !_editorSettings.autoCloseHtmlTags) {
        return;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    if (stateIt->second.lexerName != "xml") {
        return;
    }

    const sptr_t caretPos = editor->send(SCI_GETCURRENTPOS);
    if (caretPos <= 1) {
        return;
    }

    const sptr_t line = editor->send(SCI_LINEFROMPOSITION, caretPos);
    const sptr_t lineStart = editor->send(SCI_POSITIONFROMLINE, line);
    const sptr_t lookbackStart = std::max<sptr_t>(lineStart, caretPos - 512);
    const sptr_t lookbackLength = caretPos - lookbackStart;
    if (lookbackLength <= 0) {
        return;
    }

    std::string before(static_cast<size_t>(lookbackLength), '\0');
    for (sptr_t i = 0; i < lookbackLength; ++i) {
        before[static_cast<size_t>(i)] =
            static_cast<char>(editor->send(SCI_GETCHARAT, lookbackStart + i));
    }

    const size_t ltPos = before.find_last_of('<');
    if (ltPos == std::string::npos || ltPos + 1 >= before.size()) {
        return;
    }

    std::string inside = before.substr(ltPos + 1);
    if (inside.empty()) {
        return;
    }
    if (inside[0] == '/' || inside[0] == '!' || inside[0] == '?') {
        return;
    }
    if (!inside.empty() && inside.back() == '>') {
        inside.pop_back();
    }
    if (inside.empty()) {
        return;
    }
    if (!inside.empty() && inside.back() == '/') {
        return;
    }

    size_t tagEnd = 0;
    while (tagEnd < inside.size() && IsIdentifierChar(inside[tagEnd])) {
        ++tagEnd;
    }
    if (tagEnd == 0) {
        return;
    }

    const std::string tagName = inside.substr(0, tagEnd);
    const std::string tagLower = AsciiLower(tagName);
    static const std::set<std::string> kVoidTags = {
        "area", "base", "br", "col", "embed", "hr", "img",
        "input", "link", "meta", "param", "source", "track", "wbr",
    };
    if (kVoidTags.find(tagLower) != kVoidTags.end()) {
        return;
    }

    const sptr_t lookaheadEnd = std::min<sptr_t>(editor->send(SCI_GETTEXTLENGTH), caretPos + 2 + static_cast<sptr_t>(tagName.size()));
    if (lookaheadEnd > caretPos) {
        std::string after(static_cast<size_t>(lookaheadEnd - caretPos), '\0');
        for (sptr_t i = 0; i < lookaheadEnd - caretPos; ++i) {
            after[static_cast<size_t>(i)] =
                static_cast<char>(editor->send(SCI_GETCHARAT, caretPos + i));
        }
        std::string prefix = "</" + tagName;
        if (after.rfind(prefix, 0) == 0) {
            return;
        }
    }

    const std::string closing = "</" + tagName + ">";
    _suppressAutoCloseHandler = true;
    editor->send(SCI_INSERTTEXT, caretPos, reinterpret_cast<sptr_t>(closing.c_str()));
    editor->send(SCI_GOTOPOS, caretPos);
    _suppressAutoCloseHandler = false;
}

bool MainWindow::LoadFileIntoEditor(ScintillaEditBase *editor, const std::string &pathUtf8) {
    const auto read = _fileSystemService.ReadTextFile(pathUtf8);
    if (!read.ok()) {
        QMessageBox::critical(
            this,
            tr("Open Failed"),
            tr("Unable to open file:\n%1").arg(ToQString(read.status.message)));
        return false;
    }

    TextEncoding encoding = TextEncoding::kUtf8;
    std::string utf8Text;
    if (!DecodeTextForEditor(*read.value, &encoding, &utf8Text)) {
        QMessageBox::critical(
            this,
            tr("Open Failed"),
            tr("Unable to decode file with supported encodings."));
        return false;
    }

    const int eolMode = DetectDominantEolMode(utf8Text);
    SetEditorText(editor, utf8Text);
    SetEditorEolMode(editor, eolMode);
    editor->send(SCI_SETSAVEPOINT);

    auto &state = _editorStates[editor];
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    state.encoding = encoding;
    state.eolMode = eolMode;
    AutoDetectAndApplyLexer(editor, pathUtf8, utf8Text, "open");
    UpdateTabTitle(editor);
    statusBar()->showMessage(
        tr("Encoding: %1, EOL: %2")
            .arg(EncodingLabel(encoding))
            .arg(eolMode == SC_EOL_CRLF ? QStringLiteral("CRLF") : (eolMode == SC_EOL_CR ? QStringLiteral("CR") : QStringLiteral("LF"))),
        2000);
    return true;
}

bool MainWindow::SaveEditorToFile(ScintillaEditBase *editor, const std::string &pathUtf8) {
    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;

    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }

    stateIt->second.eolMode = static_cast<int>(editor->send(SCI_GETEOLMODE));
    const std::string currentLexer = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    if (ShouldFormatOnSaveForLexer(currentLexer)) {
        FormatEditorWithAvailableFormatter(editor, false, nullptr);
    }

    const std::string bytes = EncodeForWrite(
        GetEditorText(editor),
        stateIt->second.encoding,
        stateIt->second.eolMode);

    const npp::platform::Status writeStatus =
        _fileSystemService.WriteTextFile(pathUtf8, bytes, options);
    if (!writeStatus.ok()) {
        QMessageBox::critical(
            this,
            tr("Save Failed"),
            tr("Unable to save file:\n%1").arg(ToQString(writeStatus.message)));
        return false;
    }

    auto &state = stateIt->second;
    state.filePathUtf8 = pathUtf8;
    state.dirty = false;
    editor->send(SCI_SETSAVEPOINT);
    AutoDetectAndApplyLexer(editor, pathUtf8, GetEditorText(editor), "save");
    UpdateTabTitle(editor);
    UpdateLanguageActionState();
    return true;
}

bool MainWindow::DecodeTextForEditor(
    const std::string &bytes,
    TextEncoding *encoding,
    std::string *utf8Text) const {
    if (!encoding || !utf8Text) {
        return false;
    }

    if (bytes.size() >= 3 &&
        static_cast<unsigned char>(bytes[0]) == 0xEFU &&
        static_cast<unsigned char>(bytes[1]) == 0xBBU &&
        static_cast<unsigned char>(bytes[2]) == 0xBFU) {
        *encoding = TextEncoding::kUtf8Bom;
        *utf8Text = bytes.substr(3);
        return true;
    }

    if (bytes.size() >= 2 &&
        static_cast<unsigned char>(bytes[0]) == 0xFFU &&
        static_cast<unsigned char>(bytes[1]) == 0xFEU) {
        *encoding = TextEncoding::kUtf16Le;
        const QString text = DecodeUtf16(bytes.substr(2), true);
        *utf8Text = text.toUtf8().toStdString();
        return true;
    }

    if (bytes.size() >= 2 &&
        static_cast<unsigned char>(bytes[0]) == 0xFEU &&
        static_cast<unsigned char>(bytes[1]) == 0xFFU) {
        *encoding = TextEncoding::kUtf16Be;
        const QString text = DecodeUtf16(bytes.substr(2), false);
        *utf8Text = text.toUtf8().toStdString();
        return true;
    }

    if (IsValidUtf8(bytes)) {
        *encoding = TextEncoding::kUtf8;
        *utf8Text = bytes;
        return true;
    }

    *encoding = TextEncoding::kLocal8Bit;
    *utf8Text = QString::fromLocal8Bit(bytes.c_str(), static_cast<int>(bytes.size()))
                    .toUtf8()
                    .toStdString();
    return true;
}

std::string MainWindow::EncodeForWrite(
    const std::string &utf8Text,
    TextEncoding encoding,
    int eolMode) const {
    const std::string normalized = NormalizeEol(utf8Text, eolMode);
    const QString text = QString::fromUtf8(normalized.c_str(), static_cast<int>(normalized.size()));

    switch (encoding) {
        case TextEncoding::kUtf8:
            return normalized;
        case TextEncoding::kUtf8Bom:
            return std::string("\xEF\xBB\xBF") + normalized;
        case TextEncoding::kUtf16Le: {
            std::string out;
            out.push_back(static_cast<char>(0xFF));
            out.push_back(static_cast<char>(0xFE));
            const char16_t *units = reinterpret_cast<const char16_t *>(text.utf16());
            for (int i = 0; i < text.size(); ++i) {
                const char16_t value = units[i];
                out.push_back(static_cast<char>(value & 0xFF));
                out.push_back(static_cast<char>((value >> 8) & 0xFF));
            }
            return out;
        }
        case TextEncoding::kUtf16Be: {
            std::string out;
            out.push_back(static_cast<char>(0xFE));
            out.push_back(static_cast<char>(0xFF));
            const char16_t *units = reinterpret_cast<const char16_t *>(text.utf16());
            for (int i = 0; i < text.size(); ++i) {
                const char16_t value = units[i];
                out.push_back(static_cast<char>((value >> 8) & 0xFF));
                out.push_back(static_cast<char>(value & 0xFF));
            }
            return out;
        }
        case TextEncoding::kLocal8Bit:
            return text.toLocal8Bit().toStdString();
    }
    return normalized;
}

std::string MainWindow::NormalizeEol(const std::string &textUtf8, int eolMode) const {
    const std::string eol = eolMode == SC_EOL_CRLF ? "\r\n" : (eolMode == SC_EOL_CR ? "\r" : "\n");
    std::string normalized;
    normalized.reserve(textUtf8.size() + 16);

    for (size_t i = 0; i < textUtf8.size(); ++i) {
        const char ch = textUtf8[i];
        if (ch == '\r') {
            if (i + 1 < textUtf8.size() && textUtf8[i + 1] == '\n') {
                ++i;
            }
            normalized.append(eol);
            continue;
        }
        if (ch == '\n') {
            normalized.append(eol);
            continue;
        }
        normalized.push_back(ch);
    }
    return normalized;
}

void MainWindow::LoadEditorSettings() {
    _editorSettings.updateChannel = ResolveDefaultUpdateChannelFromInstall();
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(SettingsFilePath());
    if (!exists.ok() || !(*exists.value)) {
        return;
    }

    const auto settingsJson = _fileSystemService.ReadTextFile(SettingsFilePath());
    if (!settingsJson.ok()) {
        return;
    }

    const QByteArray settingsBytes(
        settingsJson.value->c_str(),
        static_cast<int>(settingsJson.value->size()));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(settingsBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    _editorSettings.tabWidth = std::clamp(obj.value(QStringLiteral("tabWidth")).toInt(4), 1, 12);
    _editorSettings.wrapEnabled = obj.value(QStringLiteral("wrapEnabled")).toBool(false);
    _editorSettings.showLineNumbers = obj.value(QStringLiteral("showLineNumbers")).toBool(true);
    _editorSettings.autoDetectLanguage = obj.value(QStringLiteral("autoDetectLanguage")).toBool(true);
    _editorSettings.autoCloseHtmlTags = obj.value(QStringLiteral("autoCloseHtmlTags")).toBool(true);
    _editorSettings.autoCloseDelimiters = obj.value(QStringLiteral("autoCloseDelimiters")).toBool(true);
    const QJsonValue updateChannelValue = obj.value(QStringLiteral("updateChannel"));
    if (updateChannelValue.isString()) {
        _editorSettings.updateChannel = NormalizeUpdateChannel(ToUtf8(updateChannelValue.toString()));
    }
    _editorSettings.formatOnSaveEnabled = obj.value(QStringLiteral("formatOnSaveEnabled")).toBool(false);
    _editorSettings.formatterDefaultProfile = NormalizeFormatterProfile(
        ToUtf8(obj.value(QStringLiteral("formatterDefaultProfile")).toString(QStringLiteral("auto"))));
    _editorSettings.splitViewMode = std::clamp(
        obj.value(QStringLiteral("splitViewMode")).toInt(kSplitModeDisabled),
        kSplitModeDisabled,
        kSplitModeHorizontal);
    _editorSettings.minimapEnabled = obj.value(QStringLiteral("minimapEnabled")).toBool(false);
    _editorSettings.formatOnSaveLanguages.clear();
    const QJsonValue formatOnSaveLanguagesValue = obj.value(QStringLiteral("formatOnSaveLanguages"));
    if (formatOnSaveLanguagesValue.isArray()) {
        const QJsonArray languagesArray = formatOnSaveLanguagesValue.toArray();
        for (const QJsonValue &value : languagesArray) {
            if (!value.isString()) {
                continue;
            }
            const std::vector<std::string> parsed = ParseLanguageCsvList(ToUtf8(value.toString()));
            _editorSettings.formatOnSaveLanguages.insert(
                _editorSettings.formatOnSaveLanguages.end(),
                parsed.begin(),
                parsed.end());
        }
        std::set<std::string> dedupe(
            _editorSettings.formatOnSaveLanguages.begin(),
            _editorSettings.formatOnSaveLanguages.end());
        _editorSettings.formatOnSaveLanguages.assign(dedupe.begin(), dedupe.end());
    } else if (formatOnSaveLanguagesValue.isString()) {
        _editorSettings.formatOnSaveLanguages = ParseLanguageCsvList(ToUtf8(formatOnSaveLanguagesValue.toString()));
    }
    _editorSettings.formatterProfilesByLanguage.clear();
    const QJsonValue formatterOverrideValue = obj.value(QStringLiteral("formatterProfileOverrides"));
    if (formatterOverrideValue.isObject()) {
        const QJsonObject overrideObject = formatterOverrideValue.toObject();
        for (auto it = overrideObject.begin(); it != overrideObject.end(); ++it) {
            if (!it.value().isString()) {
                continue;
            }
            const std::string language = AsciiLower(TrimAsciiWhitespace(ToUtf8(it.key())));
            const std::string profile = NormalizeFormatterProfile(ToUtf8(it.value().toString()));
            if (language.empty() || !IsValidFormatterProfile(profile)) {
                continue;
            }
            _editorSettings.formatterProfilesByLanguage.insert_or_assign(language, profile);
        }
    }
    _editorSettings.restoreSessionOnStartup = obj.value(QStringLiteral("restoreSessionOnStartup")).toBool(true);
    _editorSettings.usePerProjectSessionStorage =
        obj.value(QStringLiteral("usePerProjectSessionStorage")).toBool(false);
    const QJsonValue lastProjectRootValue = obj.value(QStringLiteral("lastProjectSessionRootUtf8"));
    if (lastProjectRootValue.isString()) {
        _editorSettings.lastProjectSessionRootUtf8 = ToUtf8(lastProjectRootValue.toString());
    }
    _editorSettings.autoSaveOnFocusLost = obj.value(QStringLiteral("autoSaveOnFocusLost")).toBool(false);
    _editorSettings.autoSaveOnInterval = obj.value(QStringLiteral("autoSaveOnInterval")).toBool(false);
    _editorSettings.autoSaveBeforeRun = obj.value(QStringLiteral("autoSaveBeforeRun")).toBool(false);
    _editorSettings.autoSaveIntervalSeconds = std::clamp(
        obj.value(QStringLiteral("autoSaveIntervalSeconds")).toInt(30),
        5,
        600);
    _editorSettings.extensionGuardrailsEnabled =
        obj.value(QStringLiteral("extensionGuardrailsEnabled")).toBool(true);
    _editorSettings.extensionStartupBudgetMs = std::clamp(
        obj.value(QStringLiteral("extensionStartupBudgetMs")).toInt(1200),
        100,
        15000);
    _editorSettings.extensionPerExtensionBudgetMs = std::clamp(
        obj.value(QStringLiteral("extensionPerExtensionBudgetMs")).toInt(250),
        10,
        5000);
    _editorSettings.crashRecoveryAutosaveSeconds = std::clamp(
        obj.value(QStringLiteral("crashRecoveryAutosaveSeconds")).toInt(15),
        5,
        300);
    const QJsonValue skinValue = obj.value(QStringLiteral("skinId"));
    if (skinValue.isString()) {
        const std::string loadedSkinId = ToUtf8(skinValue.toString());
        if (!loadedSkinId.empty()) {
            _editorSettings.skinId = loadedSkinId;
        }
    }
}

void MainWindow::SaveEditorSettings() const {
    const_cast<MainWindow *>(this)->EnsureConfigRoot();
    QJsonObject settingsObject;
    settingsObject.insert(QStringLiteral("tabWidth"), _editorSettings.tabWidth);
    settingsObject.insert(QStringLiteral("wrapEnabled"), _editorSettings.wrapEnabled);
    settingsObject.insert(QStringLiteral("showLineNumbers"), _editorSettings.showLineNumbers);
    settingsObject.insert(QStringLiteral("autoDetectLanguage"), _editorSettings.autoDetectLanguage);
    settingsObject.insert(QStringLiteral("autoCloseHtmlTags"), _editorSettings.autoCloseHtmlTags);
    settingsObject.insert(QStringLiteral("autoCloseDelimiters"), _editorSettings.autoCloseDelimiters);
    settingsObject.insert(QStringLiteral("updateChannel"), ToQString(NormalizeUpdateChannel(_editorSettings.updateChannel)));
    settingsObject.insert(QStringLiteral("formatOnSaveEnabled"), _editorSettings.formatOnSaveEnabled);
    settingsObject.insert(
        QStringLiteral("formatterDefaultProfile"),
        ToQString(NormalizeFormatterProfile(_editorSettings.formatterDefaultProfile)));
    settingsObject.insert(QStringLiteral("splitViewMode"), _editorSettings.splitViewMode);
    settingsObject.insert(QStringLiteral("minimapEnabled"), _editorSettings.minimapEnabled);
    QJsonArray formatOnSaveLanguagesArray;
    for (const std::string &language : _editorSettings.formatOnSaveLanguages) {
        formatOnSaveLanguagesArray.append(ToQString(language));
    }
    settingsObject.insert(QStringLiteral("formatOnSaveLanguages"), formatOnSaveLanguagesArray);
    QJsonObject formatterOverrideObject;
    for (const auto &[language, profile] : _editorSettings.formatterProfilesByLanguage) {
        formatterOverrideObject.insert(ToQString(language), ToQString(profile));
    }
    settingsObject.insert(QStringLiteral("formatterProfileOverrides"), formatterOverrideObject);
    settingsObject.insert(QStringLiteral("restoreSessionOnStartup"), _editorSettings.restoreSessionOnStartup);
    settingsObject.insert(
        QStringLiteral("usePerProjectSessionStorage"),
        _editorSettings.usePerProjectSessionStorage);
    settingsObject.insert(
        QStringLiteral("lastProjectSessionRootUtf8"),
        ToQString(_editorSettings.lastProjectSessionRootUtf8));
    settingsObject.insert(QStringLiteral("autoSaveOnFocusLost"), _editorSettings.autoSaveOnFocusLost);
    settingsObject.insert(QStringLiteral("autoSaveOnInterval"), _editorSettings.autoSaveOnInterval);
    settingsObject.insert(QStringLiteral("autoSaveBeforeRun"), _editorSettings.autoSaveBeforeRun);
    settingsObject.insert(QStringLiteral("autoSaveIntervalSeconds"), _editorSettings.autoSaveIntervalSeconds);
    settingsObject.insert(QStringLiteral("extensionGuardrailsEnabled"), _editorSettings.extensionGuardrailsEnabled);
    settingsObject.insert(QStringLiteral("extensionStartupBudgetMs"), _editorSettings.extensionStartupBudgetMs);
    settingsObject.insert(
        QStringLiteral("extensionPerExtensionBudgetMs"),
        _editorSettings.extensionPerExtensionBudgetMs);
    settingsObject.insert(
        QStringLiteral("crashRecoveryAutosaveSeconds"),
        _editorSettings.crashRecoveryAutosaveSeconds);
    settingsObject.insert(QStringLiteral("skinId"), ToQString(_editorSettings.skinId));

    const QJsonDocument doc(settingsObject);
    const auto json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    const_cast<MainWindow *>(this)->_fileSystemService.WriteTextFile(
        SettingsFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::ApplyEditorSettingsToAllEditors() {
    if (!_tabs) {
        return;
    }
    for (int index = 0; index < _tabs->count(); ++index) {
        ApplyEditorSettings(EditorAt(index));
    }
    ApplyEditorSettings(_splitEditor);
}

void MainWindow::ApplyEditorSettings(ScintillaEditBase *editor) {
    if (!editor) {
        return;
    }
    editor->send(SCI_SETTABWIDTH, _editorSettings.tabWidth);
    editor->send(SCI_SETWRAPMODE, _editorSettings.wrapEnabled ? SC_WRAP_WORD : SC_WRAP_NONE);
    editor->send(SCI_SETMARGINWIDTHN, 0, _editorSettings.showLineNumbers ? LineNumberMarginWidth(editor) : 0);
}

void MainWindow::ApplyTheme(ScintillaEditBase *editor) {
    if (!editor) {
        return;
    }

    editor->send(SCI_STYLESETFORE, STYLE_DEFAULT, _themeSettings.foreground);
    editor->send(SCI_STYLESETBACK, STYLE_DEFAULT, _themeSettings.background);
    editor->send(SCI_STYLECLEARALL);

    editor->send(SCI_STYLESETFORE, STYLE_LINENUMBER, _themeSettings.lineNumberForeground);
    editor->send(SCI_STYLESETBACK, STYLE_LINENUMBER, _themeSettings.lineNumberBackground);
    editor->send(SCI_SETCARETFORE, _themeSettings.foreground);
    editor->send(SCI_SETCARETLINEBACK, _themeSettings.caretLineBackground);
    editor->send(SCI_SETSELBACK, 1, _themeSettings.selectionBackground);
    editor->send(SCI_SETSELFORE, 1, _themeSettings.selectionForeground);
}

void MainWindow::ApplyChromeTheme() {
    const int tabBackground = BlendThemeColor(_themeSettings.menuBackground, _themeSettings.windowBackground, 0.35);
    const int tabActiveBackground = BlendThemeColor(_themeSettings.background, _themeSettings.windowBackground, 0.15);
    const int tabHoverBackground = BlendThemeColor(_themeSettings.menuBackground, _themeSettings.accent, 0.15);
    const int fieldBackground = BlendThemeColor(_themeSettings.dialogBackground, _themeSettings.background, 0.25);
    const int disabledForeground = BlendThemeColor(_themeSettings.menuForeground, _themeSettings.dialogBackground, 0.35);

    const QString styleSheet = QStringLiteral(
                                   "QMainWindow { background-color: %1; color: %2; }"
                                   "QMenuBar { background-color: %3; color: %4; }"
                                   "QMenuBar::item:selected { background-color: %5; color: %4; }"
                                   "QMenu { background-color: %3; color: %4; }"
                                   "QMenu::item:selected { background-color: %5; color: %4; }"
                                   "QStatusBar { background-color: %6; color: %7; }"
                                   "QStatusBar::item { border: none; }"
                                   "QTabWidget::pane { border: 1px solid %12; background-color: %1; top: -1px; }"
                                   "QTabBar::tab { background-color: %13; color: %4; border: 1px solid %12; "
                                   "border-bottom: none; padding: 6px 12px; margin-right: 2px; }"
                                   "QTabBar::tab:selected { background-color: %14; color: %2; }"
                                   "QTabBar::tab:hover:!selected { background-color: %15; }"
                                   "QDialog { background-color: %8; color: %9; }"
                                   "QPushButton { background-color: %10; color: %11; "
                                   "border: 1px solid %12; padding: 4px 8px; }"
                                   "QPushButton:hover { background-color: %15; }"
                                   "QPushButton:disabled { color: %17; }"
                                   "QLineEdit, QSpinBox, QTextEdit, QPlainTextEdit, QComboBox { "
                                   "background-color: %16; color: %9; border: 1px solid %12; "
                                   "selection-background-color: %5; selection-color: %11; }"
                                   "QLineEdit:focus, QSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus { "
                                   "border: 2px solid %5; }"
                                   "QDialog QLabel { color: %9; }"
                                   "QCheckBox { color: %9; }"
                                   "QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid %12; "
                                   "background-color: %16; }"
                                   "QCheckBox::indicator:checked { background-color: %5; border-color: %5; }"
                                   "QToolTip { background-color: %3; color: %4; border: 1px solid %12; }")
                                   .arg(ThemeColorHex(_themeSettings.windowBackground))
                                   .arg(ThemeColorHex(_themeSettings.windowForeground))
                                   .arg(ThemeColorHex(_themeSettings.menuBackground))
                                   .arg(ThemeColorHex(_themeSettings.menuForeground))
                                   .arg(ThemeColorHex(_themeSettings.accent))
                                   .arg(ThemeColorHex(_themeSettings.statusBackground))
                                   .arg(ThemeColorHex(_themeSettings.statusForeground))
                                   .arg(ThemeColorHex(_themeSettings.dialogBackground))
                                   .arg(ThemeColorHex(_themeSettings.dialogForeground))
                                   .arg(ThemeColorHex(_themeSettings.dialogButtonBackground))
                                   .arg(ThemeColorHex(_themeSettings.dialogButtonForeground))
                                   .arg(ThemeColorHex(_themeSettings.dialogBorder))
                                   .arg(ThemeColorHex(tabBackground))
                                   .arg(ThemeColorHex(tabActiveBackground))
                                   .arg(ThemeColorHex(tabHoverBackground))
                                   .arg(ThemeColorHex(fieldBackground))
                                   .arg(ThemeColorHex(disabledForeground));
    setStyleSheet(styleSheet);
}

void MainWindow::ApplyLexerForPath(ScintillaEditBase *editor, const std::string &pathUtf8) {
    AutoDetectAndApplyLexer(editor, pathUtf8, GetEditorText(editor), "path");
}

void MainWindow::AutoDetectAndApplyLexer(
    ScintillaEditBase *editor,
    const std::string &pathUtf8,
    const std::string &contentUtf8,
    const char *trigger,
    bool force) {
    if (!editor) {
        return;
    }

    auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return;
    }
    if (!force && !_editorSettings.autoDetectLanguage) {
        UpdateLanguageActionState();
        return;
    }
    if (stateIt->second.lexerManualLock) {
        ApplyLexerByName(editor, stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName);
        UpdateLanguageActionState();
        return;
    }

    constexpr double kAutoApplyThreshold = 0.60;
    const std::string previousLexer = stateIt->second.lexerName.empty() ? "null" : stateIt->second.lexerName;
    const std::string previousReason = stateIt->second.lexerReason;
    const double previousConfidence = stateIt->second.lexerConfidence;
    const npp::ui::LanguageDetectionResult detection = npp::ui::DetectLanguage(pathUtf8, contentUtf8);
    const bool shouldAutoApply = detection.confidence >= kAutoApplyThreshold || detection.reason == "extension";
    const std::string chosenLexer = shouldAutoApply ? detection.lexerName : std::string("null");
    const bool unchanged = chosenLexer == previousLexer &&
        detection.reason == previousReason &&
        std::abs(detection.confidence - previousConfidence) < 0.0001;
    if (!force && unchanged) {
        return;
    }

    stateIt->second.lexerName = chosenLexer;
    stateIt->second.lexerConfidence = detection.confidence;
    stateIt->second.lexerReason = detection.reason;
    ApplyLexerByName(editor, chosenLexer);

    if (statusBar()) {
        statusBar()->showMessage(
            tr("Language %1 (%2%% via %3, %4)")
                .arg(ToQString(chosenLexer))
                .arg(ToQString(PercentString(detection.confidence)))
                .arg(ToQString(detection.reason))
                .arg(QString::fromLatin1(trigger)),
            2500);
    }
    UpdateLanguageActionState();
}

void MainWindow::ApplyLexerByName(ScintillaEditBase *editor, const std::string &lexerName) {
    if (!editor) {
        return;
    }

#if defined(NPP_HAVE_LEXILLA) && NPP_HAVE_LEXILLA
    auto *lexer = CreateLexer(lexerName.c_str());
    editor->send(SCI_SETILEXER, 0, reinterpret_cast<sptr_t>(lexer));
#else
    static_cast<void>(lexerName);
    editor->send(SCI_SETILEXER, 0, 0);
#endif

    if (lexerName == "cpp") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "alignas alignof and and_eq asm auto bitand bitor bool break case catch char class "
            "const constexpr const_cast continue decltype default delete do double dynamic_cast "
            "else enum explicit export extern false float for friend goto if inline int long "
            "mutable namespace new noexcept not not_eq nullptr operator or or_eq private "
            "protected public register reinterpret_cast return short signed sizeof static "
            "static_assert static_cast struct switch template this thread_local throw true try "
            "typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq");
    } else if (lexerName == "python") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "False None True and as assert async await break class continue def del elif else "
            "except finally for from global if import in is lambda nonlocal not or pass raise "
            "return try while with yield");
    } else if (lexerName == "yaml") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "true false yes no on off null");
    } else if (lexerName == "sql") {
        editor->sends(
            SCI_SETKEYWORDS,
            0,
            "select from where join inner outer left right full on group by order having "
            "insert into values update delete create alter drop table view index as and or "
            "not in exists like between null distinct limit offset case when then else end");
    }

    ApplyTheme(editor);
    ApplyLexerStyles(editor, lexerName);
    editor->send(SCI_COLOURISE, 0, -1);
}

void MainWindow::ApplyLexerStyles(ScintillaEditBase *editor, const std::string &lexerName) {
    if (!editor) {
        return;
    }
    const auto colorForRole = [this](npp::ui::LexerStyleColorRole role) -> int {
        switch (role) {
            case npp::ui::LexerStyleColorRole::kComment:
                return _themeSettings.comment;
            case npp::ui::LexerStyleColorRole::kNumber:
                return _themeSettings.number;
            case npp::ui::LexerStyleColorRole::kKeyword:
                return _themeSettings.keyword;
            case npp::ui::LexerStyleColorRole::kString:
                return _themeSettings.stringColor;
            case npp::ui::LexerStyleColorRole::kOperator:
                return _themeSettings.operatorColor;
            case npp::ui::LexerStyleColorRole::kForeground:
                return _themeSettings.foreground;
        }
        return _themeSettings.foreground;
    };

    const auto &rules = npp::ui::LexerStyleRulesFor(lexerName);
    for (const auto &rule : rules) {
        editor->send(SCI_STYLESETFORE, rule.styleId, colorForRole(rule.colorRole));
        editor->send(SCI_STYLESETBOLD, rule.styleId, rule.bold ? 1 : 0);
    }
}

void MainWindow::EnsureConfigRoot() {
    const std::string root = ConfigRootPath();
    if (root.empty()) {
        return;
    }
    _fileSystemService.CreateDirectories(root);
}

void MainWindow::EnsureBuiltInSkins() {
    EnsureConfigRoot();
    const std::string skinRoot = SkinDirectoryPath();
    if (skinRoot.empty()) {
        return;
    }
    _fileSystemService.CreateDirectories(skinRoot);

    struct BuiltInSkin {
        const char *id;
        const char *fileName;
        const char *json;
    };

    static const BuiltInSkin kBuiltIns[] = {
        {
            "builtin.light",
            "light.json",
            R"({
  "$schema": "https://raw.githubusercontent.com/RossEngineering/notepad-plus-plus-linux/master/docs/schemas/skin-v1.schema.json",
  "formatVersion": 1,
  "metadata": {"id": "builtin.light", "name": "Built-in Light"},
  "appChrome": {"windowBackground": "#F3F3F3", "windowForeground": "#1A1A1A", "menuBackground": "#FFFFFF", "menuForeground": "#1A1A1A", "statusBackground": "#EFEFEF", "statusForeground": "#1A1A1A", "accent": "#005FB8"},
  "editor": {"background": "#FFFFFF", "foreground": "#1A1A1A", "lineNumberBackground": "#F2F2F2", "lineNumberForeground": "#616161", "caretLineBackground": "#F7FAFF", "selectionBackground": "#CFE8FF", "selectionForeground": "#000000", "comment": "#6A9955", "keyword": "#005FB8", "number": "#9A3E9D", "stringColor": "#B34100", "operatorColor": "#1A1A1A"},
  "dialogs": {"background": "#FFFFFF", "foreground": "#1A1A1A", "buttonBackground": "#E9EEF6", "buttonForeground": "#1A1A1A", "border": "#B5C3D6"}
})"},
        {
            "builtin.dark",
            "dark.json",
            R"({
  "$schema": "https://raw.githubusercontent.com/RossEngineering/notepad-plus-plus-linux/master/docs/schemas/skin-v1.schema.json",
  "formatVersion": 1,
  "metadata": {"id": "builtin.dark", "name": "Built-in Dark"},
  "appChrome": {"windowBackground": "#1B1E23", "windowForeground": "#E6EAF2", "menuBackground": "#242933", "menuForeground": "#E6EAF2", "statusBackground": "#20242D", "statusForeground": "#DCE2EC", "accent": "#4FA3FF"},
  "editor": {"background": "#15181E", "foreground": "#DDE3EE", "lineNumberBackground": "#20242D", "lineNumberForeground": "#77839A", "caretLineBackground": "#1F2530", "selectionBackground": "#304764", "selectionForeground": "#F3F7FF", "comment": "#7FA06E", "keyword": "#6CB6FF", "number": "#C78AF3", "stringColor": "#FFB86B", "operatorColor": "#DDE3EE"},
  "dialogs": {"background": "#242933", "foreground": "#E6EAF2", "buttonBackground": "#2C3442", "buttonForeground": "#E6EAF2", "border": "#41506A"}
})"},
        {
            "builtin.high_contrast",
            "high-contrast.json",
            R"({
  "$schema": "https://raw.githubusercontent.com/RossEngineering/notepad-plus-plus-linux/master/docs/schemas/skin-v1.schema.json",
  "formatVersion": 1,
  "metadata": {"id": "builtin.high_contrast", "name": "Built-in High Contrast"},
  "appChrome": {"windowBackground": "#000000", "windowForeground": "#FFFFFF", "menuBackground": "#000000", "menuForeground": "#FFFFFF", "statusBackground": "#000000", "statusForeground": "#FFFFFF", "accent": "#FFD700"},
  "editor": {"background": "#000000", "foreground": "#FFFFFF", "lineNumberBackground": "#000000", "lineNumberForeground": "#FFD700", "caretLineBackground": "#111111", "selectionBackground": "#0033CC", "selectionForeground": "#FFFFFF", "comment": "#00FF00", "keyword": "#FFD700", "number": "#00FFFF", "stringColor": "#FF8C00", "operatorColor": "#FFFFFF"},
  "dialogs": {"background": "#000000", "foreground": "#FFFFFF", "buttonBackground": "#1A1A1A", "buttonForeground": "#FFFFFF", "border": "#FFD700"}
})"},
    };

    for (const BuiltInSkin &skin : kBuiltIns) {
        const std::string skinPath = SkinDirectoryPath() + "/" + skin.fileName;
        const auto exists = _fileSystemService.Exists(skinPath);
        if (exists.ok() && *exists.value) {
            continue;
        }

        npp::platform::WriteFileOptions options;
        options.atomic = true;
        options.createParentDirs = true;
        _fileSystemService.WriteTextFile(skinPath, std::string(skin.json), options);
    }
}

void MainWindow::EnsureThemeFile() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(ThemeFilePath());
    if (exists.ok() && *exists.value) {
        return;
    }

    QJsonObject metadata;
    metadata.insert(QStringLiteral("id"), QStringLiteral("builtin.light"));
    metadata.insert(QStringLiteral("name"), QStringLiteral("Built-in Light"));
    metadata.insert(QStringLiteral("author"), QStringLiteral("notepad-plus-plus-linux"));
    metadata.insert(
        QStringLiteral("description"),
        QStringLiteral("Default light skin for app chrome, editor, and dialogs."));

    QJsonObject appChrome;
    appChrome.insert(QStringLiteral("windowBackground"), QStringLiteral("#F3F3F3"));
    appChrome.insert(QStringLiteral("windowForeground"), QStringLiteral("#1A1A1A"));
    appChrome.insert(QStringLiteral("menuBackground"), QStringLiteral("#FFFFFF"));
    appChrome.insert(QStringLiteral("menuForeground"), QStringLiteral("#1A1A1A"));
    appChrome.insert(QStringLiteral("statusBackground"), QStringLiteral("#EFEFEF"));
    appChrome.insert(QStringLiteral("statusForeground"), QStringLiteral("#1A1A1A"));
    appChrome.insert(QStringLiteral("accent"), QStringLiteral("#005FB8"));

    QJsonObject editor;
    editor.insert(QStringLiteral("background"), QStringLiteral("#FFFFFF"));
    editor.insert(QStringLiteral("foreground"), QStringLiteral("#1A1A1A"));
    editor.insert(QStringLiteral("lineNumberBackground"), QStringLiteral("#F2F2F2"));
    editor.insert(QStringLiteral("lineNumberForeground"), QStringLiteral("#616161"));
    editor.insert(QStringLiteral("caretLineBackground"), QStringLiteral("#F7FAFF"));
    editor.insert(QStringLiteral("selectionBackground"), QStringLiteral("#CFE8FF"));
    editor.insert(QStringLiteral("selectionForeground"), QStringLiteral("#000000"));
    editor.insert(QStringLiteral("comment"), QStringLiteral("#6A9955"));
    editor.insert(QStringLiteral("keyword"), QStringLiteral("#005FB8"));
    editor.insert(QStringLiteral("number"), QStringLiteral("#9A3E9D"));
    editor.insert(QStringLiteral("stringColor"), QStringLiteral("#B34100"));
    editor.insert(QStringLiteral("operatorColor"), QStringLiteral("#1A1A1A"));

    QJsonObject dialogs;
    dialogs.insert(QStringLiteral("background"), QStringLiteral("#FFFFFF"));
    dialogs.insert(QStringLiteral("foreground"), QStringLiteral("#1A1A1A"));
    dialogs.insert(QStringLiteral("buttonBackground"), QStringLiteral("#E9EEF6"));
    dialogs.insert(QStringLiteral("buttonForeground"), QStringLiteral("#1A1A1A"));
    dialogs.insert(QStringLiteral("border"), QStringLiteral("#B5C3D6"));

    QJsonObject themeJson;
    themeJson.insert(
        QStringLiteral("$schema"),
        QStringLiteral("https://raw.githubusercontent.com/RossEngineering/notepad-plus-plus-linux/master/docs/schemas/skin-v1.schema.json"));
    themeJson.insert(QStringLiteral("formatVersion"), 1);
    themeJson.insert(QStringLiteral("metadata"), metadata);
    themeJson.insert(QStringLiteral("appChrome"), appChrome);
    themeJson.insert(QStringLiteral("editor"), editor);
    themeJson.insert(QStringLiteral("dialogs"), dialogs);

    const QJsonDocument doc(themeJson);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    _fileSystemService.WriteTextFile(
        ThemeFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::LoadTheme() {
    EnsureThemeFile();
    std::string selectedThemePath = ThemeFilePath();
    if (!_editorSettings.skinId.empty()) {
        const std::string builtInSkinPath = SkinFilePathForId(_editorSettings.skinId);
        if (!builtInSkinPath.empty()) {
            selectedThemePath = builtInSkinPath;
        }
    }

    auto themeJson = _fileSystemService.ReadTextFile(selectedThemePath);
    if (!themeJson.ok() && selectedThemePath != ThemeFilePath()) {
        themeJson = _fileSystemService.ReadTextFile(ThemeFilePath());
    }
    if (!themeJson.ok()) {
        return;
    }

    QJsonParseError parseError{};
    const QByteArray bytes(themeJson.value->c_str(), static_cast<int>(themeJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();

    // Skin v1 sections (`appChrome`, `editor`, `dialogs`) are preferred.
    // Fallback to legacy flat keys so existing user themes keep working.
    _themeSettings.windowBackground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "windowBackground",
        ParseThemeColor(obj, "windowBackground", _themeSettings.windowBackground));
    _themeSettings.windowForeground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "windowForeground",
        ParseThemeColor(obj, "windowForeground", _themeSettings.windowForeground));
    _themeSettings.menuBackground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "menuBackground",
        ParseThemeColor(obj, "menuBackground", _themeSettings.menuBackground));
    _themeSettings.menuForeground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "menuForeground",
        ParseThemeColor(obj, "menuForeground", _themeSettings.menuForeground));
    _themeSettings.statusBackground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "statusBackground",
        ParseThemeColor(obj, "statusBackground", _themeSettings.statusBackground));
    _themeSettings.statusForeground = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "statusForeground",
        ParseThemeColor(obj, "statusForeground", _themeSettings.statusForeground));
    _themeSettings.accent = ParseThemeColorFromSection(
        obj,
        "appChrome",
        "accent",
        ParseThemeColor(obj, "accent", _themeSettings.accent));

    _themeSettings.background = ParseThemeColorFromSection(
        obj,
        "editor",
        "background",
        ParseThemeColor(obj, "background", _themeSettings.background));
    _themeSettings.foreground = ParseThemeColorFromSection(
        obj,
        "editor",
        "foreground",
        ParseThemeColor(obj, "foreground", _themeSettings.foreground));
    _themeSettings.lineNumberBackground = ParseThemeColorFromSection(
        obj,
        "editor",
        "lineNumberBackground",
        ParseThemeColor(obj, "lineNumberBackground", _themeSettings.lineNumberBackground));
    _themeSettings.lineNumberForeground = ParseThemeColorFromSection(
        obj,
        "editor",
        "lineNumberForeground",
        ParseThemeColor(obj, "lineNumberForeground", _themeSettings.lineNumberForeground));
    _themeSettings.caretLineBackground = ParseThemeColorFromSection(
        obj,
        "editor",
        "caretLineBackground",
        ParseThemeColor(obj, "caretLineBackground", _themeSettings.caretLineBackground));
    _themeSettings.selectionBackground = ParseThemeColorFromSection(
        obj,
        "editor",
        "selectionBackground",
        ParseThemeColor(obj, "selectionBackground", _themeSettings.selectionBackground));
    _themeSettings.selectionForeground = ParseThemeColorFromSection(
        obj,
        "editor",
        "selectionForeground",
        ParseThemeColor(obj, "selectionForeground", _themeSettings.selectionForeground));
    _themeSettings.comment = ParseThemeColorFromSection(
        obj,
        "editor",
        "comment",
        ParseThemeColor(obj, "comment", _themeSettings.comment));
    _themeSettings.keyword = ParseThemeColorFromSection(
        obj,
        "editor",
        "keyword",
        ParseThemeColor(obj, "keyword", _themeSettings.keyword));
    _themeSettings.number = ParseThemeColorFromSection(
        obj,
        "editor",
        "number",
        ParseThemeColor(obj, "number", _themeSettings.number));
    _themeSettings.stringColor = ParseThemeColorFromSection(
        obj,
        "editor",
        "stringColor",
        ParseThemeColor(obj, "stringColor", _themeSettings.stringColor));
    _themeSettings.operatorColor = ParseThemeColorFromSection(
        obj,
        "editor",
        "operatorColor",
        ParseThemeColor(obj, "operatorColor", _themeSettings.operatorColor));

    _themeSettings.dialogBackground = ParseThemeColorFromSection(
        obj,
        "dialogs",
        "background",
        ParseThemeColor(obj, "dialogBackground", _themeSettings.dialogBackground));
    _themeSettings.dialogForeground = ParseThemeColorFromSection(
        obj,
        "dialogs",
        "foreground",
        ParseThemeColor(obj, "dialogForeground", _themeSettings.dialogForeground));
    _themeSettings.dialogButtonBackground = ParseThemeColorFromSection(
        obj,
        "dialogs",
        "buttonBackground",
        ParseThemeColor(obj, "dialogButtonBackground", _themeSettings.dialogButtonBackground));
    _themeSettings.dialogButtonForeground = ParseThemeColorFromSection(
        obj,
        "dialogs",
        "buttonForeground",
        ParseThemeColor(obj, "dialogButtonForeground", _themeSettings.dialogButtonForeground));
    _themeSettings.dialogBorder = ParseThemeColorFromSection(
        obj,
        "dialogs",
        "border",
        ParseThemeColor(obj, "dialogBorder", _themeSettings.dialogBorder));

    ApplyChromeTheme();
}

void MainWindow::EnsureShortcutConfigFile() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(ShortcutFilePath());
    if (exists.ok() && *exists.value) {
        return;
    }

    QJsonObject defaults;
    for (const auto &[actionId, shortcut] : DefaultShortcutMap()) {
        defaults.insert(QString::fromStdString(actionId), shortcut.toString(QKeySequence::PortableText));
    }

    const QJsonDocument doc(defaults);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    _fileSystemService.WriteTextFile(
        ShortcutFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::LoadShortcutOverrides() {
    _shortcutOverrides = QJsonObject{};

    const auto exists = _fileSystemService.Exists(ShortcutFilePath());
    if (!exists.ok() || !(*exists.value)) {
        return;
    }

    const auto jsonFile = _fileSystemService.ReadTextFile(ShortcutFilePath());
    if (!jsonFile.ok()) {
        return;
    }

    const QByteArray bytes(jsonFile.value->c_str(), static_cast<int>(jsonFile.value->size()));
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    _shortcutOverrides = doc.object();
}

void MainWindow::ApplyShortcuts() {
    const auto &defaults = DefaultShortcutMap();
    for (const auto &[id, action] : _actionsById) {
        if (!action) {
            continue;
        }

        QKeySequence shortcut;
        const auto defaultIt = defaults.find(id);
        if (defaultIt != defaults.end()) {
            shortcut = defaultIt->second;
        }

        if (_shortcutOverrides.contains(QString::fromStdString(id))) {
            const QJsonValue overrideValue = _shortcutOverrides.value(QString::fromStdString(id));
            if (overrideValue.isString()) {
                const QKeySequence overrideShortcut(overrideValue.toString());
                if (!overrideShortcut.isEmpty()) {
                    shortcut = overrideShortcut;
                }
            }
        }

        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
    }
}

void MainWindow::ReloadShortcuts() {
    LoadShortcutOverrides();
    ApplyShortcuts();
    statusBar()->showMessage(tr("Shortcuts reloaded"), 1500);
}

void MainWindow::OpenShortcutConfigFile() {
    EnsureShortcutConfigFile();
    const npp::platform::Status status = _processService.OpenPath(ShortcutFilePath());
    if (!status.ok()) {
        QMessageBox::warning(
            this,
            tr("Shortcuts"),
            tr("Unable to open shortcut config file:\n%1").arg(ToQString(status.message)));
    }
}

void MainWindow::InitializeExtensionsWithGuardrails() {
    _extensionStartupEstimateMsById.clear();
    _extensionManifestScanMsById.clear();

    const auto startupBegin = std::chrono::steady_clock::now();
    const npp::platform::Status initStatus = _extensionService.Initialize();
    const auto afterInit = std::chrono::steady_clock::now();

    if (!initStatus.ok()) {
        statusBar()->showMessage(
            tr("Extension service init failed: %1").arg(ToQString(initStatus.message)),
            5000);
        return;
    }

    const auto discovered = _extensionService.DiscoverInstalled();
    const auto startupEnd = std::chrono::steady_clock::now();
    if (!discovered.ok()) {
        statusBar()->showMessage(
            tr("Extension discovery failed: %1").arg(ToQString(discovered.status.message)),
            5000);
        return;
    }

    const std::size_t extensionCount = discovered.value->size();
    const auto initMs = std::chrono::duration_cast<std::chrono::milliseconds>(afterInit - startupBegin).count();
    const auto discoverMs = std::chrono::duration_cast<std::chrono::milliseconds>(startupEnd - afterInit).count();
    const auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(startupEnd - startupBegin).count();
    const double perExtensionMs = extensionCount > 0
        ? static_cast<double>(discoverMs) / static_cast<double>(extensionCount)
        : static_cast<double>(discoverMs);
    for (const npp::platform::InstalledExtension &extension : *discovered.value) {
        _extensionStartupEstimateMsById.insert_or_assign(extension.manifest.id, perExtensionMs);
        const auto manifestStart = std::chrono::steady_clock::now();
        const auto manifestStatus = _extensionService.LoadManifestFromDirectory(extension.installPath);
        const auto manifestEnd = std::chrono::steady_clock::now();
        if (manifestStatus.ok()) {
            const double scanMs = static_cast<double>(
                std::chrono::duration_cast<std::chrono::milliseconds>(manifestEnd - manifestStart).count());
            _extensionManifestScanMsById.insert_or_assign(extension.manifest.id, scanMs);
        }
    }

    if (!_editorSettings.extensionGuardrailsEnabled) {
        return;
    }

    const bool startupExceeded = totalMs > _editorSettings.extensionStartupBudgetMs;
    const bool perExtensionExceeded = extensionCount > 0 &&
        perExtensionMs > static_cast<double>(_editorSettings.extensionPerExtensionBudgetMs);
    if (!startupExceeded && !perExtensionExceeded) {
        return;
    }

    const QString summary = tr(
        "Extension startup guardrail: total=%1ms (budget=%2ms), per-extension=%3ms (budget=%4ms), count=%5")
            .arg(totalMs)
            .arg(_editorSettings.extensionStartupBudgetMs)
            .arg(QString::number(perExtensionMs, 'f', 1))
            .arg(_editorSettings.extensionPerExtensionBudgetMs)
            .arg(static_cast<int>(extensionCount));
    statusBar()->showMessage(summary, 7000);

    std::ostringstream details;
    details << "captured_at_utc=" << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toStdString() << "\n";
    details << "extension_init_ms=" << initMs << "\n";
    details << "extension_discover_ms=" << discoverMs << "\n";
    details << "extension_startup_total_ms=" << totalMs << "\n";
    details << "extension_count=" << extensionCount << "\n";
    details << "extension_per_extension_ms=" << perExtensionMs << "\n";
    details << "startup_budget_ms=" << _editorSettings.extensionStartupBudgetMs << "\n";
    details << "per_extension_budget_ms=" << _editorSettings.extensionPerExtensionBudgetMs << "\n";

    const std::string fileName =
        "extension-startup-guardrail-" +
        std::to_string(QDateTime::currentSecsSinceEpoch()) + ".log";
    _diagnosticsService.WriteDiagnostic(kAppName, "logs", fileName, details.str());
}

void MainWindow::NotifyExtensionUpdatesIfAvailable() {
    if (_safeModeNoExtensions) {
        return;
    }

    const std::string indexPath = ResolveLocalExtensionMarketplaceIndexPath();
    if (indexPath.empty()) {
        return;
    }

    const auto indexJson = _fileSystemService.ReadTextFile(indexPath);
    if (!indexJson.ok()) {
        return;
    }

    QString parseError;
    const std::vector<ExtensionMarketplaceEntry> entries =
        ParseMarketplaceIndex(*indexJson.value, indexPath, &parseError);
    if (entries.empty()) {
        return;
    }

    const auto discovered = _extensionService.DiscoverInstalled();
    if (!discovered.ok()) {
        return;
    }

    std::map<std::string, std::string> installedVersionById;
    for (const npp::platform::InstalledExtension &installed : *discovered.value) {
        installedVersionById.insert_or_assign(installed.manifest.id, installed.manifest.version);
    }

    int updatesAvailable = 0;
    for (const ExtensionMarketplaceEntry &entry : entries) {
        const auto installedIt = installedVersionById.find(entry.id);
        if (installedIt == installedVersionById.end()) {
            continue;
        }
        if (CompareLooseVersion(entry.version, installedIt->second) > 0) {
            ++updatesAvailable;
        }
    }

    if (updatesAvailable <= 0) {
        return;
    }

    statusBar()->showMessage(
        tr("%1 extension update(s) available in local marketplace").arg(updatesAvailable),
        4500);
}

void MainWindow::StartCrashRecoveryTimer() {
    if (_crashRecoveryTimer) {
        _crashRecoveryTimer->stop();
        _crashRecoveryTimer->deleteLater();
        _crashRecoveryTimer = nullptr;
    }

    _crashRecoveryTimer = new QTimer(this);
    _crashRecoveryTimer->setInterval(_editorSettings.crashRecoveryAutosaveSeconds * 1000);
    connect(_crashRecoveryTimer, &QTimer::timeout, this, [this]() {
        SaveCrashRecoveryJournal();
    });
    _crashRecoveryTimer->start();
}

void MainWindow::StartAutoSaveTimer() {
    if (_autoSaveTimer) {
        _autoSaveTimer->stop();
        _autoSaveTimer->deleteLater();
        _autoSaveTimer = nullptr;
    }

    if (!_editorSettings.autoSaveOnInterval) {
        return;
    }

    _autoSaveTimer = new QTimer(this);
    _autoSaveTimer->setInterval(std::clamp(_editorSettings.autoSaveIntervalSeconds, 5, 600) * 1000);
    connect(_autoSaveTimer, &QTimer::timeout, this, [this]() {
        const int savedCount = AutoSaveDirtyEditors();
        if (savedCount > 0) {
            statusBar()->showMessage(
                tr("Auto-saved %1 file(s).").arg(savedCount),
                1500);
        }
    });
    _autoSaveTimer->start();
}

bool MainWindow::AutoSaveEditorIfNeeded(ScintillaEditBase *editor, std::string *savedPathUtf8) {
    if (!editor) {
        return false;
    }

    const auto stateIt = _editorStates.find(editor);
    if (stateIt == _editorStates.end()) {
        return false;
    }

    const EditorState &state = stateIt->second;
    if (!state.dirty || state.filePathUtf8.empty()) {
        return false;
    }

    if (!SaveEditorToFile(editor, state.filePathUtf8)) {
        return false;
    }

    if (savedPathUtf8) {
        *savedPathUtf8 = state.filePathUtf8;
    }
    return true;
}

int MainWindow::AutoSaveDirtyEditors(std::vector<std::string> *savedPathsUtf8) {
    if (!_tabs) {
        return 0;
    }

    int savedCount = 0;
    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }

        std::string savedPathUtf8;
        if (AutoSaveEditorIfNeeded(editor, &savedPathUtf8)) {
            ++savedCount;
            if (savedPathsUtf8) {
                savedPathsUtf8->push_back(savedPathUtf8);
            }
        }
    }

    return savedCount;
}

std::string MainWindow::ConfigRootPath() const {
    const auto configPath = _pathService.GetAppPath(npp::platform::PathScope::kConfig, kAppName);
    if (!configPath.ok()) {
        return std::string{};
    }
    return *configPath.value;
}

std::string MainWindow::SkinDirectoryPath() const {
    return ConfigRootPath() + "/skins";
}

std::string MainWindow::SkinFilePathForId(const std::string &skinId) const {
    std::string fileName;
    if (skinId == "builtin.light") {
        fileName = "light.json";
    } else if (skinId == "builtin.dark") {
        fileName = "dark.json";
    } else if (skinId == "builtin.high_contrast") {
        fileName = "high-contrast.json";
    } else {
        return std::string{};
    }

    const std::vector<std::string> candidatePaths = {
        SkinDirectoryPath() + "/" + fileName,
        std::string("/usr/share/notepad-plus-plus-linux/skins/") + fileName,
        std::string("packaging/linux/skins/") + fileName,
    };

    for (const std::string &candidate : candidatePaths) {
        const auto exists = _fileSystemService.Exists(candidate);
        if (exists.ok() && *exists.value) {
            return candidate;
        }
    }

    return std::string{};
}

std::string MainWindow::SettingsFilePath() const {
    return ConfigRootPath() + "/editor-settings.json";
}

std::string MainWindow::ShortcutFilePath() const {
    return ConfigRootPath() + "/shortcuts-linux.json";
}

std::string MainWindow::ThemeFilePath() const {
    return ConfigRootPath() + "/theme-linux.json";
}

std::string MainWindow::ResolveLocalExtensionMarketplaceIndexPath() const {
    std::vector<std::string> candidatePaths;
    const std::string configRoot = ConfigRootPath();
    if (!configRoot.empty()) {
        candidatePaths.push_back(configRoot + "/extensions-marketplace-index.json");
    }

    const std::string appDirUtf8 = ToUtf8(QCoreApplication::applicationDirPath());
    if (!appDirUtf8.empty()) {
        const std::filesystem::path appDirPath(appDirUtf8);
        candidatePaths.push_back(
            (appDirPath / ".." / "share" / "notepad-plus-plus-linux" / "extensions" / "index.json")
                .lexically_normal()
                .string());
    }

    candidatePaths.push_back("/usr/local/share/notepad-plus-plus-linux/extensions/index.json");
    candidatePaths.push_back("/usr/share/notepad-plus-plus-linux/extensions/index.json");

    for (const std::string &candidatePath : candidatePaths) {
        const auto exists = _fileSystemService.Exists(candidatePath);
        if (exists.ok() && *exists.value) {
            return candidatePath;
        }
    }

    return std::string{};
}

std::string MainWindow::ResolveDefaultUpdateChannelFromInstall() const {
    std::vector<std::string> candidatePaths;

    const std::string appDirUtf8 = ToUtf8(QCoreApplication::applicationDirPath());
    if (!appDirUtf8.empty()) {
        const std::filesystem::path appDirPath(appDirUtf8);
        candidatePaths.push_back(
            (appDirPath / ".." / "share" / "notepad-plus-plus-linux" / "default-update-channel")
                .lexically_normal()
                .string());
    }

    candidatePaths.push_back("/usr/local/share/notepad-plus-plus-linux/default-update-channel");
    candidatePaths.push_back("/usr/share/notepad-plus-plus-linux/default-update-channel");

    for (const std::string &candidatePath : candidatePaths) {
        const auto exists = _fileSystemService.Exists(candidatePath);
        if (!exists.ok() || !(*exists.value)) {
            continue;
        }
        const auto channelFile = _fileSystemService.ReadTextFile(candidatePath);
        if (!channelFile.ok()) {
            continue;
        }
        return NormalizeUpdateChannel(*channelFile.value);
    }

    return "stable";
}

void MainWindow::SaveCrashRecoveryJournal() const {
    const_cast<MainWindow *>(this)->EnsureConfigRoot();

    QJsonArray tabs;
    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }
        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end() || !stateIt->second.dirty) {
            continue;
        }

        QJsonObject tab;
        tab.insert(QStringLiteral("filePath"), ToQString(stateIt->second.filePathUtf8));
        tab.insert(QStringLiteral("content"), ToQString(GetEditorText(editor)));
        tab.insert(QStringLiteral("dirty"), stateIt->second.dirty);
        tab.insert(QStringLiteral("encoding"), static_cast<int>(stateIt->second.encoding));
        tab.insert(QStringLiteral("eolMode"), stateIt->second.eolMode);
        tab.insert(QStringLiteral("lexerName"), ToQString(stateIt->second.lexerName));
        tabs.append(tab);
    }

    if (tabs.isEmpty()) {
        const_cast<MainWindow *>(this)->ClearCrashRecoveryJournal();
        return;
    }

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 1);
    root.insert(
        QStringLiteral("capturedAtUtc"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    root.insert(QStringLiteral("activeIndex"), _tabs->currentIndex());
    root.insert(QStringLiteral("tabs"), tabs);

    const QJsonDocument doc(root);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    const_cast<MainWindow *>(this)->_fileSystemService.WriteTextFile(
        CrashRecoveryJournalPath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

void MainWindow::ClearCrashRecoveryJournal() const {
    const auto exists = _fileSystemService.Exists(CrashRecoveryJournalPath());
    if (!exists.ok() || !(*exists.value)) {
        return;
    }
    const_cast<MainWindow *>(this)->_fileSystemService.RemoveFile(CrashRecoveryJournalPath());
}

bool MainWindow::RestoreCrashRecoveryJournal() {
    EnsureConfigRoot();
    const auto exists = _fileSystemService.Exists(CrashRecoveryJournalPath());
    if (!exists.ok() || !(*exists.value)) {
        return false;
    }

    const auto journalJson = _fileSystemService.ReadTextFile(CrashRecoveryJournalPath());
    if (!journalJson.ok()) {
        return false;
    }

    QJsonParseError parseError{};
    const QByteArray jsonBytes(journalJson.value->c_str(), static_cast<int>(journalJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        ClearCrashRecoveryJournal();
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray tabs = root.value(QStringLiteral("tabs")).toArray();
    if (tabs.isEmpty()) {
        ClearCrashRecoveryJournal();
        return false;
    }

    const QString capturedAt = root.value(QStringLiteral("capturedAtUtc")).toString();
    const auto recoverChoice = QMessageBox::question(
        this,
        tr("Recover Unsaved Changes"),
        tr("Unsaved content journal found%1 with %2 tab(s).\nRecover now?")
            .arg(capturedAt.isEmpty() ? QString{} : tr(" (%1)").arg(capturedAt))
            .arg(tabs.size()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    if (recoverChoice != QMessageBox::Yes) {
        ClearCrashRecoveryJournal();
        return false;
    }

    int restored = 0;
    for (const QJsonValue &tabValue : tabs) {
        if (!tabValue.isObject()) {
            continue;
        }
        const QJsonObject tabObj = tabValue.toObject();
        const std::string contentUtf8 = ToUtf8(tabObj.value(QStringLiteral("content")).toString());

        NewTab();
        ScintillaEditBase *editor = CurrentEditor();
        if (!editor) {
            continue;
        }

        auto &state = _editorStates[editor];
        state.filePathUtf8 = ToUtf8(tabObj.value(QStringLiteral("filePath")).toString());
        state.dirty = tabObj.value(QStringLiteral("dirty")).toBool(true);
        const int rawEncoding = tabObj.value(QStringLiteral("encoding")).toInt(static_cast<int>(TextEncoding::kUtf8));
        const int boundedEncoding = std::clamp(rawEncoding, 0, static_cast<int>(TextEncoding::kLocal8Bit));
        state.encoding = static_cast<TextEncoding>(boundedEncoding);
        state.eolMode = tabObj.value(QStringLiteral("eolMode")).toInt(SC_EOL_LF);
        state.lexerName = ToUtf8(tabObj.value(QStringLiteral("lexerName")).toString());
        state.lexerManualLock = false;

        SetEditorText(editor, contentUtf8);
        SetEditorEolMode(editor, state.eolMode);
        if (!state.lexerName.empty()) {
            ApplyLexerByName(editor, state.lexerName);
        } else {
            AutoDetectAndApplyLexer(editor, state.filePathUtf8, contentUtf8, "crash-recovery");
        }
        editor->send(SCI_SETSAVEPOINT);
        UpdateTabTitle(editor);
        ++restored;
    }

    if (restored <= 0) {
        return false;
    }

    const int activeIndex = root.value(QStringLiteral("activeIndex")).toInt(0);
    const int boundedIndex = std::clamp(activeIndex, 0, _tabs->count() - 1);
    _tabs->setCurrentIndex(boundedIndex);
    statusBar()->showMessage(tr("Recovered %1 tab(s) from crash journal").arg(restored), 4000);
    ClearCrashRecoveryJournal();
    return true;
}

bool MainWindow::RestoreSession() {
    EnsureConfigRoot();
    const std::string sessionPath = SessionFilePath();
    const auto exists = _fileSystemService.Exists(sessionPath);
    if (!exists.ok() || !(*exists.value)) {
        return false;
    }

    const auto sessionJson = _fileSystemService.ReadTextFile(sessionPath);
    if (!sessionJson.ok()) {
        return false;
    }

    QJsonParseError parseError{};
    const QByteArray jsonBytes(sessionJson.value->c_str(), static_cast<int>(sessionJson.value->size()));
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray files = root.value(QStringLiteral("openFiles")).toArray();
    if (files.isEmpty()) {
        return false;
    }

    int opened = 0;
    for (const QJsonValue &fileValue : files) {
        if (!fileValue.isString()) {
            continue;
        }

        const std::string pathUtf8 = ToUtf8(fileValue.toString());
        if (pathUtf8.empty()) {
            continue;
        }

        const auto fileExists = _fileSystemService.Exists(pathUtf8);
        if (!fileExists.ok() || !(*fileExists.value)) {
            continue;
        }

        NewTab();
        ScintillaEditBase *editor = CurrentEditor();
        if (editor && LoadFileIntoEditor(editor, pathUtf8)) {
            ++opened;
            continue;
        }

        const int currentIndex = _tabs->currentIndex();
        if (currentIndex >= 0 && editor) {
            _tabs->removeTab(currentIndex);
            _editorStates.erase(editor);
            editor->deleteLater();
        }
    }

    if (opened <= 0) {
        return false;
    }

    const int activeIndex = root.value(QStringLiteral("activeIndex")).toInt(0);
    const int boundedIndex = std::clamp(activeIndex, 0, _tabs->count() - 1);
    _tabs->setCurrentIndex(boundedIndex);
    return true;
}

std::string MainWindow::DetermineProjectSessionRootFromOpenTabs() const {
    std::vector<std::filesystem::path> parentDirectories;
    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }

        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end() || stateIt->second.filePathUtf8.empty()) {
            continue;
        }

        std::filesystem::path parentPath = std::filesystem::path(stateIt->second.filePathUtf8).parent_path();
        if (parentPath.empty()) {
            continue;
        }
        parentDirectories.push_back(parentPath.lexically_normal());
    }

    if (parentDirectories.empty()) {
        return std::string{};
    }

    std::vector<std::string> commonComponents;
    for (const auto &component : parentDirectories.front()) {
        commonComponents.push_back(component.string());
    }

    for (size_t pathIndex = 1; pathIndex < parentDirectories.size(); ++pathIndex) {
        std::vector<std::string> currentComponents;
        for (const auto &component : parentDirectories[pathIndex]) {
            currentComponents.push_back(component.string());
        }

        const size_t maxCommon = std::min(commonComponents.size(), currentComponents.size());
        size_t matchCount = 0;
        while (matchCount < maxCommon && commonComponents[matchCount] == currentComponents[matchCount]) {
            ++matchCount;
        }
        commonComponents.resize(matchCount);
        if (commonComponents.empty()) {
            return std::string{};
        }
    }

    std::filesystem::path commonPath;
    for (const std::string &component : commonComponents) {
        commonPath /= component;
    }
    return commonPath.string();
}

void MainWindow::SaveSession() const {
    MainWindow *self = const_cast<MainWindow *>(this);
    self->EnsureConfigRoot();

    QJsonArray files;
    for (int index = 0; index < _tabs->count(); ++index) {
        ScintillaEditBase *editor = EditorAt(index);
        if (!editor) {
            continue;
        }
        const auto stateIt = _editorStates.find(editor);
        if (stateIt == _editorStates.end() || stateIt->second.filePathUtf8.empty()) {
            continue;
        }
        files.append(ToQString(stateIt->second.filePathUtf8));
    }

    if (_editorSettings.usePerProjectSessionStorage) {
        const std::string projectRoot = DetermineProjectSessionRootFromOpenTabs();
        if (!projectRoot.empty() && projectRoot != _editorSettings.lastProjectSessionRootUtf8) {
            self->_editorSettings.lastProjectSessionRootUtf8 = projectRoot;
            self->SaveEditorSettings();
        }
    }

    QJsonObject root;
    root.insert(QStringLiteral("openFiles"), files);
    root.insert(QStringLiteral("activeIndex"), _tabs->currentIndex());
    const QJsonDocument doc(root);
    const QByteArray json = doc.toJson(QJsonDocument::Indented);

    npp::platform::WriteFileOptions options;
    options.atomic = true;
    options.createParentDirs = true;
    self->_fileSystemService.WriteTextFile(
        SessionFilePath(),
        std::string(json.constData(), static_cast<size_t>(json.size())),
        options);
}

std::string MainWindow::SessionFilePath() const {
    if (_editorSettings.usePerProjectSessionStorage) {
        const std::string projectRoot = _editorSettings.lastProjectSessionRootUtf8;
        if (!projectRoot.empty()) {
            const size_t projectHash = std::hash<std::string>{}(projectRoot);
            std::ostringstream hashStream;
            hashStream << std::hex << projectHash;

            std::string projectName = std::filesystem::path(projectRoot).filename().string();
            if (projectName.empty() || projectName == "/" || projectName == ".") {
                projectName = "root";
            }

            return ConfigRootPath() + "/sessions/" + projectName + "-" + hashStream.str() + ".json";
        }
    }
    return ConfigRootPath() + "/session.json";
}

std::string MainWindow::CrashRecoveryJournalPath() const {
    return ConfigRootPath() + "/crash-recovery-journal.json";
}

QString MainWindow::EncodingLabel(TextEncoding encoding) {
    switch (encoding) {
        case TextEncoding::kUtf8:
            return QStringLiteral("UTF-8");
        case TextEncoding::kUtf8Bom:
            return QStringLiteral("UTF-8 with BOM");
        case TextEncoding::kUtf16Le:
            return QStringLiteral("UTF-16 LE");
        case TextEncoding::kUtf16Be:
            return QStringLiteral("UTF-16 BE");
        case TextEncoding::kLocal8Bit:
            return QStringLiteral("Local 8-bit");
    }
    return QStringLiteral("UTF-8");
}

std::string MainWindow::ToUtf8(const QString &value) {
    return value.toUtf8().toStdString();
}

QString MainWindow::ToQString(const std::string &value) {
    return QString::fromUtf8(value.c_str(), static_cast<int>(value.size()));
}
