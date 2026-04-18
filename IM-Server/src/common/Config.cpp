#include "common/Config.h"

#include <fstream>
#include <iostream>

#include "common/json.hpp"

using json = nlohmann::json;
Config &Config::GetInstance()
{
    static Config instance;
    return instance;
}
bool Config::Load(const std::string &config_file)
{
    try
    {
        std::ifstream file(config_file);
        if (!file.is_open())
        {
            std::cerr << "Failed to open config files: " << config_file
                      << std::endl;
            return false;
        }
        json config_json;
        file >> config_json;
        // 解析并赋值
        server_ip_ = config_json["server"]["ip"];
        server_port_ = config_json["server"]["port"];
        // 【新增】解析数据库配置
        db_host_ = config_json["mysql"]["host"];
        db_port_ = config_json["mysql"]["port"];
        db_user_ = config_json["mysql"]["user"];
        db_password_ = config_json["mysql"]["password"];
        db_name_ = config_json["mysql"]["db_name"];
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Config parsing error: " << e.what() << std::endl;
        return false;
    }
}