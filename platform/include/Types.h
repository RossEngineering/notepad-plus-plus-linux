#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace npp::platform {

enum class StatusCode {
    kOk = 0,
    kNotFound,
    kPermissionDenied,
    kInvalidArgument,
    kUnavailable,
    kTimeout,
    kIoError,
    kUnknown,
};

struct Status {
    StatusCode code = StatusCode::kOk;
    std::string message;

    [[nodiscard]] bool ok() const {
        return code == StatusCode::kOk;
    }

    static Status Ok() {
        return {};
    }
};

template <typename T>
struct StatusOr {
    Status status;
    std::optional<T> value;

    [[nodiscard]] bool ok() const {
        return status.ok() && value.has_value();
    }

    static StatusOr<T> FromStatus(Status s) {
        return StatusOr<T>{s, std::nullopt};
    }
};

struct RgbaColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

struct Point {
    int x = 0;
    int y = 0;
};

}  // namespace npp::platform
