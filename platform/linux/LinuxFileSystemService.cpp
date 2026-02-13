#include "LinuxFileSystemService.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <unistd.h>

namespace npp::platform {

namespace {

StatusCode MapErrorCode(const std::error_code& ec) {
    using std::errc;
    switch (static_cast<errc>(ec.value())) {
        case errc::no_such_file_or_directory:
            return StatusCode::kNotFound;
        case errc::permission_denied:
            return StatusCode::kPermissionDenied;
        case errc::timed_out:
            return StatusCode::kTimeout;
        case errc::invalid_argument:
            return StatusCode::kInvalidArgument;
        default:
            return StatusCode::kIoError;
    }
}

Status MakeStatusFromError(const std::error_code& ec, const std::string& context) {
    if (!ec) {
        return Status::Ok();
    }
    return Status{MapErrorCode(ec), context + ": " + ec.message()};
}

Status MakeInvalidPathStatus() {
    return Status{StatusCode::kInvalidArgument, "Path must not be empty"};
}

std::filesystem::path ToPath(const std::string& utf8Path) {
    return std::filesystem::path(utf8Path);
}

Status WriteContentToPath(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return Status{StatusCode::kIoError, "Failed opening file for write: " + path.string()};
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!out.good()) {
        return Status{StatusCode::kIoError, "Failed writing file: " + path.string()};
    }
    out.close();
    if (!out.good()) {
        return Status{StatusCode::kIoError, "Failed closing file: " + path.string()};
    }
    return Status::Ok();
}

}  // namespace

StatusOr<bool> LinuxFileSystemService::Exists(const std::string& utf8Path) const {
    if (utf8Path.empty()) {
        return StatusOr<bool>::FromStatus(MakeInvalidPathStatus());
    }
    std::error_code ec;
    const bool exists = std::filesystem::exists(ToPath(utf8Path), ec);
    if (ec) {
        return StatusOr<bool>::FromStatus(MakeStatusFromError(ec, "exists"));
    }
    return StatusOr<bool>{Status::Ok(), exists};
}

StatusOr<bool> LinuxFileSystemService::IsDirectory(const std::string& utf8Path) const {
    if (utf8Path.empty()) {
        return StatusOr<bool>::FromStatus(MakeInvalidPathStatus());
    }
    std::error_code ec;
    const bool isDir = std::filesystem::is_directory(ToPath(utf8Path), ec);
    if (ec) {
        return StatusOr<bool>::FromStatus(MakeStatusFromError(ec, "is_directory"));
    }
    return StatusOr<bool>{Status::Ok(), isDir};
}

StatusOr<std::string> LinuxFileSystemService::ReadTextFile(const std::string& utf8Path) const {
    if (utf8Path.empty()) {
        return StatusOr<std::string>::FromStatus(MakeInvalidPathStatus());
    }

    std::ifstream in(ToPath(utf8Path), std::ios::binary);
    if (!in.is_open()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kIoError, "Failed opening file for read: " + utf8Path});
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (!in.good() && !in.eof()) {
        return StatusOr<std::string>::FromStatus(
            Status{StatusCode::kIoError, "Failed reading file: " + utf8Path});
    }
    return StatusOr<std::string>{Status::Ok(), buffer.str()};
}

Status LinuxFileSystemService::WriteTextFile(
    const std::string& utf8Path,
    const std::string& content,
    const WriteFileOptions& options) {
    if (utf8Path.empty()) {
        return MakeInvalidPathStatus();
    }

    const std::filesystem::path target = ToPath(utf8Path);
    std::error_code ec;

    if (options.createParentDirs && target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), ec);
        if (ec) {
            return MakeStatusFromError(ec, "create_directories");
        }
    }

    if (!options.atomic) {
        return WriteContentToPath(target, content);
    }

    const auto tick = static_cast<long long>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const std::filesystem::path tempPath =
        target.string() + ".tmp." + std::to_string(getpid()) + "." + std::to_string(tick);

    Status writeStatus = WriteContentToPath(tempPath, content);
    if (!writeStatus.ok()) {
        return writeStatus;
    }

    std::filesystem::rename(tempPath, target, ec);
    if (ec) {
        ec.clear();
        std::filesystem::remove(target, ec);
        ec.clear();
        std::filesystem::rename(tempPath, target, ec);
        if (ec) {
            std::error_code cleanupEc;
            std::filesystem::remove(tempPath, cleanupEc);
            return MakeStatusFromError(ec, "rename");
        }
    }
    return Status::Ok();
}

