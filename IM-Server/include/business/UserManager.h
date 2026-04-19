#ifndef USER_MANAGER_H
#define USER_MANAGER_H
#include <mutex>
#include <string>
#include <unordered_map>

#include "network/Connection.h"
class UserManager {
private:
    UserManager() = default;
    ~UserManager() = default;
    UserManager(const UserManager &) = delete;
    UserManager &operator=(const UserManager &) = delete;
    std::unordered_map<std::string, Connection *> user_map_;
    std::mutex mutex_;

public:
    static UserManager &GetInstance();
    void AddUser(const std::string &username, Connection *conn);
    void RemoveUser(const std::string &username);
    Connection *GetConnection(const std::string &username);
};
#endif