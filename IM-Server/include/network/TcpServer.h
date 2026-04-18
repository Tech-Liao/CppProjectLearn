#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

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
};
#endif