Status LinuxFileSystemService::CreateDirectories(const std::string& utf8Path) {
    if (utf8Path.empty()) {
        return MakeInvalidPathStatus();
    }
    std::error_code ec;
    std::filesystem::create_directories(ToPath(utf8Path), ec);
    return MakeStatusFromError(ec, "create_directories");
}

Status LinuxFileSystemService::RemoveFile(const std::string& utf8Path) {
    if (utf8Path.empty()) {
        return MakeInvalidPathStatus();
    }
    std::error_code ec;
    const bool removed = std::filesystem::remove(ToPath(utf8Path), ec);
    if (ec) {
        return MakeStatusFromError(ec, "remove");
    }
    if (!removed) {
        return Status{StatusCode::kNotFound, "File not found: " + utf8Path};
    }
    return Status::Ok();
}

Status LinuxFileSystemService::RenameReplace(
    const std::string& srcUtf8Path,
    const std::string& dstUtf8Path) {
    if (srcUtf8Path.empty() || dstUtf8Path.empty()) {
        return MakeInvalidPathStatus();
    }

    std::error_code ec;
    std::filesystem::rename(ToPath(srcUtf8Path), ToPath(dstUtf8Path), ec);
    if (ec) {
        ec.clear();
        std::filesystem::remove(ToPath(dstUtf8Path), ec);
        ec.clear();
        std::filesystem::rename(ToPath(srcUtf8Path), ToPath(dstUtf8Path), ec);
        if (ec) {
            return MakeStatusFromError(ec, "rename");
        }
    }
    return Status::Ok();
}

StatusOr<std::vector<std::string>> LinuxFileSystemService::ListDirectory(
    const std::string& utf8Path,
    const ListDirectoryOptions& options) const {
    if (utf8Path.empty()) {
        return StatusOr<std::vector<std::string>>::FromStatus(MakeInvalidPathStatus());
    }

    const std::filesystem::path root = ToPath(utf8Path);
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return StatusOr<std::vector<std::string>>::FromStatus(
            Status{StatusCode::kNotFound, "Directory does not exist: " + utf8Path});
    }
    if (ec) {
        return StatusOr<std::vector<std::string>>::FromStatus(
            MakeStatusFromError(ec, "exists"));
    }
    if (!std::filesystem::is_directory(root, ec)) {
        return StatusOr<std::vector<std::string>>::FromStatus(
            Status{StatusCode::kInvalidArgument, "Path is not a directory: " + utf8Path});
    }
    if (ec) {
        return StatusOr<std::vector<std::string>>::FromStatus(
            MakeStatusFromError(ec, "is_directory"));
    }

    std::vector<std::string> entries;
    if (options.recursive) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
            if (ec) {
                return StatusOr<std::vector<std::string>>::FromStatus(
                    MakeStatusFromError(ec, "recursive_directory_iterator"));
            }
            if (!options.includeDirectories && entry.is_directory(ec)) {
                ec.clear();
                continue;
            }
            entries.push_back(entry.path().string());
        }
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
            if (ec) {
                return StatusOr<std::vector<std::string>>::FromStatus(
                    MakeStatusFromError(ec, "directory_iterator"));
            }
            if (!options.includeDirectories && entry.is_directory(ec)) {
                ec.clear();
                continue;
            }
            entries.push_back(entry.path().string());
        }
    }

    return StatusOr<std::vector<std::string>>{Status::Ok(), entries};
}

}  // namespace npp::platform
