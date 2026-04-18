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

private:
    std::string buf_;
};
#endif