#include "LinuxClipboardService.h"

namespace npp::platform {

namespace {

Status UnsupportedBufferStatus() {
    return Status{StatusCode::kInvalidArgument, "Clipboard buffer is not supported"};
}

}  // namespace

std::string* LinuxClipboardService::MutableBufferSlot(ClipboardBuffer buffer) {
    switch (buffer) {
        case ClipboardBuffer::kClipboard:
            return &_clipboardText;
        case ClipboardBuffer::kPrimarySelection:
            return &_primarySelectionText;
    }
    return nullptr;
}

const std::string* LinuxClipboardService::BufferSlot(ClipboardBuffer buffer) const {
    switch (buffer) {
        case ClipboardBuffer::kClipboard:
            return &_clipboardText;
        case ClipboardBuffer::kPrimarySelection:
            return &_primarySelectionText;
    }
    return nullptr;
}

Status LinuxClipboardService::SetText(const std::string& utf8Text, ClipboardBuffer buffer) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::string* slot = MutableBufferSlot(buffer);
    if (slot == nullptr) {
        return UnsupportedBufferStatus();
    }
    *slot = utf8Text;
    return Status::Ok();
}

StatusOr<std::string> LinuxClipboardService::GetText(ClipboardBuffer buffer) const {
    std::lock_guard<std::mutex> lock(_mutex);
    const std::string* slot = BufferSlot(buffer);
    if (slot == nullptr) {
        return StatusOr<std::string>::FromStatus(UnsupportedBufferStatus());
    }
    return StatusOr<std::string>{Status::Ok(), *slot};
}

Status LinuxClipboardService::Clear(ClipboardBuffer buffer) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::string* slot = MutableBufferSlot(buffer);
    if (slot == nullptr) {
        return UnsupportedBufferStatus();
    }
    slot->clear();
    return Status::Ok();
}

bool LinuxClipboardService::SupportsBuffer(ClipboardBuffer buffer) const {
    switch (buffer) {
        case ClipboardBuffer::kClipboard:
        case ClipboardBuffer::kPrimarySelection:
            return true;
    }
    return false;
}

}  // namespace npp::platform
