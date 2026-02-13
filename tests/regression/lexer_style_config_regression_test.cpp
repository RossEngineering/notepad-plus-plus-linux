#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "LexerStyleConfig.h"

namespace {

bool HasRole(
    const std::vector<npp::ui::LexerStyleRule>& rules,
    npp::ui::LexerStyleColorRole role) {
    for (const auto& rule : rules) {
        if (rule.colorRole == role) {
            return true;
        }
    }
    return false;
}

bool ValidateNoDuplicateStyleIds(const std::vector<npp::ui::LexerStyleRule>& rules) {
    std::set<int> seen;
    for (const auto& rule : rules) {
        if (!seen.insert(rule.styleId).second) {
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    struct ExpectedStyledLexer {
        const char* lexerName;
        bool requiresKeyword;
        bool requiresString;
        bool requiresComment;
    };

    const std::vector<ExpectedStyledLexer> expectedLexers = {
        {"cpp", true, true, true},
        {"json", true, true, true},
        {"xml", true, true, true},
        {"markdown", true, true, true},
        {"python", true, true, true},
        {"bash", true, true, true},
        {"yaml", true, false, true},
        {"sql", true, true, true},
    };

    for (const auto& expected : expectedLexers) {
        if (!npp::ui::HasLexerStyleRules(expected.lexerName)) {
            std::cerr << "missing style rules for lexer: " << expected.lexerName << "\n";
            return 1;
        }
        const auto& rules = npp::ui::LexerStyleRulesFor(expected.lexerName);
        if (rules.empty()) {
            std::cerr << "empty style rules for lexer: " << expected.lexerName << "\n";
            return 2;
        }
        if (!ValidateNoDuplicateStyleIds(rules)) {
            std::cerr << "duplicate style ids in lexer mapping: " << expected.lexerName << "\n";
            return 3;
        }
        if (expected.requiresKeyword && !HasRole(rules, npp::ui::LexerStyleColorRole::kKeyword)) {
            std::cerr << "missing keyword style role in lexer mapping: " << expected.lexerName << "\n";
            return 4;
        }
        if (expected.requiresString && !HasRole(rules, npp::ui::LexerStyleColorRole::kString)) {
            std::cerr << "missing string style role in lexer mapping: " << expected.lexerName << "\n";
            return 5;
        }
        if (expected.requiresComment && !HasRole(rules, npp::ui::LexerStyleColorRole::kComment)) {
            std::cerr << "missing comment style role in lexer mapping: " << expected.lexerName << "\n";
            return 6;
        }
    }

    if (npp::ui::HasLexerStyleRules("null")) {
        std::cerr << "plain text lexer unexpectedly has custom style rules\n";
        return 7;
    }

    return 0;
}
