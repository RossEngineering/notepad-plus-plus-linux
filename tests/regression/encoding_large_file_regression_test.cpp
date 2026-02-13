#include <algorithm>
#include <cstddef>
#include <cstdint>
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

    const std::string utf8Text = u8"Grusse - Salut - Hello";
    if (!Expect(UTF8IsValid(utf8Text), "valid UTF-8 should be accepted")) {
        return 1;
    }

    const size_t utf16Length = UTF16Length(utf8Text);
    std::vector<wchar_t> utf16Buffer(utf16Length);
    const size_t utf16Written = UTF16FromUTF8(utf8Text, utf16Buffer.data(), utf16Buffer.size());
    if (!Expect(utf16Written == utf16Length, "UTF-16 conversion length mismatch")) {
        return 2;
    }

    const size_t utf8RoundTripLength = UTF8Length(std::wstring_view(utf16Buffer.data(), utf16Written));
    std::vector<char> utf8RoundTrip(utf8RoundTripLength);
    UTF8FromUTF16(
        std::wstring_view(utf16Buffer.data(), utf16Written),
        utf8RoundTrip.data(),
        utf8RoundTrip.size());
    const std::string roundTrip(utf8RoundTrip.begin(), utf8RoundTrip.end());
    if (!Expect(roundTrip == utf8Text, "UTF-8 -> UTF-16 -> UTF-8 round trip mismatch")) {
        return 3;
    }

    const std::string invalidUtf8 = std::string("ok") + static_cast<char>(0xC3) + '(';
    if (!Expect(!UTF8IsValid(invalidUtf8), "invalid UTF-8 should be rejected")) {
        return 4;
    }

    const std::string fixedUtf8 = FixInvalidUTF8(invalidUtf8);
    if (!Expect(UTF8IsValid(fixedUtf8), "fixed UTF-8 should be valid")) {
        return 5;
    }

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());

    constexpr size_t kLargeBufferSize = 2 * 1024 * 1024;
    std::string largeBuffer(kLargeBufferSize, 'a');
    largeBuffer[0] = 'L';
    largeBuffer[kLargeBufferSize - 1] = 'Z';

    doc.BeginUndoAction();
    if (!Expect(doc.InsertString(0, largeBuffer) >= 0, "insert large buffer")) {
        return 6;
    }
    doc.EndUndoAction();
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize),
            "large buffer length check")) {
        return 7;
    }

    if (!Expect(doc.InsertString(doc.LengthNoExcept(), "\nTAIL", 5) >= 0, "append tail marker")) {
        return 8;
    }
    if (!Expect(doc.CharAt(doc.LengthNoExcept() - 1) == 'L', "tail marker char check")) {
        return 9;
    }

    if (!Expect(doc.Undo() >= 0, "undo tail append")) {
        return 10;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize),
            "length after undo tail append")) {
        return 11;
    }

    if (!Expect(doc.Redo() >= 0, "redo tail append")) {
        return 12;
    }
    if (!Expect(ReadDocumentText(doc).substr(kLargeBufferSize - 1, 6) == "Z\nTAIL", "tail content check")) {
        return 13;
    }

    constexpr Sci::Position kDeleteLength = 1024;
    if (!Expect(doc.DeleteChars(0, kDeleteLength), "delete initial chunk")) {
        return 14;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize + 5 - kDeleteLength),
            "length after delete")) {
        return 15;
    }

    if (!Expect(doc.Undo() >= 0, "undo large delete")) {
        return 16;
    }
    if (!Expect(
            doc.LengthNoExcept() == static_cast<Sci::Position>(kLargeBufferSize + 5),
            "length after undo large delete")) {
        return 17;
    }

    return 0;
}
