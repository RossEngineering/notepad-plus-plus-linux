#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "ILspClientService.h"
#include "IProcessService.h"

namespace npp::platform {

class LinuxLspClientService final : public ILspClientService {
public:
    explicit LinuxLspClientService(IProcessService* processService);
    ~LinuxLspClientService() override = default;

    Status RegisterServer(const LspServerConfig& config) override;
    StatusOr<std::uint64_t> StartSession(const LspSessionSpec& spec) override;
    Status StopSession(std::uint64_t sessionId) override;
    Status StopAllSessions() override;
    bool IsSessionActive(std::uint64_t sessionId) const override;
    std::size_t ActiveSessionCount() const override;

private:
    struct SessionEntry {
        std::uint64_t sessionId = 0;
        std::uint64_t processHandle = 0;
        std::string languageId;
        std::string workspacePathUtf8;
        std::string documentPathUtf8;
    };

    IProcessService* _processService = nullptr;
    std::unordered_map<std::string, LspServerConfig> _serversByLanguage;
    std::unordered_map<std::uint64_t, SessionEntry> _activeSessions;
    std::uint64_t _nextSessionId = 1;
};

}  // namespace npp::platform
