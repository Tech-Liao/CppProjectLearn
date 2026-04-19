#include "business/GroupManager.h"

#include <spdlog/spdlog.h>

#include "storage/MySQLManager.h"
void GroupManager::InitLoadFromDB() {
    std::lock_guard<std::mutex> lock(mutex_);
    group_map_ = MySQLManager::GetInstance().GetAllGroupMembers();
    spdlog::info("GroupManager initialized.Load {} group from DB.",
                 group_map_.size());
}

std::unordered_set<std::string> GroupManager::GetGroupMembers(int group_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (group_map_.find(group_id) == group_map_.end()) {
        return {};
    }
    return group_map_[group_id];
}

bool GroupManager::IsUserInGroup(int group_id, const std::string &username) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (group_map_.find(group_id) == group_map_.end()) {
        return false;
    }
    return group_map_[group_id].count(username) > 0;
}