#pragma once

#include <string>

namespace npp::ui {

// Converts legacy editor settings keys into canonical keys and normalizes
// value formats so migrations are deterministic across runs.
std::string ApplyEditorSettingsMigrations(const std::string& settingsJsonUtf8);

}  // namespace npp::ui
