#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "Debugging.h"

namespace Scintilla::Internal::Platform {

void DebugDisplay(const char *s) noexcept {
    if (s != nullptr) {
        std::fputs(s, stderr);
    }
}

void DebugPrintf(const char *format, ...) noexcept {
    if (format == nullptr) {
        return;
    }
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
}

bool ShowAssertionPopUps(bool assertionPopUps_) noexcept {
    return assertionPopUps_;
}

void Assert(const char *condition, const char *file, int line) noexcept {
    std::fprintf(stderr, "Scintilla assert failed: %s (%s:%d)\n", condition, file, line);
    std::abort();
}

}  // namespace Scintilla::Internal::Platform
