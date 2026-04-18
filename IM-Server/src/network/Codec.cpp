#include "network/Codec.h"

#include <arpa/inet.h>

#include <algorithm>

bool Codec::ParseMessage(Buffer *buffer, uint32_t &out_msg_type,
                         std::string &out_msg_body) {
    // 1、检查buffer数据是否足够一个包头（8B），不够就是半包
    if (buffer->ReadableBytes() < sizeof(MsgHeader)) {
        return false;
    }
    MsgHeader header;
    // 2、看包头
    const char *data = buffer->Peek();
    std::copy(data, data + sizeof(MsgHeader),
              reinterpret_cast<char *>(&header));
    // 3、取得包体长度
    uint32_t length = ntohl(header.body_length);
    // 4、检查buffer的数据够不够一个完成的包（包头+包体）
    if (buffer->ReadableBytes() < sizeof(MsgHeader) + length) {
        return false;  // 包体不完整，继续等待接收
    }
    // 5、完整的包
    buffer->Retrieve(sizeof(MsgHeader));  // 丢弃8字节包头
    // 赋值给传出数据
    out_msg_type = ntohl(header.msg_type);
    out_msg_body = buffer->RetrieveAsString(length);
    return true;
}

std::string Codec::PackMessage(uint32_t msg_type, const std::string &msg_body) {
    MsgHeader header;
    header.msg_type = htonl(msg_type);
    header.body_length = htonl(msg_body.length());

    std::string packet;
    packet.append(reinterpret_cast<const char *>(&header), sizeof(MsgHeader));
    packet.append(msg_body);
    return packet;
}