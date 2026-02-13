#include "LinuxProcessService.h"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace npp::platform {

namespace {

StatusCode MapErrno(int err) {
    switch (err) {
        case ENOENT:
            return StatusCode::kNotFound;
        case EACCES:
            return StatusCode::kPermissionDenied;
        case EINVAL:
            return StatusCode::kInvalidArgument;
        case ETIMEDOUT:
            return StatusCode::kTimeout;
        default:
            return StatusCode::kIoError;
    }
}

Status MakeErrnoStatus(int err, const std::string& context) {
    return Status{MapErrno(err), context + ": " + std::strerror(err)};
}

Status ValidateSpec(const ProcessSpec& spec) {
    if (spec.program.empty()) {
        return Status{StatusCode::kInvalidArgument, "program must not be empty"};
    }
    return Status::Ok();
}

std::vector<char*> BuildArgv(const ProcessSpec& spec) {
    std::vector<char*> argv;
    argv.reserve(spec.args.size() + 2);
    argv.push_back(const_cast<char*>(spec.program.c_str()));
    for (const std::string& arg : spec.args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

void ApplyChildEnvironment(const ProcessSpec& spec) {
    if (!spec.inheritEnvironment) {
#if defined(__GLIBC__) || defined(__linux__)
        clearenv();
#endif
    }
    for (const auto& kv : spec.environmentOverrides) {
        setenv(kv.first.c_str(), kv.second.c_str(), 1);
    }
}

int ExitCodeFromWaitStatus(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return -1;
}

}  // namespace

StatusOr<std::uint64_t> LinuxProcessService::Spawn(const ProcessSpec& spec) {
    const Status validation = ValidateSpec(spec);
    if (!validation.ok()) {
        return StatusOr<std::uint64_t>::FromStatus(validation);
    }

    pid_t pid = fork();
    if (pid < 0) {
        return StatusOr<std::uint64_t>::FromStatus(MakeErrnoStatus(errno, "fork"));
    }

    if (pid == 0) {
        if (spec.detached) {
            setsid();
        }
        if (spec.workingDirectoryUtf8.has_value()) {
            if (chdir(spec.workingDirectoryUtf8->c_str()) != 0) {
                _exit(127);
            }
        }

        ApplyChildEnvironment(spec);
        std::vector<char*> argv = BuildArgv(spec);
        execvp(spec.program.c_str(), argv.data());
        _exit(127);
    }

    return StatusOr<std::uint64_t>{Status::Ok(), static_cast<std::uint64_t>(pid)};
}

StatusOr<ProcessResult> LinuxProcessService::Run(
    const ProcessSpec& spec,
    std::chrono::milliseconds timeout) {
    if (timeout.count() <= 0) {
        return StatusOr<ProcessResult>::FromStatus(
            Status{StatusCode::kInvalidArgument, "timeout must be > 0"});
    }

    auto child = Spawn(spec);
    if (!child.ok()) {
        return StatusOr<ProcessResult>::FromStatus(child.status);
    }

    const pid_t pid = static_cast<pid_t>(*child.value);
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    int waitStatus = 0;

    while (true) {
        const pid_t waitResult = waitpid(pid, &waitStatus, WNOHANG);
        if (waitResult == pid) {
            ProcessResult result;
            result.exitCode = ExitCodeFromWaitStatus(waitStatus);
            return StatusOr<ProcessResult>{Status::Ok(), result};
        }
        if (waitResult < 0) {
            return StatusOr<ProcessResult>::FromStatus(MakeErrnoStatus(errno, "waitpid"));
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            kill(pid, SIGTERM);
            waitpid(pid, &waitStatus, 0);
            return StatusOr<ProcessResult>::FromStatus(
                Status{StatusCode::kTimeout, "process timed out"});
        }
        usleep(10 * 1000);
    }
}

Status LinuxProcessService::Terminate(std::uint64_t processHandle) {
    const pid_t pid = static_cast<pid_t>(processHandle);
    if (pid <= 0) {
        return Status{StatusCode::kInvalidArgument, "invalid process handle"};
    }
    if (kill(pid, SIGTERM) != 0) {
        return MakeErrnoStatus(errno, "kill");
    }
    return Status::Ok();
}

Status LinuxProcessService::OpenPath(const std::string& utf8Path) {
    ProcessSpec spec;
    spec.program = "xdg-open";
    spec.args = {utf8Path};
    spec.detached = true;
    auto spawned = Spawn(spec);
    if (!spawned.ok()) {
        return spawned.status;
    }
    return Status::Ok();
}

Status LinuxProcessService::OpenUrl(const std::string& utf8Url) {
    ProcessSpec spec;
    spec.program = "xdg-open";
    spec.args = {utf8Url};
    spec.detached = true;
    auto spawned = Spawn(spec);
    if (!spawned.ok()) {
        return spawned.status;
    }
    return Status::Ok();
}

}  // namespace npp::platform
