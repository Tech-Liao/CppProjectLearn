#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <memory>  // 引入智能指针头文件
#include <string>

#include "network/Connection.h"
#include "network/EventLoop.h"

class TcpServer {
public:
    TcpServer(const std::string& ip, uint16_t port);
    ~TcpServer();
    // 启动服务器
    void start();

private:
    std::string ip_;
    uint16_t port_;
    int listen_fd_;  // 监听socket文件描述符
    std::unique_ptr<EventLoop> loop_;
    std::map<int, std::shared_ptr<Connection>> connections_;
};
#endif