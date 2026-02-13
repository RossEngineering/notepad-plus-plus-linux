#include "LinuxPathService.h"

#include <cstdlib>
#include <filesystem>
#include <string>

namespace npp::platform {

namespace {

Status MakeInvalidArgumentStatus(const std::string& message) {
    return Status{StatusCode::kInvalidArgument, message};
}

StatusOr<std::string> GetHomeDirectory() {
    const char* home = std::getenv("HOME");
    if (!home || std::string(home).empty()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kUnavailable, "HOME is not set"});
    }
    return StatusOr<std::string>{Status::Ok(), std::string(home)};
}

std::string GetEnvOrDefault(const char* varName, const std::string& defaultValue) {
    const char* value = std::getenv(varName);
    if (value && std::string(value).size() > 0) {
        return std::string(value);
    }
    return defaultValue;
}

StatusOr<std::string> ExpandUserPathInternal(const std::string& utf8Path) {
    if (utf8Path.empty() || utf8Path[0] != '~') {
        return StatusOr<std::string>{Status::Ok(), utf8Path};
    }

    if (utf8Path.size() > 1 && utf8Path[1] != '/') {
        return StatusOr<std::string>::FromStatus(
            MakeInvalidArgumentStatus("Only ~/... user paths are supported"));
    }

    const auto home = GetHomeDirectory();
    if (!home.ok()) {
        return StatusOr<std::string>::FromStatus(home.status);
    }

    if (utf8Path == "~") {
        return StatusOr<std::string>{Status::Ok(), *home.value};
    }
    return StatusOr<std::string>{Status::Ok(), *home.value + utf8Path.substr(1)};
}

}  // namespace

StatusOr<std::string> LinuxPathService::GetPath(PathScope scope) const {
    const auto home = GetHomeDirectory();
    if (!home.ok()) {
        return StatusOr<std::string>::FromStatus(home.status);
    }

    switch (scope) {
        case PathScope::kHome:
            return StatusOr<std::string>{Status::Ok(), *home.value};
        case PathScope::kConfig:
            return StatusOr<std::string>{
                Status::Ok(),
                GetEnvOrDefault("XDG_CONFIG_HOME", *home.value + "/.config")};
        case PathScope::kData:
            return StatusOr<std::string>{
                Status::Ok(),
                GetEnvOrDefault("XDG_DATA_HOME", *home.value + "/.local/share")};
        case PathScope::kCache:
            return StatusOr<std::string>{
                Status::Ok(),
                GetEnvOrDefault("XDG_CACHE_HOME", *home.value + "/.cache")};
        case PathScope::kState:
            return StatusOr<std::string>{
                Status::Ok(),
                GetEnvOrDefault("XDG_STATE_HOME", *home.value + "/.local/state")};
        case PathScope::kTemp: {
            return StatusOr<std::string>{
                Status::Ok(),
                GetEnvOrDefault("TMPDIR", "/tmp")};
        }
    }

    return StatusOr<std::string>::FromStatus(
        Status{StatusCode::kInvalidArgument, "Unknown PathScope value"});
}

StatusOr<std::string> LinuxPathService::GetAppPath(
    PathScope scope,
    const std::string& appName) const {
    if (appName.empty()) {
        return StatusOr<std::string>::FromStatus(
            MakeInvalidArgumentStatus("Application name must not be empty"));
    }
    if (appName.find('/') != std::string::npos) {
        return StatusOr<std::string>::FromStatus(
            MakeInvalidArgumentStatus("Application name must not contain '/'"));
    }

    const auto base = GetPath(scope);
    if (!base.ok()) {
        return StatusOr<std::string>::FromStatus(base.status);
    }

    std::filesystem::path merged = std::filesystem::path(*base.value) / appName;
    return StatusOr<std::string>{Status::Ok(), merged.lexically_normal().string()};
}

StatusOr<std::string> LinuxPathService::Normalize(const std::string& utf8Path) const {
    if (utf8Path.empty()) {
        return StatusOr<std::string>::FromStatus(
            MakeInvalidArgumentStatus("Path must not be empty"));
    }

    const auto expanded = ExpandUserPathInternal(utf8Path);
    if (!expanded.ok()) {
        return StatusOr<std::string>::FromStatus(expanded.status);
    }

    std::filesystem::path normalized = std::filesystem::path(*expanded.value).lexically_normal();
    return StatusOr<std::string>{Status::Ok(), normalized.string()};
}

StatusOr<std::string> LinuxPathService::ExpandUser(const std::string& utf8Path) const {
    return ExpandUserPathInternal(utf8Path);
}

}  // namespace npp::platform
