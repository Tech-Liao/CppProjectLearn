#include <iostream>

#include "network/TcpServer.h"
int main() {
    try {
        TcpServer server("0.0.0.0", 8080);
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Server Error:" << e.what() << std::endl;
        return 1;
    }
    return 0;
}