#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <cstdint>
#pragma pack(push 1)
struct MsgHeader {
    uint32_t msg_type;     // 消息类型（1:登录；3:单聊等）
    uint32_t body_length;  // 消息体的长度
};
#pragma pack(pop)
#endif