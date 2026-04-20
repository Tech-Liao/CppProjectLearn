#include "network/Connection.h"

#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <chrono>
#include "business/UserManager.h"
#include "common/json.hpp"
#include "network/Codec.h"
#include "storage/MySQLManager.h"
#include "business/GroupManager.h"
#include "storage/RedisManager.h"
using json = nlohmann::json;
static void SetNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl get failed" << std::endl;
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl set failed" << std::endl;
    }
}

Connection::Connection(EventLoop *loop, int fd) : loop_(loop), fd_(fd) {
    SetNonBlocking(fd_);
    loop_->AddEvent(fd_, EPOLLIN, [this]() { this->Read(); });
}

Connection::~Connection() {
    loop_->RemoveEvent(fd_);
    close(fd_);
    std::cout << "Connection " << fd_ << " closed and destroyed." << std::endl;
}

void Connection::Read() {
    char buf[1024];
    while (true) {  // 非阻塞必须循环度，直到无法读取数据为止
        int bytes_read = read(fd_, buf, sizeof(buf));
        if (bytes_read > 0) {
            read_buffer_.Append(buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {
            // 被系统信息打断，正常情况继续读
            continue;
        } else if (bytes_read == -1 &&
                   (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // EAGAIN说明当前缓冲区已经读取完，需要等下次epoll通知
            break;
        } else if (bytes_read == 0) {
            // 代表客户端主动断开了连接
            spdlog::info("client disconnected,fd:{}", fd_);
            if (!current_user_.empty()) {
                // 在线用户本删除
                UserManager::GetInstance().RemoveUser(current_user_);
                spdlog::info("User '{}' removed from UserManager.",
                             current_user_);
                // 从redis 删除状态
                RedisManager::GetInstance().SetUserOffline(current_user_);
                spdlog::info("User '{}' status synced to Redis (offline).",
                             current_user_);
            }
            if (close_callback_) {
                close_callback_(fd_);
            }
            return;
        } else {
            std::cerr << "Read error on fd: " << fd_ << std::endl;
            if (close_callback_) close_callback_(fd_);
            return;
        }
    }

    while (true) {
        uint32_t msg_type = 0;
        std::string msg_body = "";
        bool success = Codec::ParseMessage(&read_buffer_, msg_type, msg_body);
        if (!success) {
            // 解析失败
            break;
        }
        std::cout << "[Codec] 成功拆出一个完整包！Type: " << msg_type
                  << ", Body: " << msg_body << std::endl;
        this->UpdateActiveTime();
        if (msg_type == 3) {
            // 3 代表ping
            spdlog::debug("Received Ping from fd: {}", fd_);
            // 立即回一个type =4（pong）
            std::string pong_json = "{\"msg\":\"pong\"}";
            std::string pong_packet = Codec::PackMessage(4, pong_json);
            Send(pong_packet);
            continue;
        }
        ThreadPool::GetInstance().Enqueue([self = shared_from_this(), msg_type,
                                           msg_body] {
            try {
                // json 反序列化
                json req_json = json::parse(msg_body);
                // 构造回包 json
                json resp_json;
                // ======== 业务路由分发 ==========
                if (msg_type == 1) {  // 登录请求
                    std::string cmd = req_json.value("cmd", "");
                    if (cmd == "login") {
                        std::string username = req_json.value("username", "");
                        std::string password = req_json.value("password", "");

                        bool is_valid = MySQLManager::GetInstance().CheckUser(
                            username, password);
                        resp_json["cmd"] = "login_resp";
                        if (is_valid) {
                            resp_json["code"] = 200;
                            resp_json["msg"] = "Login Success!";
                            self->current_user_ = username;
                            UserManager::GetInstance().AddUser(username, self);
                            spdlog::info(
                                "User '{}' login and registered in "
                                "UserManager.",
                                username);
                            // 将状态写入redis
                            RedisManager::GetInstance().SetUserOnline(username);
                            spdlog::info(
                                "User '{}' status synced to Redis (Online).",
                                username);
                            std::string response_packet =
                                Codec::PackMessage(msg_type, resp_json.dump());
                            self->Send(response_packet);
                            // 获取离线消息
                            auto offline_msgs =
                                MySQLManager::GetInstance()
                                    .GetAndClearOfflineMessages(username);
                            if (!offline_msgs.empty()) {
                                spdlog::info(
                                    "pushing {} offline message to user '{}'",
                                    offline_msgs.size(), username);
                                for (const auto &msg_str : offline_msgs) {
                                    std::string push_packet =
                                        Codec::PackMessage(2, msg_str);
                                    self->Send(push_packet);
                                }
                            }
                            return;
                        } else {
                            resp_json["code"] = 401;
                            resp_json["msg"] = "Invalid username or password";
                        }
                        std::string response_packet =
                            Codec::PackMessage(msg_type, resp_json.dump());
                        self->Send(response_packet);
                    }
                } else if (msg_type == 2) {  // 单聊
                    std::string cmd = req_json.value("cmd", "");
                    if (cmd == "chat") {
                        std::string target_user = req_json.value("to", "");
                        std::string content = req_json.value("msg", "");
                        spdlog::info("Route msg from '{}' to '{}'",
                                     self->current_user_, target_user);
                        auto target_conn_ptr = UserManager::GetInstance()
                                                   .GetConnection(target_user)
                                                   .lock();  // 尝试获取
                        if (target_conn_ptr != nullptr) {    // 目标在线
                            json push_json;
                            push_json["cmd"] = "push_chat";
                            push_json["from"] = self->current_user_;
                            push_json["msg"] = content;
                            std::string push_packet =
                                Codec::PackMessage(2, push_json.dump());
                            // 跨对象调用
                            target_conn_ptr->Send(push_packet);
                            resp_json["code"] = 200;
                            resp_json["msg"] =
                                "Message forwarded successfully.";
                        } else {
                            // 目标不在线
                            spdlog::info(
                                "User '{}' if offline. Saving message to "
                                "database",
                                target_user);
                            bool saved =
                                MySQLManager::GetInstance()
                                    .InsertOfflineMessage(self->current_user_,
                                                          target_user, content);
                            if (saved) {  // 用户离线，存放数据库
                                resp_json["code"] = 200;
                                resp_json["msg"] =
                                    "User offline. Message save to server "
                                    "successfully.";
                            } else {  // 数据库挂了
                                resp_json["code"] = 500;
                                resp_json["msg"] =
                                    "Internal server erro.Failed to save "
                                    "message";
                            }
                        }
                    } else if (cmd == "group_chat") {  // 群聊
                        int group_id = req_json["group_id"];
                        std::string content = req_json["msg"];
                        // 1、验证：发送者自己必须在群里
                        if (!GroupManager::GetInstance().IsUserInGroup(
                                group_id, self->current_user_)) {
                            resp_json["code"] = 403;
                            resp_json["msg"] =
                                "Permission denied.You are not in the group";
                            self->Send(
                                Codec::PackMessage(msg_type, resp_json.dump()));
                            return;
                        }
                        // 2、拿到群名单开始暴力裂变
                        auto members =
                            GroupManager::GetInstance().GetGroupMembers(
                                group_id);
                        int online_count = 0;
                        int offline_count = 0;
                        for (const auto &member : members) {
                            if (member == self->current_user_) continue;
                            json push_json;
                            push_json["cmd"] = "push_group_chat";
                            push_json["group_id"] = group_id;
                            push_json["from"] = self->current_user_;
                            push_json["msg"] = content;
                            std::string push_packet =
                                Codec::PackMessage(2, push_json.dump());
                            // 3. 核心路由分支：去户口本查人
                            std::weak_ptr<Connection> target_conn =
                                UserManager::GetInstance().GetConnection(
                                    member);
                            if (auto target_conn_ptr =
                                    UserManager::GetInstance()
                                        .GetConnection(member)
                                        .lock()) {
                                target_conn_ptr->Send(push_packet);
                                online_count++;
                            } else {
                                MySQLManager::GetInstance()
                                    .InsertOfflineMessage(self->current_user_,
                                                          member,
                                                          "[群聊] " + content);
                                offline_count++;
                            }
                        }
                        // 4. 给发送者回执
                        resp_json["code"] = 200;
                        resp_json["msg"] =
                            "Group message sent! Online: " +
                            std::to_string(online_count) +
                            ", Offline saved: " + std::to_string(offline_count);
                        self->Send(
                            Codec::PackMessage(msg_type, resp_json.dump()));
                        return;
                    }
                    // 给发送者回执包
                    std::string response_packet =
                        Codec::PackMessage(msg_type, resp_json.dump());
                    self->Send(response_packet);
                }
            } catch (json::parse_error &e) {
                spdlog::error("JSON parsing error on fd {}:{}", self->fd_,
                              e.what());
            }
        });
    }
}

void Connection::Send(const std::string &msg) {
    // 简化板，先用write，一次发不完，应该存入write_buffer_
    write(fd_, msg.c_str(), msg.length());
}