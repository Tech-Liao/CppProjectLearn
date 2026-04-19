#include "storage/MySQLManager.h"

#include "common/json.hpp"
using json = nlohmann::json;
#include <spdlog/spdlog.h>
MySQLManager &MySQLManager::GetInstance() {
    static MySQLManager instance;
    return instance;
}
MySQLManager::MySQLManager() : conn_(nullptr) { conn_ = mysql_init(nullptr); }

MySQLManager::~MySQLManager() { Close(); }

bool MySQLManager::Init(const std::string &host, const std::string &user,
                        const std::string &pwd, const std::string &db_name,
                        int port) {
    if (conn_ == nullptr) {
        spdlog::error("MYSQL init failed!");
        return false;
    }
    // 尝试连接
    if (mysql_real_connect(conn_, host.c_str(), user.c_str(), pwd.c_str(),
                           db_name.c_str(), port, nullptr, 0) == 0) {
        spdlog::error("MYSQL connect error:{}", mysql_error(conn_));
        return false;
    }
    // 设置字符集，防止中文乱码
    mysql_query(conn_, "SET NAMES utf8mb4");
    spdlog::info("MySQL connected successfully to database: {}", db_name);
    return true;
}
void MySQLManager::Close() {
    if (conn_ != nullptr) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

// 模拟登录校验逻辑
bool MySQLManager::CheckUser(const std::string &username,
                             const std::string &password) {
    std::lock_guard<std::mutex> lock(mutex_);
    // 防止SQL注入的简单处理
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT password FROM user WHERE username = '%s'",
             username.c_str());
    if (mysql_query(conn_, query)) {
        spdlog::error("MYSQL query error: {}", mysql_error(conn_));
        return false;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (res == nullptr) {
        return false;
    }
    bool success = false;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row != nullptr) {
        std::string db_pwd = row[0];
        if (db_pwd == password) {
            success = true;
        }
    }
    mysql_free_result(res);
    return success;
}

bool MySQLManager::InsertOfflineMessage(const std::string &sender,
                                        const std::string &receiver,
                                        const std::string &content) {
    // 1、加锁 操作数据库
    std::lock_guard<std::mutex> lock(mutex_);
    // 2、准备sql语句
    char query[2048];
    snprintf(query, sizeof(query),
             "INSERT INTO offline_message (sender, receiver, content) VALUE "
             "('%s', '%s', '%s')",
             sender.c_str(), receiver.c_str(), content.c_str());
    // 4、执行sql语句
    if (mysql_query(conn_, query) != 0) {
        spdlog::error("Failed to insert offline message: {}",
                      mysql_error(conn_));
        return false;
    }

    spdlog::info("Offline message saved! From {} or {}", sender, receiver);
    return true;
}

std::vector<std::string> MySQLManager::GetAndClearOfflineMessages(
    const std::string &receiver) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> messages;

    // 1. 拼凑 SELECT 语句
    char query[2048];
    snprintf(query, sizeof(query),
             "SELECT sender, content, send_time FROM offline_message WHERE "
             "receiver = '%s'",
             receiver.c_str());

    // 2. 执行 SELECT 语句
    if (mysql_query(conn_, query) != 0) {
        spdlog::error("Failed to select offline message: {}",
                      mysql_error(conn_));
        return messages;
    }
    // 3. 获取结果集 (MYSQL_RES* res = mysql_store_result(conn_);)
    MYSQL_RES *res = mysql_store_result(conn_);
    if (res == nullptr) {
        return messages;  // 没查到数据，返回空数组
    }

    // 4. 循环遍历结果集 (MYSQL_ROW row;)
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        json msg_json;
        msg_json["cmd"] = "push_chat";
        msg_json["from"] = row[0] ? row[0] : "unknown";
        msg_json["msg"] = row[1] ? row[1] : "";
        msg_json["time"] = row[2] ? row[2] : "";
        messages.push_back(msg_json.dump());
    }

    // 5. 释放结果集 (mysql_free_result(res);)
    mysql_free_result(res);
    // 6. 如果 messages 不为空，执行 DELETE 语句清空数据库里的记录
    if (!messages.empty()) {
        snprintf(query, sizeof(query),
                 "DELETE FROM offline_message WHERE receiver = '%s'",
                 receiver.c_str());
        if (mysql_query(conn_, query) != 0) {
            spdlog::error("Falied to clear offline message: {}",
                          mysql_error(conn_));
        } else {
            spdlog::info("cleard {} offline message for user '{}'",
                         messages.size(), receiver);
        }
    }
    return messages;
}

std::unordered_map<int, std::unordered_set<std::string>>
MySQLManager::GetAllGroupMembers() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<int, std::unordered_set<std::string>> result;
    const char *query = "SELECT group_id,user_id FROM group_member";
    if (mysql_query(conn_, query)) {
        spdlog::error("Failed to select group members:{}.", mysql_error(conn_));
        return result;
    }
    MYSQL_RES *res = mysql_store_result(conn_);
    if (res == nullptr) return result;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        if (row[0] != nullptr && row[1] != nullptr) {
            int group_id = std::stoi(row[0]);
            std::string user_id = row[1];
            result[group_id].insert(user_id);
        }
    }
    mysql_free_result(res);
    return result;
}