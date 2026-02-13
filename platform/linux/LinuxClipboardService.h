#pragma once

#include <mutex>
#include <string>

#include "IClipboardService.h"

namespace npp::platform {

// Headless-safe clipboard implementation for migration lanes.
// Uses process-local storage for clipboard buffers.
class LinuxClipboardService final : public IClipboardService {
public:
    ~LinuxClipboardService() override = default;

    Status SetText(
        const std::string& utf8Text,
        ClipboardBuffer buffer = ClipboardBuffer::kClipboard) override;
    StatusOr<std::string> GetText(
        ClipboardBuffer buffer = ClipboardBuffer::kClipboard) const override;
    Status Clear(ClipboardBuffer buffer = ClipboardBuffer::kClipboard) override;
    bool SupportsBuffer(ClipboardBuffer buffer) const override;

private:
    std::string* MutableBufferSlot(ClipboardBuffer buffer);
    const std::string* BufferSlot(ClipboardBuffer buffer) const;

    mutable std::mutex _mutex;
    std::string _clipboardText;
    std::string _primarySelectionText;
};

}  // namespace npp::platform
