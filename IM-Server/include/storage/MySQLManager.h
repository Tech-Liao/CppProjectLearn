#ifndef MYSQL_MANAGER_H
#define MYSQL_MANAGER_H
#include <mysql/mysql.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
class MySQLManager {
public:
    static MySQLManager &GetInstance();
    // 初始化数据库连接
    bool Init(const std::string &host, const std::string &user,
              const std::string &pwd, const std::string &db_name,
              int port = 3306);
    // 关闭连接
    void Close();
    // 测试用的查询功能：根据用户名查询密码;
    bool CheckUser(const std::string &username, const std::string &password);
    // 存入离线消息 (返回 bool 表示是否入库成功，参数加 const
    // 保护，变量名语义化)
    bool InsertOfflineMessage(const std::string &sender,
                              const std::string &receiver,
                              const std::string &content);

    // 提取并清理离线消息 (去掉 &，按值返回，依赖 C++ RVO 优化机制)
    std::vector<std::string> GetAndClearOfflineMessages(
        const std::string &receiver);
    // 查询群所有成员
    std::unordered_map<int, std::unordered_set<std::string>>
    GetAllGroupMembers();

private:
    MySQLManager();
    ~MySQLManager();
    MySQLManager(const MySQLManager &) = delete;
    MySQLManager &operator=(const MySQLManager &) = delete;

    MYSQL *conn_;
    std::mutex mutex_;  // 防止多线程查询冲突
};

#endif