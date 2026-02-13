#include "LinuxLspClientService.h"

#include <utility>

namespace npp::platform {

namespace {

Status InvalidConfig(const std::string& detail) {
    return Status{StatusCode::kInvalidArgument, detail};
}

}  // namespace

LinuxLspClientService::LinuxLspClientService(IProcessService* processService)
    : _processService(processService) {}

Status LinuxLspClientService::RegisterServer(const LspServerConfig& config) {
    if (_processService == nullptr) {
        return Status{StatusCode::kUnavailable, "LSP client requires process service"};
    }
    if (config.languageId.empty()) {
        return InvalidConfig("LSP languageId must not be empty");
    }
    if (config.command.empty()) {
        return InvalidConfig("LSP command must not be empty");
    }

    _serversByLanguage.insert_or_assign(config.languageId, config);
    return Status::Ok();
}

StatusOr<std::uint64_t> LinuxLspClientService::StartSession(const LspSessionSpec& spec) {
    if (_processService == nullptr) {
        return StatusOr<std::uint64_t>::FromStatus(
            Status{StatusCode::kUnavailable, "LSP client requires process service"});
    }
    if (spec.languageId.empty()) {
        return StatusOr<std::uint64_t>::FromStatus(
            InvalidConfig("LSP session languageId must not be empty"));
    }

    const auto configIt = _serversByLanguage.find(spec.languageId);
    if (configIt == _serversByLanguage.end()) {
        return StatusOr<std::uint64_t>::FromStatus(
            Status{StatusCode::kNotFound, "No LSP server configured for language: " + spec.languageId});
    }

    ProcessSpec processSpec;
    processSpec.program = configIt->second.command;
    processSpec.args = configIt->second.args;
    processSpec.environmentOverrides = configIt->second.environmentOverrides;
    processSpec.detached = true;
    if (!spec.workspacePathUtf8.empty()) {
        processSpec.workingDirectoryUtf8 = spec.workspacePathUtf8;
    }

    const auto spawned = _processService->Spawn(processSpec);
    if (!spawned.ok()) {
        return StatusOr<std::uint64_t>::FromStatus(spawned.status);
    }

    const std::uint64_t sessionId = _nextSessionId++;
    SessionEntry entry;
    entry.sessionId = sessionId;
    entry.processHandle = *spawned.value;
    entry.languageId = spec.languageId;
    entry.workspacePathUtf8 = spec.workspacePathUtf8;
    entry.documentPathUtf8 = spec.documentPathUtf8;
    _activeSessions.insert_or_assign(sessionId, std::move(entry));

    return StatusOr<std::uint64_t>{Status::Ok(), sessionId};
}

Status LinuxLspClientService::StopSession(std::uint64_t sessionId) {
    if (_processService == nullptr) {
        return Status{StatusCode::kUnavailable, "LSP client requires process service"};
    }
    const auto it = _activeSessions.find(sessionId);
    if (it == _activeSessions.end()) {
        return Status{StatusCode::kNotFound, "LSP session not found"};
    }
    const Status terminateStatus = _processService->Terminate(it->second.processHandle);
    if (!terminateStatus.ok()) {
        return terminateStatus;
    }
    _activeSessions.erase(it);
    return Status::Ok();
}

Status LinuxLspClientService::StopAllSessions() {
    if (_processService == nullptr) {
        return Status{StatusCode::kUnavailable, "LSP client requires process service"};
    }
    Status firstFailure = Status::Ok();

    for (auto it = _activeSessions.begin(); it != _activeSessions.end();) {
        const Status terminateStatus = _processService->Terminate(it->second.processHandle);
        if (!terminateStatus.ok() && firstFailure.ok()) {
            firstFailure = terminateStatus;
            ++it;
            continue;
        }
        it = _activeSessions.erase(it);
    }

    return firstFailure;
}

bool LinuxLspClientService::IsSessionActive(std::uint64_t sessionId) const {
    return _activeSessions.find(sessionId) != _activeSessions.end();
}

std::size_t LinuxLspClientService::ActiveSessionCount() const {
    return _activeSessions.size();
}

}  // namespace npp::platform
