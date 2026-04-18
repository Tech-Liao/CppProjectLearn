#ifndef CONNECTION_H
#define CONNECTION_H
#include <functional>
#include <memory>
#include <string>

#include "network/Buffer.h"
#include "network/EventLoop.h"
class Connection {
public:
    using CloseCallback = std::function<void(int)>;
    Connection(EventLoop *loop, int fd);
    ~Connection();

    // 核心：当epoll 发现有数据可读时，调用该函数
    void Read();
    // 核心：给客户端发消息
    void Send(const std::string &msg);

    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }

private:
    EventLoop *loop_;
    int fd_;
    Buffer read_buffer_;
    CloseCallback close_callback_;
    // 未来要加的：用户登陆绑定的ID
    // int user_id = -1;
};
#endif