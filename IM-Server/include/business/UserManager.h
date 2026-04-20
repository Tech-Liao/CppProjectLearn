#ifndef USER_MANAGER_H
#define USER_MANAGER_H
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory>
#include "network/Connection.h"
class UserManager {
private:
    UserManager() = default;
    ~UserManager() = default;
    UserManager(const UserManager &) = delete;
    UserManager &operator=(const UserManager &) = delete;
    std::unordered_map<std::string, std::weak_ptr<Connection>> user_map_;
    std::mutex mutex_;

public:
    static UserManager &GetInstance();
    void AddUser(const std::string &username, std::shared_ptr<Connection> conn);
    void RemoveUser(const std::string &username);
    std::weak_ptr<Connection> GetConnection(const std::string &username);
    void CheckTimeouts(int timeout_seconds);
};
#endif