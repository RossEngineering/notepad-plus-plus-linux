#pragma once

#include <string>

#include "Types.h"

namespace npp::platform {

enum class ThemeMode {
    kSystem = 0,
    kLight,
    kDark,
};

enum class ThemeColorRole {
    kWindowBackground = 0,
    kWindowForeground,
    kEditorBackground,
    kEditorForeground,
    kAccent,
    kError,
};

class IThemeService {
public:
    virtual ~IThemeService() = default;

    virtual ThemeMode GetPreferredMode() const = 0;
    virtual Status SetPreferredMode(ThemeMode mode) = 0;
    virtual ThemeMode GetResolvedMode() const = 0;
    virtual StatusOr<RgbaColor> GetColor(ThemeColorRole role) const = 0;
    virtual Status Reload() = 0;
    virtual StatusOr<bool> IsHighContrastEnabled() const = 0;
    virtual StatusOr<std::string> GetThemeName() const = 0;
};

}  // namespace npp::platform
