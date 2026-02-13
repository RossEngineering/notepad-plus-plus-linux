#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <memory>

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

namespace {

std::string ReadDocumentText(const Scintilla::Internal::Document &doc) {
    const auto length = static_cast<size_t>(doc.LengthNoExcept());
    std::string out;
    out.resize(length);
    for (size_t i = 0; i < length; ++i) {
        out[i] = doc.CharAt(static_cast<Sci::Position>(i));
    }
    return out;
}

bool ExpectText(
    const Scintilla::Internal::Document &doc,
    const std::string &expected,
    const char *context) {
    const std::string actual = ReadDocumentText(doc);
    if (actual != expected) {
        std::cerr << context << " expected [" << expected << "] got [" << actual << "]\n";
        return false;
    }
    return true;
}

}  // namespace

int main() {
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::Document;

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<Scintilla::Internal::CaseFolderUnicode>());

    if (doc.InsertString(0, "hello", 5) < 0) {
        std::cerr << "insert hello failed\n";
        return 1;
    }
    if (doc.InsertString(5, " world", 6) < 0) {
        std::cerr << "insert world failed\n";
        return 2;
    }
    if (!ExpectText(doc, "hello world", "after inserts")) {
        return 3;
    }

    if (!doc.DeleteChars(5, 1)) {
        std::cerr << "delete space failed\n";
        return 4;
    }
    if (!ExpectText(doc, "helloworld", "after delete")) {
        return 5;
    }

    if (doc.Undo() < 0) {
        std::cerr << "undo delete failed\n";
        return 6;
    }
    if (!ExpectText(doc, "hello world", "after undo delete")) {
        return 7;
    }

    if (doc.Undo() < 0) {
        std::cerr << "undo second insert failed\n";
        return 8;
    }
    if (!ExpectText(doc, "", "after undo second insert")) {
        return 9;
    }

    if (doc.Redo() < 0) {
        std::cerr << "redo second insert failed\n";
        return 10;
    }
    if (!ExpectText(doc, "hello world", "after redo second insert")) {
        return 11;
    }

    doc.BeginUndoAction();
    doc.InsertString(doc.LengthNoExcept(), "!", 1);
    doc.InsertString(doc.LengthNoExcept(), "?", 1);
    doc.EndUndoAction();
    if (!ExpectText(doc, "hello world!?", "after grouped edits")) {
        return 12;
    }

    if (doc.Undo() < 0) {
        std::cerr << "grouped undo failed\n";
        return 13;
    }
    if (!ExpectText(doc, "hello world", "after grouped undo")) {
        return 14;
    }

    if (doc.Redo() < 0) {
        std::cerr << "grouped redo failed\n";
        return 15;
    }
    if (!ExpectText(doc, "hello world!?", "after grouped redo")) {
        return 16;
    }

    return 0;
}
