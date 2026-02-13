#pragma once

#include <string>

#include "Types.h"

namespace npp::platform {

class IDiagnosticsService {
public:
    virtual ~IDiagnosticsService() = default;

    virtual StatusOr<std::string> EnsureCrashDirectory(const std::string& appName) = 0;
    virtual StatusOr<std::string> EnsureLogDirectory(const std::string& appName) = 0;
    virtual Status WriteDiagnostic(
        const std::string& appName,
        const std::string& category,
        const std::string& fileName,
        const std::string& content) = 0;
};

}  // namespace npp::platform
