#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Types.h"

namespace npp::platform {

struct LspServerConfig {
    std::string languageId;
    std::string command;
    std::vector<std::string> args;
    std::vector<std::pair<std::string, std::string>> environmentOverrides;
};

struct LspSessionSpec {
    std::string languageId;
    std::string workspacePathUtf8;
    std::string documentPathUtf8;
};

class ILspClientService {
public:
    virtual ~ILspClientService() = default;

    virtual Status RegisterServer(const LspServerConfig& config) = 0;
    virtual StatusOr<std::uint64_t> StartSession(const LspSessionSpec& spec) = 0;
    virtual Status StopSession(std::uint64_t sessionId) = 0;
    virtual Status StopAllSessions() = 0;
    virtual bool IsSessionActive(std::uint64_t sessionId) const = 0;
    virtual std::size_t ActiveSessionCount() const = 0;
};

}  // namespace npp::platform
