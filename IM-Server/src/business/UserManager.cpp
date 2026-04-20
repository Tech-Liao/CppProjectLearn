#include "business/UserManager.h"

#include <spdlog/spdlog.h>
UserManager& UserManager::GetInstance() {
    static UserManager instance;
    return instance;
}

void UserManager::AddUser(const std::string& username,
                          std::shared_ptr<Connection> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_map_[username] = conn;
}

void UserManager::RemoveUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_map_.erase(username);
}
std::weak_ptr<Connection> UserManager::GetConnection(
    const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_map_.find(username);
    if (it != user_map_.end()) {
        return it->second;
    }
    return std::weak_ptr<Connection>();
}

void UserManager::CheckTimeouts(int timeout_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    time_t now = time(NULL);

    for (auto it = user_map_.begin(); it != user_map_.end();) {
        if (auto conn = it->second.lock()) {
            if (now - conn->GetLastActiveTime() > timeout_seconds) {
                spdlog::warn("User '{}' timeout, kicking out.", it->first);
                // 1、强制断开socket
                // 2、从用户本移除
                it = user_map_.erase(it);
            } else {
                ++it;
            }
        } else {
            it = user_map_.erase(it);
        }
    }
}