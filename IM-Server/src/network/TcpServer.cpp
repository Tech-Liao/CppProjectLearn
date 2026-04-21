#include "network/TcpServer.h"

#include <fcntl.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
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
TcpServer::TcpServer(const std::string &ip, uint16_t port)
    : ip_(ip), port_(port), listen_fd_(-1), loop_(new EventLoop()) {
    // 1、创建tcp socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) throw std::runtime_error("创建失败");
    // 增加非阻塞
    SetNonBlocking(listen_fd_);
    // 2、设置端口复用
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 3、绑定ip与端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    if (bind(listen_fd_, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        close(listen_fd_);
        throw std::runtime_error("绑定失败");
    }
    // 4、开始监听
    if (listen(listen_fd_, 128) == -1) {
        close(listen_fd_);
        throw std::runtime_error("监听失败");
    }
    std::cout << "Tcp Server初始化 " << ip_ << ":" << port_ << std::endl;
}
TcpServer::~TcpServer() {
    if (listen_fd_ != -1) {
        close(listen_fd_);
        std::cout << "Tcp Server关闭" << std::endl;
    }
}
void TcpServer::start() {
    loop_->AddEvent(listen_fd_, EPOLLIN, [this](uint32_t revents) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(this->listen_fd_,
                               (struct sockaddr *)&client_addr, &client_len);
        if (client_fd != -1) {
            std::cout << "New Client Connected! fd: " << client_fd << std::endl;
            // 1、创建专属connection对象
            std::unique_ptr<Connection> conn(
                new Connection(loop_.get(), client_fd));
            // 2、告诉Connection
            conn->SetCloseCallback(
                [this](int fd) { this->connections_.erase(fd); });
            // 3、将其保存到tcp中
            connections_[client_fd] = std::move(conn);
        }
    });
    std::cout << "IM server start with epoll" << std::endl;
    loop_->Loop();
}