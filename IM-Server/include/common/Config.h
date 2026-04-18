#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>
#include <string>
class Config
{
public:
    // 单例
    static Config &GetInstance();
    // 加载配置
    bool Load(const std::string &config_file);
    std::string GetServerIp() const { return server_ip_; }
    uint16_t GetServerPort() const { return server_port_; }

    // 【新增】获取数据库配置
    std::string GetDbHost() const { return db_host_; }
    int GetDbPort() const { return db_port_; }
    std::string GetDbUser() const { return db_user_; }
    std::string GetDbPassword() const { return db_password_; }
    std::string GetDbName() const { return db_name_; }

private:
    Config() = default;
    ~Config() = default;
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    std::string server_ip_;
    uint16_t server_port_;

    // 【新增】数据库私有变量
    std::string db_host_;
    int db_port_;
    std::string db_user_;
    std::string db_password_;
    std::string db_name_;
};
#endif