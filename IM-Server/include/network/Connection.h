#ifndef CONNECTION_H
#define CONNECTION_H
#include <ctime>
#include <functional>
#include <memory>
#include <string>

#include "network/Buffer.h"
#include "network/EventLoop.h"
#include "network/ThreadPool.h"
class Connection : public std::enable_shared_from_this<Connection> {
public:
    using CloseCallback = std::function<void(uint32_t)>;
    Connection(EventLoop *loop, int fd);
    ~Connection();

    // 核心：当epoll 发现有数据可读时，调用该函数
    void Read();
    // 核心：给客户端发消息
    void Send(const std::string &msg);

    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }
    // 获取最后活跃时间
    time_t GetLastActiveTime() const { return last_active_time_; }
    // 更新活跃时间（只要收到任何数据就调用)
    void UpdateActiveTime() { last_active_time_ = time(NULL); }
    // 统一事件分发器
    void HandleEvent(uint32_t revents);
    // 异步写函数
    void Write();

private:
    EventLoop *loop_;
    int fd_;
    Buffer read_buffer_;
    CloseCallback close_callback_;
    // 未来要加的：用户登陆绑定的ID
    // int user_id = -1;
    // 【新增 2】：记住当前这个连接登录成功的用户名
    std::string current_user_;
    // 记录最后一次收到包的时间
    time_t last_active_time_;

    // 写缓冲的锁与写缓冲
    std::mutex send_mutex_;
    std::string write_buffer_;
};
#endif