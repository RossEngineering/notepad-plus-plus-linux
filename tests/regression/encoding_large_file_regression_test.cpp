#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "ScintillaTypes.h"
#include "ILoader.h"
#include "ILexer.h"
#include "json.hpp"

#include "Debugging.h"

#include "CharacterCategoryMap.h"
#include "Position.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "CellBuffer.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"

namespace {

using Json = nlohmann::json;

bool Expect(bool condition, const char *context) {
    if (!condition) {
        std::cerr << "regression check failed: " << context << "\n";
        return false;
    }
    return true;
}

std::string ReadDocumentText(const Scintilla::Internal::Document &doc) {
    const auto length = static_cast<size_t>(doc.LengthNoExcept());
    std::string out(length, '\0');
    doc.GetCharRange(out.data(), 0, static_cast<Sci::Position>(length));
    return out;
}

std::optional<std::string> ReadFileText(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::nullopt;
    }
    return std::string(
        (std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());
}

std::optional<std::string> DecodeHex(std::string_view hex) {
    if ((hex.size() % 2U) != 0U) {
        return std::nullopt;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return 10 + (c - 'a');
        }
        if (c >= 'A' && c <= 'F') {
            return 10 + (c - 'A');
        }
        return -1;
    };

    std::string bytes;
    bytes.reserve(hex.size() / 2U);
    for (size_t i = 0; i < hex.size(); i += 2U) {
        const int high = nibble(hex[i]);
        const int low = nibble(hex[i + 1U]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes.push_back(static_cast<char>((high << 4) | low));
    }
    return bytes;
}

}  // namespace

