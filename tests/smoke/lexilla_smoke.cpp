#include "ILexer.h"
#include "Lexilla.h"

int main() {
	Scintilla::ILexer5 *lexer = CreateLexer("cpp");
	if (!lexer) {
		return 1;
	}
	lexer->Release();
	return 0;
}
