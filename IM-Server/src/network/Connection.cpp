#include "network/Connection.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

static void SetNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl get failed" << std::endl;
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl set failed" << std::endl;
    }
}

Connection::Connection(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {
    SetNonBlocking(fd_);
    loop_->AddEvent(fd_, EPOLLIN, [this]() { this->Read(); });
}

Connection::~Connection() {
    loop_->RemoveEvent(fd_);
    close(fd_);
    std::cout << "Connection " << fd_ << " closed and destroyed." << std::endl;
}

void Connection::Read() {
    char buf[1024];
    while (true) {  // 非阻塞必须循环度，直到无法读取数据为止
        int bytes_read = read(fd_, buf, sizeof(buf));
        if (bytes_read > 0) {
            read_buffer_.Append(buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {
            // 被系统信息打断，正常情况继续读
            continue;
        } else if (bytes_read == -1 &&
                   (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // EAGAIN说明当前缓冲区已经读取完，需要等下次epoll通知
            break;
        } else if (bytes_read == 0) {
            // 代表客户端主动断开了连接
            std::cout << "client disconnected,fd:" << fd_ << std::endl;
            if (close_callback_) {
                close_callback_(fd_);
            }
            break;
        } else {
            std::cerr << "Read error on fd: " << fd_ << std::endl;
            if (close_callback_) close_callback_(fd_);
            break;
        }
        if (read_buffer_.ReadableBytes() > 0) {
            std::string msg = read_buffer_.RetrieveAllAsString();
            std::cout << "Received from fd " << fd_ << ":" << msg << std::endl;
            Send("Echo: " + msg);
        }
    }
}

void Connection::Send(const std::string& msg) {
    // 简化板，先用write，一次发不完，应该存入write_buffer_
    write(fd_, msg.c_str(), msg.length());
}