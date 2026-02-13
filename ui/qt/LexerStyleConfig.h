#pragma once

#include <string>
#include <vector>

#include "SciLexer.h"

namespace npp::ui {

enum class LexerStyleColorRole {
    kForeground = 0,
    kComment,
    kNumber,
    kKeyword,
    kString,
    kOperator,
};

struct LexerStyleRule {
    int styleId = 0;
    LexerStyleColorRole colorRole = LexerStyleColorRole::kForeground;
    bool bold = false;
};

inline const std::vector<LexerStyleRule>& LexerStyleRulesFor(const std::string& lexerName) {
    static const std::vector<LexerStyleRule> kNone = {};
    static const std::vector<LexerStyleRule> kCppLike = {
        {SCE_C_COMMENT, LexerStyleColorRole::kComment, false},
        {SCE_C_COMMENTLINE, LexerStyleColorRole::kComment, false},
        {SCE_C_COMMENTDOC, LexerStyleColorRole::kComment, false},
        {SCE_C_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_C_WORD, LexerStyleColorRole::kKeyword, true},
        {SCE_C_STRING, LexerStyleColorRole::kString, false},
        {SCE_C_CHARACTER, LexerStyleColorRole::kString, false},
        {SCE_C_OPERATOR, LexerStyleColorRole::kOperator, false},
        {SCE_C_PREPROCESSOR, LexerStyleColorRole::kKeyword, false},
    };
    static const std::vector<LexerStyleRule> kXml = {
        {SCE_H_TAG, LexerStyleColorRole::kKeyword, false},
        {SCE_H_TAGUNKNOWN, LexerStyleColorRole::kKeyword, false},
        {SCE_H_ATTRIBUTE, LexerStyleColorRole::kForeground, false},
        {SCE_H_ATTRIBUTEUNKNOWN, LexerStyleColorRole::kForeground, false},
        {SCE_H_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_H_DOUBLESTRING, LexerStyleColorRole::kString, false},
        {SCE_H_SINGLESTRING, LexerStyleColorRole::kString, false},
        {SCE_H_COMMENT, LexerStyleColorRole::kComment, false},
        {SCE_H_ENTITY, LexerStyleColorRole::kNumber, false},
        {SCE_H_CDATA, LexerStyleColorRole::kString, false},
    };
    static const std::vector<LexerStyleRule> kMarkdown = {
        {SCE_MARKDOWN_HEADER1, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_HEADER2, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_HEADER3, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_HEADER4, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_HEADER5, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_HEADER6, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_STRONG1, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_STRONG2, LexerStyleColorRole::kKeyword, true},
        {SCE_MARKDOWN_EM1, LexerStyleColorRole::kOperator, false},
        {SCE_MARKDOWN_EM2, LexerStyleColorRole::kOperator, false},
        {SCE_MARKDOWN_LINK, LexerStyleColorRole::kKeyword, false},
        {SCE_MARKDOWN_CODE, LexerStyleColorRole::kString, false},
        {SCE_MARKDOWN_CODE2, LexerStyleColorRole::kString, false},
        {SCE_MARKDOWN_CODEBK, LexerStyleColorRole::kString, false},
        {SCE_MARKDOWN_BLOCKQUOTE, LexerStyleColorRole::kComment, false},
        {SCE_MARKDOWN_HRULE, LexerStyleColorRole::kOperator, false},
    };
    static const std::vector<LexerStyleRule> kPython = {
        {SCE_P_COMMENTLINE, LexerStyleColorRole::kComment, false},
        {SCE_P_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_P_WORD, LexerStyleColorRole::kKeyword, true},
        {SCE_P_STRING, LexerStyleColorRole::kString, false},
        {SCE_P_CHARACTER, LexerStyleColorRole::kString, false},
        {SCE_P_OPERATOR, LexerStyleColorRole::kOperator, false},
        {SCE_P_DEFNAME, LexerStyleColorRole::kKeyword, false},
        {SCE_P_CLASSNAME, LexerStyleColorRole::kKeyword, false},
    };
    static const std::vector<LexerStyleRule> kBash = {
        {SCE_SH_COMMENTLINE, LexerStyleColorRole::kComment, false},
        {SCE_SH_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_SH_WORD, LexerStyleColorRole::kKeyword, true},
        {SCE_SH_STRING, LexerStyleColorRole::kString, false},
        {SCE_SH_OPERATOR, LexerStyleColorRole::kOperator, false},
    };
    static const std::vector<LexerStyleRule> kYaml = {
        {SCE_YAML_COMMENT, LexerStyleColorRole::kComment, false},
        {SCE_YAML_IDENTIFIER, LexerStyleColorRole::kForeground, false},
        {SCE_YAML_KEYWORD, LexerStyleColorRole::kKeyword, true},
        {SCE_YAML_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_YAML_REFERENCE, LexerStyleColorRole::kOperator, false},
        {SCE_YAML_DOCUMENT, LexerStyleColorRole::kKeyword, false},
        {SCE_YAML_TEXT, LexerStyleColorRole::kForeground, false},
        {SCE_YAML_OPERATOR, LexerStyleColorRole::kOperator, false},
    };
    static const std::vector<LexerStyleRule> kSql = {
        {SCE_SQL_COMMENT, LexerStyleColorRole::kComment, false},
        {SCE_SQL_COMMENTLINE, LexerStyleColorRole::kComment, false},
        {SCE_SQL_COMMENTDOC, LexerStyleColorRole::kComment, false},
        {SCE_SQL_COMMENTLINEDOC, LexerStyleColorRole::kComment, false},
        {SCE_SQL_COMMENTDOCKEYWORD, LexerStyleColorRole::kComment, false},
        {SCE_SQL_COMMENTDOCKEYWORDERROR, LexerStyleColorRole::kComment, false},
        {SCE_SQL_NUMBER, LexerStyleColorRole::kNumber, false},
        {SCE_SQL_WORD, LexerStyleColorRole::kKeyword, true},
        {SCE_SQL_WORD2, LexerStyleColorRole::kKeyword, true},
        {SCE_SQL_STRING, LexerStyleColorRole::kString, false},
        {SCE_SQL_CHARACTER, LexerStyleColorRole::kString, false},
        {SCE_SQL_QUOTEDIDENTIFIER, LexerStyleColorRole::kString, false},
        {SCE_SQL_OPERATOR, LexerStyleColorRole::kOperator, false},
        {SCE_SQL_QOPERATOR, LexerStyleColorRole::kOperator, false},
        {SCE_SQL_IDENTIFIER, LexerStyleColorRole::kForeground, false},
    };

    if (lexerName == "cpp" || lexerName == "json") {
        return kCppLike;
    }
    if (lexerName == "xml") {
        return kXml;
    }
    if (lexerName == "markdown") {
        return kMarkdown;
    }
    if (lexerName == "python") {
        return kPython;
    }
    if (lexerName == "bash") {
        return kBash;
    }
    if (lexerName == "yaml") {
        return kYaml;
    }
    if (lexerName == "sql") {
        return kSql;
    }
    return kNone;
}

inline bool HasLexerStyleRules(const std::string& lexerName) {
    return !LexerStyleRulesFor(lexerName).empty();
}

}  // namespace npp::ui
