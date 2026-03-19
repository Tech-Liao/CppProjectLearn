#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>  // bzero, strlen
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

int main() {
    // === 【第一阶段：开业转变】====
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(8080);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sockfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    listen(sockfd, 5);
    std::cout << "服务器已启动，正在监听 8080 端口..." << std::endl;
    // 大门设置为非阻塞
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    // 聘请大堂经理
    int epollfd = epoll_create(1);
    // 安上门铃
    struct epoll_event event;
    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
    // ===【第二阶段：正式营业】===
    const int MAX_EVENTS = 1024;
    struct epoll_event events[MAX_EVENTS];
    while (true) {
        // 服务员在吧台等候，返回响铃的数量
        int num_events = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < num_events; i++) {
            int current_fd = events[i].data.fd;
            if (current_fd == sockfd) {
                while (true) {
                    struct sockaddr_in clt_addr;
                    socklen_t clt_len = sizeof(clt_addr);
                    int connfd =
                        accept(sockfd, (struct sockaddr *)&clt_addr, &clt_len);
                    if (connfd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        else {
                            std::cout << errno << std::endl;
                            break;
                        }
                    } else {
                        int flags = fcntl(connfd, F_GETFL, 0);
                        fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
                        struct epoll_event event;
                        event.data.fd = connfd;
                        event.events = EPOLLIN | EPOLLET;
                        epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
                    }
                }
            } else if (events[i].events & EPOLLIN) {
                char buf[1024];
                bzero(buf, sizeof(buf));
                while (true) {
                    int bytes_read = read(current_fd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        // 成功读到了一部分数据，打印出来看看，然后继续循环读下一波！
                        std::cout << "收到数据: \n" << buf << std::endl;
                        bzero(buf, sizeof(buf));  // 清空 buffer 准备下一次读取

                    } else if (bytes_read == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 【核心转折点】：数据全读完了！该上菜了！
                            const char *response =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "Connection: close\r\n"  // 告诉浏览器发完就断开
                                "\r\n"
                                "<html><body><h1>Hello, Epoll "
                                "WebServer!</h1></body></html>";
                            write(current_fd, response, strlen(response));
                            break;  // 菜上完了，跳出读取循环，回吧台
                        } else {
                            std::cout << "read error: " << errno << std::endl;
                            close(current_fd);  // 出错了，强制掀桌子
                            break;
                        }

                    } else if (bytes_read == 0) {
                        // 顾客主动断开了连接（TCP 挥手）
                        std::cout << "客户端断开连接！" << std::endl;
                        close(current_fd);
                        break;
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}