int main() {
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;
    using Scintilla::Internal::FixInvalidUTF8;
    using Scintilla::Internal::UTF8FromUTF16;
    using Scintilla::Internal::UTF8IsValid;
    using Scintilla::Internal::UTF8Length;
    using Scintilla::Internal::UTF16FromUTF8;
    using Scintilla::Internal::UTF16Length;

    const std::filesystem::path sourceRoot = std::filesystem::path(NPP_SOURCE_DIR);
    const std::filesystem::path mixedEncodingFixturePath =
        sourceRoot / "tests/fixtures/encoding-large-file/mixed-encoding-cases.json";
    const std::filesystem::path largePatternFixturePath =
        sourceRoot / "tests/fixtures/encoding-large-file/very-large-file-pattern.txt";

    const auto mixedEncodingFixture = ReadFileText(mixedEncodingFixturePath);
    if (!Expect(mixedEncodingFixture.has_value(), "load mixed-encoding fixture")) {
        return 1;
    }
    const Json mixedCases = Json::parse(*mixedEncodingFixture, nullptr, false);
    if (!Expect(!mixedCases.is_discarded() && mixedCases.is_object(), "parse mixed-encoding fixture")) {
        return 2;
    }
    if (!Expect(mixedCases.value("schemaVersion", 0) == 1, "mixed-encoding schema version")) {
        return 3;
    }

    const Json cases = mixedCases.value("cases", Json::array());
    if (!Expect(cases.is_array() && !cases.empty(), "mixed-encoding cases array")) {
        return 4;
    }
    for (const Json& fixtureCase : cases) {
        const std::string name = fixtureCase.value("name", std::string{});
        const std::string expectMode = fixtureCase.value("expect", std::string{});
        const std::string bytesHex = fixtureCase.value("bytesHex", std::string{});
        const auto bytes = DecodeHex(bytesHex);
        if (!Expect(bytes.has_value(), ("decode fixture hex " + name).c_str())) {
            return 5;
        }
        if (expectMode == "valid_utf8") {
            if (!Expect(UTF8IsValid(*bytes), ("fixture expected valid utf8: " + name).c_str())) {
                return 6;
            }
        } else if (expectMode == "invalid_utf8_fixable") {
            if (!Expect(!UTF8IsValid(*bytes), ("fixture expected invalid utf8: " + name).c_str())) {
                return 7;
            }
            const std::string fixed = FixInvalidUTF8(*bytes);
            if (!Expect(UTF8IsValid(fixed), ("fixture fixed utf8 should be valid: " + name).c_str())) {
                return 8;
            }
        } else {
            if (!Expect(false, ("unknown fixture expectation mode: " + name).c_str())) {
                return 9;
            }
        }
    }

    const std::string utf8Text = u8"Grusse - Salut - Hello";
    if (!Expect(UTF8IsValid(utf8Text), "valid UTF-8 should be accepted")) {
        return 10;
    }

    const size_t utf16Length = UTF16Length(utf8Text);
    std::vector<wchar_t> utf16Buffer(utf16Length);
    const size_t utf16Written = UTF16FromUTF8(utf8Text, utf16Buffer.data(), utf16Buffer.size());
    if (!Expect(utf16Written == utf16Length, "UTF-16 conversion length mismatch")) {
        return 11;
    }

    const size_t utf8RoundTripLength = UTF8Length(std::wstring_view(utf16Buffer.data(), utf16Written));
    std::vector<char> utf8RoundTrip(utf8RoundTripLength);
    UTF8FromUTF16(
        std::wstring_view(utf16Buffer.data(), utf16Written),
        utf8RoundTrip.data(),
        utf8RoundTrip.size());
    const std::string roundTrip(utf8RoundTrip.begin(), utf8RoundTrip.end());
    if (!Expect(roundTrip == utf8Text, "UTF-8 -> UTF-16 -> UTF-8 round trip mismatch")) {
        return 12;
    }

    const std::string invalidUtf8 = std::string("ok") + static_cast<char>(0xC3) + '(';
    if (!Expect(!UTF8IsValid(invalidUtf8), "invalid UTF-8 should be rejected")) {
        return 13;
    }

    const std::string fixedUtf8 = FixInvalidUTF8(invalidUtf8);
    if (!Expect(UTF8IsValid(fixedUtf8), "fixed UTF-8 should be valid")) {
        return 14;
    }

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());

    const auto largePatternFixture = ReadFileText(largePatternFixturePath);
    if (!Expect(largePatternFixture.has_value() && !largePatternFixture->empty(), "load large-file pattern fixture")) {
        return 15;
    }

    constexpr size_t kLargeBufferSize = 3 * 1024 * 1024;
    std::string largeBuffer;
    largeBuffer.reserve(kLargeBufferSize);
    while (largeBuffer.size() < kLargeBufferSize) {
        largeBuffer.append(*largePatternFixture);
        largeBuffer.push_back('\n');
    }
    largeBuffer.resize(kLargeBufferSize);
    largeBuffer[0] = 'L';
    largeBuffer[kLargeBufferSize - 1] = 'Z';

    doc.BeginUndoAction();
    if (!Expect(doc.InsertString(0, largeBuffer) >= 0, "insert large buffer")) {
        return 16;
    }
    doc.EndUndoAction();
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize),
            "large buffer length check")) {
        return 17;
    }

    if (!Expect(doc.InsertString(doc.LengthNoExcept(), "\nTAIL", 5) >= 0, "append tail marker")) {
        return 18;
    }
    if (!Expect(doc.CharAt(doc.LengthNoExcept() - 1) == 'L', "tail marker char check")) {
        return 19;
    }

    if (!Expect(doc.Undo() >= 0, "undo tail append")) {
        return 20;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize),
            "length after undo tail append")) {
        return 21;
    }

    if (!Expect(doc.Redo() >= 0, "redo tail append")) {
        return 22;
    }
    if (!Expect(ReadDocumentText(doc).substr(kLargeBufferSize - 1, 6) == "Z\nTAIL", "tail content check")) {
        return 23;
    }

    constexpr Sci::Position kDeleteLength = 1024;
    if (!Expect(doc.DeleteChars(0, kDeleteLength), "delete initial chunk")) {
        return 24;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize + 5 - kDeleteLength),
            "length after delete")) {
        return 25;
    }

    if (!Expect(doc.Undo() >= 0, "undo large delete")) {
        return 26;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize + 5),
            "length after undo large delete")) {
        return 27;
    }

    return 0;
}
