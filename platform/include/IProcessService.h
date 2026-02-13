#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Types.h"

namespace npp::platform {

struct ProcessSpec {
    std::string program;
    std::vector<std::string> args;
    std::optional<std::string> workingDirectoryUtf8;
    std::vector<std::pair<std::string, std::string>> environmentOverrides;
    bool inheritEnvironment = true;
    bool detached = false;
};

struct ProcessResult {
    int exitCode = -1;
    std::string stdOutUtf8;
    std::string stdErrUtf8;
};

class IProcessService {
public:
    virtual ~IProcessService() = default;

    virtual Status OpenPath(const std::string& utf8Path) = 0;
    virtual Status OpenUrl(const std::string& utf8Url) = 0;
    virtual StatusOr<ProcessResult> Run(
        const ProcessSpec& spec,
        std::chrono::milliseconds timeout) = 0;
    virtual StatusOr<std::uint64_t> Spawn(const ProcessSpec& spec) = 0;
    virtual Status Terminate(std::uint64_t processHandle) = 0;
};

}  // namespace npp::platform
