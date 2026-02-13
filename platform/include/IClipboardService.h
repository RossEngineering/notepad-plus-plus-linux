#pragma once

#include <string>

#include "Types.h"

namespace npp::platform {

enum class ClipboardBuffer {
    kClipboard = 0,
    kPrimarySelection,
};

class IClipboardService {
public:
    virtual ~IClipboardService() = default;

    virtual Status SetText(
        const std::string& utf8Text,
        ClipboardBuffer buffer = ClipboardBuffer::kClipboard) = 0;
    virtual StatusOr<std::string> GetText(
        ClipboardBuffer buffer = ClipboardBuffer::kClipboard) const = 0;
    virtual Status Clear(ClipboardBuffer buffer = ClipboardBuffer::kClipboard) = 0;
    virtual bool SupportsBuffer(ClipboardBuffer buffer) const = 0;
};

}  // namespace npp::platform
