#include "network/TcpServer.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

TcpServer::TcpServer(const std::string &ip, uint16_t port)
    : ip_(ip), port_(port), listen_fd_(-1) {
    // 1、创建tcp socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) throw std::runtime_error("创建失败");
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
    std::cout << "等待客户端链接" << std::endl;
    while (true) {
        // 5、阻塞等待客户端链接
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd =
            accept(listen_fd_, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            std::cerr << "accept失败" << std::endl;
            continue;
        }
        // 打印新连接的信息
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        uint16_t client_port = ntohs(client_addr.sin_port);
        std::cout << "新连接来自:" << client_ip << ":" << client_port
                  << "(fd:" << client_fd << ")" << std::endl;
        const char *msg = "欢迎来到IM Server\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
    }
}