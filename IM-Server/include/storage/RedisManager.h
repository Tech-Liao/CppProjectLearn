#ifndef REDIS_MANAGER_H
#define REDIS_MANAGER_H
#include <string>
#include <memory>
#include <chrono>
#include <spdlog/spdlog.h>
#include <sw/redis++/redis++.h>

class RedisManager {
private:
    RedisManager() = default;
    ~RedisManager() = default;
    std::unique_ptr<sw::redis::Redis> redis_;

public:
    static RedisManager& GetInstance() {
        static RedisManager instance;
        return instance;
    }
    bool Init(const std::string& host = "127.0.0.1", int port = 6379) {
        try {
            sw::redis::ConnectionOptions opts;
            opts.host = host;
            opts.port = port;
            // 连接池配置
            sw::redis::ConnectionPoolOptions pool_opts;
            pool_opts.size = 10;  // 维持10个连接
            redis_ = std::make_unique<sw::redis::Redis>(opts, pool_opts);

            // 发送ping 测试连通
            if (redis_->ping() == "PONG") {
                spdlog::info("Successfully connected to Redis at {}:{}", host,
                             port);
                return true;
            }
            return false;
        } catch (const sw::redis::Error& e) {
            spdlog::error("Redis Init failed: {}", e.what());
            return false;
        }
    }
    // 1、用户上线（代表120s过期时间，防止服务器崩溃导致幽灵在）
    bool SetUserOnline(const std::string& username) {
        if (!redis_) return false;
        try {
            std::string key = "online:user:" + username;
            redis_->set(key, "1", std::chrono::seconds(120));
            return true;
        } catch (const sw::redis::Error& e) {
            spdlog::error("Redis SetUserOnline error:{}", e.what());
            return false;
        }
    }
    // 2、用户下线
    bool SetUserOffline(const std::string& username) {
        if (!redis_) return false;
        try {
            std::string key = "online:user:" + username;
            redis_->del(key);
            return true;
        } catch (const sw::redis::Error& e) {
            spdlog::error("Redis SetUserOffline error:{}", e.what());
            return false;
        }
    }
    // 3、查询是否在线
    bool IsUserOnlinea(const std::string& username) {
        if (!redis_) return false;
        try {
            std::string key = "online:user:" + username;
            auto val = redis_->get(key);
            return val.has_value();
        } catch (const sw::redis::Error& e) {
            spdlog::error("Redis IsUserOnline error: {}", e.what());
            return false;
        }
    }
};
#endif