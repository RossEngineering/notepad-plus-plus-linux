#pragma once

#include "IProcessService.h"

namespace npp::platform {

class LinuxProcessService final : public IProcessService {
public:
    ~LinuxProcessService() override = default;

    Status OpenPath(const std::string& utf8Path) override;
    Status OpenUrl(const std::string& utf8Url) override;
    StatusOr<ProcessResult> Run(
        const ProcessSpec& spec,
        std::chrono::milliseconds timeout) override;
    StatusOr<std::uint64_t> Spawn(const ProcessSpec& spec) override;
    Status Terminate(std::uint64_t processHandle) override;
};

}  // namespace npp::platform
