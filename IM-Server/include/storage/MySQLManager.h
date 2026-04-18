#ifndef MYSQL_MANAGER_H
#define MYSQL_MANAGER_H
#include <mysql/mysql.h>
#include <string>
#include <mutex>

class MySQLManager
{
public:
    static MySQLManager &GetInstance();
    // 初始化数据库连接
    bool Init(const std::string &host, const std::string &user, const std::string &pwd,
              const std::string &db_name, int port = 3306);
    // 关闭连接
    void Close();
    // 测试用的查询功能：根据用户名查询密码;
    bool CheckUser(const std::string &username, const std::string &password);

private:
    MySQLManager();
    ~MySQLManager();
    MySQLManager(const MySQLManager &) = delete;
    MySQLManager &operator=(const MySQLManager &) = delete;

    MYSQL *conn_;
    std::mutex mutex_; // 防止多线程查询冲突
};

#endif