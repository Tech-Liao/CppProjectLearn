#include "network/TcpServer.h"
#include "common/Config.h"
#include "storage/MySQLManager.h" // 引入数据库管理器
#include <spdlog/spdlog.h>
#include <iostream>

int main()
{
    // 1. 初始化日志
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("IM Server is initializing...");

    // 2. 加载配置文件
    if (!Config::GetInstance().Load("../conf/server.json"))
    {
        spdlog::error("Failed to load server.json. Exiting...");
        return 1;
    }

    // 3. 【重点测试区域】初始化数据库并测试查表
    bool db_ready = MySQLManager::GetInstance().Init(
        Config::GetInstance().GetDbHost(),
        Config::GetInstance().GetDbUser(),
        Config::GetInstance().GetDbPassword(),
        Config::GetInstance().GetDbName(),
        Config::GetInstance().GetDbPort());

    if (db_ready)
    {
        // 数据库连上了，我们来查一下刚才在终端里插入的 'user1'
        spdlog::info("Start checking user credentials...");
        bool is_valid = MySQLManager::GetInstance().CheckUser("user1", "123456");

        if (is_valid)
        {
            spdlog::info("🎉 Validation SUCCESS! 'user1' exists and password is correct.");
        }
        else
        {
            spdlog::error("❌ Validation FAILED! Invalid username or password.");
        }
    }
    else
    {
        spdlog::critical("Failed to connect to MySQL. Server will start without DB support.");
    }

    // 4. 启动网络引擎
    std::string ip = Config::GetInstance().GetServerIp();
    uint16_t port = Config::GetInstance().GetServerPort();
    spdlog::info("Config loaded. Server will bind to {}:{}", ip, port);

    try
    {
        TcpServer server(ip, port);
        server.start();
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Server crashed: {}", e.what());
        return 1;
    }
    return 0;
}