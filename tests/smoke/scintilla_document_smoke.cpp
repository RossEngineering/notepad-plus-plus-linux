#include <cstddef>
#include <string_view>

#include "UniConversion.h"

int main() {
	using Scintilla::Internal::UTF16Length;
	using Scintilla::Internal::UTF8IsValid;
	if (!UTF8IsValid(std::string_view{"hello"})) {
		return 2;
	}
	if (UTF16Length(std::string_view{"hello"}) != 5) {
		return 3;
	}
	return 0;
}
