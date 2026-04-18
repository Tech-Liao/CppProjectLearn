#include "network/Buffer.h"

void Buffer::Append(const char *data, size_t len) { buf_.append(data, len); }

std::string Buffer::RetrieveAllAsString() {
    std::string result = buf_;
    buf_.clear();
    return result;
}

size_t Buffer::ReadableBytes() const { return buf_.size(); }