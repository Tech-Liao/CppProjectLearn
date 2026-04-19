#include "business/UserManager.h"

UserManager& UserManager::GetInstance() {
    static UserManager instance;
    return instance;
}

void UserManager::AddUser(const std::string& username, Connection* conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_map_[username] = conn;
}

void UserManager::RemoveUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_map_.erase(username);
}
Connection* UserManager::GetConnection(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_map_.find(username);
    if (it != user_map_.end()) {
        return it->second;
    }
    return nullptr;
}