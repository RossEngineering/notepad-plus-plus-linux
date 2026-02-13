#include "Document.h"

int main() {
	using namespace Scintilla;
	using namespace Scintilla::Internal;

	Document doc(DocumentOption::Default);
	const char *text = "abc";
	if (doc.InsertString(0, text, 3) != 3) {
		return 1;
	}
	if (doc.LengthNoExcept() != 3) {
		return 2;
	}
	if (!doc.DeleteChars(1, 1)) {
		return 3;
	}
	if (doc.LengthNoExcept() != 2) {
		return 4;
	}
	return 0;
}
