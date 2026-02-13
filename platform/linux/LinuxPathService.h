#pragma once

#include "IPathService.h"

namespace npp::platform {

class LinuxPathService final : public IPathService {
public:
    ~LinuxPathService() override = default;

    StatusOr<std::string> GetPath(PathScope scope) const override;
    StatusOr<std::string> GetAppPath(
        PathScope scope,
        const std::string& appName) const override;
    StatusOr<std::string> Normalize(const std::string& utf8Path) const override;
    StatusOr<std::string> ExpandUser(const std::string& utf8Path) const override;
};

}  // namespace npp::platform
