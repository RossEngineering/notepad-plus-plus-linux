#pragma once

#include <string>

#include "Types.h"

namespace npp::platform {

enum class PathScope {
    kHome = 0,
    kConfig,
    kData,
    kCache,
    kTemp,
};

class IPathService {
public:
    virtual ~IPathService() = default;

    virtual StatusOr<std::string> GetPath(PathScope scope) const = 0;
    virtual StatusOr<std::string> GetAppPath(
        PathScope scope,
        const std::string& appName) const = 0;
    virtual StatusOr<std::string> Normalize(const std::string& utf8Path) const = 0;
    virtual StatusOr<std::string> ExpandUser(const std::string& utf8Path) const = 0;
};

}  // namespace npp::platform
