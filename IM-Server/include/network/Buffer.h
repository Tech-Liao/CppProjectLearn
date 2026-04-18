#ifndef BUFFER_H
#define BUFFER_H
#include <string>

class Buffer {
public:
    Buffer() = default;
    ~Buffer() = default;

    // 往buffer放数据
    void Append(const char *data, size_t len);
    // 获取buffer所有数据
    std::string RetrieveAllAsString();
    // 查看buffer有多少数据
    size_t ReadableBytes() const;
    // 查看数据
    const char *Peek() const { return buf_.data(); }
    // 拿走指定长度的数据
    void Retrieve(size_t len) {
        if (len <= buf_.size()) buf_.erase(0, len);
    }
    std::string RetrieveAsString(size_t len) {
        std::string result(buf_.data(), len);
        Retrieve(len);
        return result;
    }

private:
    std::string buf_;
};
#endif