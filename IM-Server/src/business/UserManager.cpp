#include "business/UserManager.h"

#include <spdlog/spdlog.h>
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

void UserManager::CheckTimeouts(int timeout_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    time_t now = time(NULL);

    for (auto it = user_map_.begin(); it != user_map_.end();) {
        Connection* conn = it->second;
        if (now - conn->GetLastActiveTime() > timeout_seconds) {
            spdlog::warn("User '{}' timeout, kicking out.", it->first);
            // 1、强制断开socket
            // 2、从用户本移除
            it = user_map_.erase(it);
        } else {
            ++it;
        }
    }
}