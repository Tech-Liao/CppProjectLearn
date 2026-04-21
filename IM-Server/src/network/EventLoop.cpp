#include "network/EventLoop.h"

#include <unistd.h>

#include <iostream>
#include <stdexcept>

EventLoop::EventLoop() : quit_(false) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) throw std::runtime_error("epoll create failed");
}
EventLoop::~EventLoop() {
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

void EventLoop::AddEvent(int fd, uint32_t events, EventCallback cb) {
    struct epoll_event ev{0};
    ev.data.fd = fd;
    ev.events = events;
    // 简化处理，一般需要进行判断
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    // 绑定业务逻辑
    callbacks_[fd] = cb;
}

void EventLoop::RemoveEvent(int fd) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    callbacks_.erase(fd);  // 清理回调
}

void EventLoop::Loop() {
    while (!quit_) {
        // 2、等待事件发生
        int nfds = epoll_wait(epoll_fd_, events_, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            break;
        }
        for (int i = 0; i < nfds; ++i) {
            int fd = events_[i].data.fd;
            uint32_t revents = events_[i].events;
            // 这是处理逻辑的分发点
            std::cout << "Event triggered on fd: " << fd << std::endl;
            if (callbacks_.count(fd)) {
                callbacks_[fd](revents);
            }
        }
    }
}