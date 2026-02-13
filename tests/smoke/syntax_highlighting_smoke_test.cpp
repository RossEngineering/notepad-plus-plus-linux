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

#include "Lexilla.h"

namespace {

struct LexerReleaser {
    void operator()(Scintilla::ILexer5 *lexer) const noexcept {
        if (lexer != nullptr) {
            lexer->Release();
        }
    }
};

bool Expect(bool condition, const char *context, const char *language) {
    if (!condition) {
        std::cerr << "syntax smoke failed for " << language << ": " << context << "\n";
        return false;
    }
    return true;
}

bool RunLexerCase(const char *language, std::string_view sampleText) {
    using Scintilla::CpUtf8;
    using Scintilla::DocumentOption;
    using Scintilla::Internal::CaseFolderUnicode;
    using Scintilla::Internal::Document;

    std::unique_ptr<Scintilla::ILexer5, LexerReleaser> lexer(CreateLexer(language));
    if (!Expect(lexer != nullptr, "lexer creation", language)) {
        return false;
    }

    Document doc(DocumentOption::Default);
    doc.SetDBCSCodePage(CpUtf8);
    doc.SetCaseFolder(std::make_unique<CaseFolderUnicode>());

    if (!Expect(doc.InsertString(0, sampleText) >= 0, "document insert", language)) {
        return false;
    }

    const Sci::Position length = doc.LengthNoExcept();
    lexer->Lex(0, length, 0, &doc);
    lexer->Fold(0, length, 0, &doc);

    std::vector<unsigned char> styles(static_cast<size_t>(length), 0);
    doc.GetStyleRange(styles.data(), 0, length);

    const bool hasNonDefaultStyle =
        std::any_of(styles.begin(), styles.end(), [](unsigned char style) { return style != 0; });
    return Expect(hasNonDefaultStyle, "expected at least one non-default style", language);
}

}  // namespace

int main() {
    if (!RunLexerCase("cpp", "int main() { return 42; }\n")) {
        return 1;
    }
    if (!RunLexerCase("python", "def add(x, y):\n    return x + y\n")) {
        return 2;
    }
    if (!RunLexerCase("json", "{ \"name\": \"npp\", \"enabled\": true }\n")) {
        return 3;
    }
    if (!RunLexerCase("bash", "#!/bin/bash\necho hello\n")) {
        return 4;
    }
    return 0;
}
