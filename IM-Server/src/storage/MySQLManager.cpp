#include "storage/MySQLManager.h"
#include <spdlog/spdlog.h>
MySQLManager &MySQLManager::GetInstance()
{
    static MySQLManager instance;
    return instance;
}
MySQLManager::MySQLManager() : conn_(nullptr)
{
    conn_ = mysql_init(nullptr);
}

MySQLManager::~MySQLManager()
{
    Close();
}

bool MySQLManager::Init(const std::string &host, const std::string &user, const std::string &pwd, const std::string &db_name, int port)
{
    if (conn_ == nullptr)
    {
        spdlog::error("MYSQL init failed!");
        return false;
    }
    // 尝试连接
    if (mysql_real_connect(conn_, host.c_str(), user.c_str(), pwd.c_str(), db_name.c_str(), port, nullptr, 0) == 0)
    {
        spdlog::error("MYSQL connect error:{}", mysql_error(conn_));
        return false;
    }
    // 设置字符集，防止中文乱码
    mysql_query(conn_, "SET NAMES utf8mb4");
    spdlog::info("MySQL connected successfully to database: {}", db_name);
    return true;
}
void MySQLManager::Close()
{
    if (conn_ != nullptr)
    {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

// 模拟登录校验逻辑
bool MySQLManager::CheckUser(const std::string &username, const std::string &password)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 防止SQL注入的简单处理
    char query[256];
    snprintf(query, sizeof(query), "SELECT password FROM user WHERE username = '%s'", username.c_str());
    if (mysql_query(conn_, query))
    {
        spdlog::error("MYSQL query error: {}", mysql_error(conn_));
        return false;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (res == nullptr)
    {
        return false;
    }
    bool success = false;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row != nullptr)
    {
        std::string db_pwd = row[0];
        if (db_pwd == password)
        {
            success = true;
        }
    }
    mysql_free_result(res);
    return success;
}