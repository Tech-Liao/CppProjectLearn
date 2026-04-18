#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include <sys/epoll.h>

#include <functional>
#include <map>
#include <vector>
class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    // 定义回调函数
    using EventCallback = std::function<void()>;
    // 核心循环
    void Loop();
    // 将文件描述符添加到监听列表
    void AddEvent(int fd, uint32_t events, EventCallback cb);
    // 移除监听
    void RemoveEvent(int fd);

private:
    int epoll_fd_;
    static const int MAX_EVENTS = 1024;
    struct epoll_event events_[MAX_EVENTS];  // 接受就绪事件
    bool quit_;

    // 保存fd与回调函数的映射
    std::map<int, EventCallback> callbacks_;
};
#endif