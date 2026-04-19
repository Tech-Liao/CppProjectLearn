#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "network/Connection.h"
class GroupManager {
private:
    GroupManager() = default;
    ~GroupManager() = default;
    GroupManager(const GroupManager&) = delete;
    GroupManager& operator=(const GroupManager&) = delete;
    std::mutex mutex_;  // 保护路由表
    // 核心路由表 群号->set-用户表
    std::unordered_map<int, std::unordered_set<std::string>> group_map_;

public:
    // 1、单例模型
    static GroupManager& GetInstance() {
        static GroupManager instance;
        return instance;
    }
    // 2、初始化 --将群成员加载在内存中
    void InitLoadFromDB();
    // 3、获取成员名字
    std::unordered_set<std::string> GetGroupMembers(int group_id);
    // 4、判断成员是否在群
    bool IsUserInGroup(int group_id, const std::string& username);
};

#endif